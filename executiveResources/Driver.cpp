#include <ntddk.h>

#include "resource.h"

#define DTAG 'eveR'

extern "C" DRIVER_UNLOAD DriverUnload;
extern "C" DRIVER_INITIALIZE DriverEntry;

PERESOURCE globalResource; // <-- Global declaration of 'resource'

extern "C" NTSTATUS
DriverEntry(_In_ PDRIVER_OBJECT driver_object,
            _In_ PUNICODE_STRING registry_path)
{
  KdPrint(("[+] Driver loaded, registry_path: %wZ\n", registry_path));
  driver_object->DriverUnload = DriverUnload;
  globalResource =
    (PERESOURCE)ExAllocatePool2(POOL_FLAG_NON_PAGED, sizeof(ERESOURCE), DTAG);
  if (globalResource == nullptr) {
    KdPrint(("[-] Not enough memory to allocate the resource\n"));
    return STATUS_INSUFFICIENT_RESOURCES;
  }

  NTSTATUS status = ExInitializeResourceLite(globalResource);
  if (!NT_SUCCESS(status)) {
    KdPrint(
      ("[-] Error while trying to initialize the resource, returned (0x%d)\n",
       status));
    ExFreePool(globalResource);
    return status;
  }
  KdPrint(("[+] Resource allocated successfully\n"));

  ResourceShared shared_resource(globalResource);
  ResourceExclusive exclusive_resource(globalResource);
  {
    Resource<ResourceExclusive> exclusive_resource_raii(&exclusive_resource);
    Resource<ResourceShared> shared_resource_raii(&shared_resource);
  }

  ASSERT(status == STATUS_SUCCESS);
  return STATUS_SUCCESS;
}

extern "C" void
DriverUnload(_In_ PDRIVER_OBJECT driver_object)
{
  UNREFERENCED_PARAMETER(driver_object);
  if (globalResource != nullptr) {
    ExDeleteResourceLite(globalResource);
    ExFreePool(globalResource);
  }
  KdPrint(("[+] Unloaded driver successfuly\n"));
}
