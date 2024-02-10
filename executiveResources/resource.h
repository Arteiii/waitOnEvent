#pragma once

// Include the Windows Driver Model (WDM) header file
#include <wdm.h>

/*
 * References:
 *  Windows Drivers:
 *      https://docs.microsoft.com/en-us/windows-hardware/drivers/
 *
 *  ExEnterCriticalRegionAndAcquireResourceShared Function:
 *      https://docs.microsoft.com/en-us/windows-hardware/drivers/ddi/wdm/nf-wdm-exentercriticalregionandacquireresourceshared
 *
 *  ExEnterCriticalRegionAndAcquireResourceExclusive Function:
 *      https://docs.microsoft.com/en-us/windows-hardware/drivers/ddi/wdm/nf-wdm-exentercriticalregionandacquireresourceexclusive
 *
 *  ExReleaseResourceAndLeaveCriticalRegion Function:
 *      https://docs.microsoft.com/en-us/windows-hardware/drivers/ddi/wdm/nf-wdm-exreleaseresourceandleavecriticalregion
 *
 */

// Define a structure for a resource shared lock
struct ResourceShared
{
  // Constructor that initializes the structure
  // with a given PERESOURCE (sharedresource)
  ResourceShared(PERESOURCE resource)
    : _resource(resource)
  {
  }

  // Method to acquire the shared resource lock
  void Acquire() { ExEnterCriticalRegionAndAcquireResourceShared(_resource); }

  // Method to release the shared resource lock
  void Release() { ExReleaseResourceAndLeaveCriticalRegion(_resource); }

  // Method to return a string representing the type of lock (shared)
  const char* Name() const { return "Shared"; }

private:
  PERESOURCE _resource; // Pointer to the shared resource
};

// Define a structure for a resource exclusive lock
struct ResourceExclusive
{
  // Constructor that initializes the structure with a given PERESOURCE
  // (exclusive resource)
  ResourceExclusive(PERESOURCE resource)
    : _resource(resource)
  {
  }

  // Method to acquire the exclusive resource lock
  void Acquire()
  {
    ExEnterCriticalRegionAndAcquireResourceExclusive(_resource);
  }

  // Method to release the exclusive resource lock
  void Release() { ExReleaseResourceAndLeaveCriticalRegion(_resource); }

  // Method to return a string representing the type of lock (exclusive)
  const char* Name() const { return "Exclusive"; }

private:
  PERESOURCE _resource; // Pointer to the exclusive resource
};

// Define a template structure for a generic resource
// (either shared or exclusive)
template<typename IResource>
struct Resource
{
  // Constructor that acquires the resource lock
  // if the provided resource is not null
  Resource(IResource* resource)
    : _resource(resource)
  {
    if (_resource != nullptr) {
      // Print a debug message indicating the start of the critical region and
      // acquiring the resource
      KdPrint(("enter critical region! acquiring: %s\n", _resource->Name()));
      _resource->Acquire(); // Acquire the resource lock
    }
  }

  // Destructor that releases the resource lock
  ~Resource()
  {
    if (_resource != nullptr) {
      _resource->Release(); // Release the resource lock
      // Print a debug message indicating the end of the critical region
      // and releasing the resource
      KdPrint(("leave critical region! releasing: %s\n", _resource->Name()));
    }
  }

private:
  IResource* _resource; // Pointer to the resource (either shared or exclusive)
};
