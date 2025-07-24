#include "OpenXRSpaceMgr.h"

#include "OpenXRCoreMgr.h"
#include "OpenXRHelper.h"

XrSpace OpenXRSpaceMgr::activeSpaces = XR_NULL_HANDLE;

void OpenXRSpaceMgr::CreateReferenceSpace()
{
    XrReferenceSpaceCreateInfo referenceSpaceCreateInfo{XR_TYPE_REFERENCE_SPACE_CREATE_INFO, nullptr};
    referenceSpaceCreateInfo.referenceSpaceType = XR_REFERENCE_SPACE_TYPE_LOCAL;
    referenceSpaceCreateInfo.poseInReferenceSpace = XrPosef{{0.0f, 0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 0.0f}};
    OPENXR_CHECK(xrCreateReferenceSpace(OpenXRCoreMgr::xrSession, &referenceSpaceCreateInfo, &activeSpaces),
                 "Failed to create OpenXR local reference space");
}

void OpenXRSpaceMgr::DestroyReferenceSpace()
{
    OPENXR_CHECK(xrDestroySpace(activeSpaces), "Failed to destroy Space.");
}