#include <ntddk.h>

class ExecutiveResourceWrapper
{
public:
  ExecutiveResourceWrapper()
  {
    KeEnterCriticalRegion();
    ExAcquireResourceExclusiveLite(&resource, TRUE);
  }

  ~ExecutiveResourceWrapper()
  {
    ExReleaseResourceLite(&resource);
    KeLeaveCriticalRegion();
  }

private:
  // The executive resource
  ERESOURCE resource;
};
