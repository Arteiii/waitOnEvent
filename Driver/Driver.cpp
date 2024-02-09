#include <ntddk.h>

// Forward declaration of the Unload routine
extern "C" VOID
TestUnload(_In_ PDRIVER_OBJECT DriverObject);

// Entry point for the driver
extern "C" NTSTATUS
DriverEntry(_In_ PDRIVER_OBJECT DriverObject,
            _In_ PUNICODE_STRING /*RegistryPath*/)
{
  // Assign the Unload routine to the DriverUnload member of the DRIVER_OBJECT
  DriverObject->DriverUnload = TestUnload;

  // Print a message to the debugger indicating successful driver initialization
  DbgPrint("Driver initialized!\n");

  // Print a debug message using KdPrint (visible in debug builds)
  KdPrint(("You will see this only if you compile in debug mode\n"));

  // Return STATUS_SUCCESS to indicate successful driver initialization
  return STATUS_SUCCESS;
}

// Unload routine for the driver
extern "C" VOID
TestUnload(_In_ PDRIVER_OBJECT /*DriverObject*/)
{
  // Print a message to the debugger indicating driver unload
  KdPrint(("Driver unload\n"));
}
