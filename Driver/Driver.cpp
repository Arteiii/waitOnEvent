#include <ntddk.h>

static volatile BOOLEAN g_IsDriverUnloading = FALSE;

// Forward declaration of the Driver routines
extern "C" DRIVER_UNLOAD NotificationUnload;
extern "C" DRIVER_INITIALIZE DriverEntry;

_Function_class_(KSTART_ROUTINE) void EventThreadRoutine(_In_ PVOID context);

// Initialize the event object in the DriverEntry function
extern "C" NTSTATUS
DriverEntry(_In_ PDRIVER_OBJECT DriverObject, _In_ PUNICODE_STRING RegistryPath)
{
  UNREFERENCED_PARAMETER(RegistryPath);

  KdPrint(("DriverEntry\n"));
  DriverObject->DriverUnload = NotificationUnload;

  UNICODE_STRING ObjectNames[] = {
    RTL_CONSTANT_STRING(L"\\KernelObjects\\HighMemoryCondition"),
    RTL_CONSTANT_STRING(L"\\KernelObjects\\LowMemoryCondition"),
    RTL_CONSTANT_STRING(L"\\KernelObjects\\HighPagedPoolCondition"),
    RTL_CONSTANT_STRING(L"\\KernelObjects\\LowPagedPoolCondition"),
    RTL_CONSTANT_STRING(L"\\KernelObjects\\HighNonPagedPoolCondition"),
    RTL_CONSTANT_STRING(L"\\KernelObjects\\LowNonPagedPoolCondition"),
    RTL_CONSTANT_STRING(L"\\KernelObjects\\HighCommitCondition"),
    RTL_CONSTANT_STRING(L"\\KernelObjects\\LowCommitCondition"),
    RTL_CONSTANT_STRING(L"\\KernelObjects\\MaximumCommitCondition")
  };

  static constexpr size_t NumObjects = 9;

  HANDLE hEvents[NumObjects];

  for (size_t i = 0; i < NumObjects; ++i) {
    PKEVENT event = nullptr;
    HANDLE thread_handle = nullptr;

    NTSTATUS leave_status = STATUS_SUCCESS;

    __try {
      event = IoCreateNotificationEvent(&ObjectNames[i], &hEvents[i]);

      const NTSTATUS status = IoCreateSystemThread(DriverObject,
                                                   &thread_handle,
                                                   THREAD_ALL_ACCESS,
                                                   nullptr,
                                                   NtCurrentProcess(),
                                                   nullptr,
                                                   EventThreadRoutine,
                                                   event);

      if (!NT_SUCCESS(status)) {
        KdPrint(("Failed to Initialize %wZ thread routine, Status: 0x%X\n",
                 &ObjectNames[i],
                 status));
        leave_status = status;
        __leave;
      }
      // Continue with the loop if thread creation is successful
    } __finally {
      if (leave_status != STATUS_SUCCESS) {
        KdPrint(("Exception caught during DriverEntry\n"));
        return leave_status;
      }
    }
  }

  return STATUS_SUCCESS;
}

// Unload routine for the driver
extern "C" VOID
NotificationUnload(_In_ PDRIVER_OBJECT DriverObject)
{
  UNREFERENCED_PARAMETER(DriverObject);

  // Signal the threads to exit
  g_IsDriverUnloading = TRUE;

  // Wait for the threads to exit
  // (you may need to implement a proper synchronization mechanism here)

  // Print a message to the debugger indicating driver unload
  KdPrint(("DriverUnload\n"));
}

_Function_class_(KSTART_ROUTINE) void EventThreadRoutine(_In_ PVOID context)
{
  UNICODE_STRING* name = static_cast<UNICODE_STRING*>(context);

  KdPrint(("Routine: %wZ\n", name));

  while (!g_IsDriverUnloading) {
    __try {
      const NTSTATUS status =
        KeWaitForSingleObject(context, Executive, KernelMode, FALSE, nullptr);

      if (!NT_SUCCESS(status)) {
        KdPrint(("Expected NT_SUCCESS but received: 0x%X\n", status));
        continue;
      }

      KdPrint(("%wZ event signaled!\n", name));
      // Do something with the signaled event

    } __except (EXCEPTION_EXECUTE_HANDLER) {
      NTSTATUS exceptionCode = GetExceptionCode();
      PVOID exceptionAddress = _ReturnAddress();

      KdPrint(("Exception caught during NamedEventRoutine for %wZ\n", name));
      KdPrint(("Exception Code: 0x%X\n", exceptionCode));
      KdPrint(("Exception Address: %p\n", exceptionAddress));

      // Handle specific exceptions if needed
      if (exceptionCode == STATUS_ACCESS_VIOLATION) {
        // Handle memory access violation
        // ...
      }
    }
  }

  KdPrint(("Exit Routine: %wZ\n", name));
  PsTerminateSystemThread(STATUS_SUCCESS);
}
