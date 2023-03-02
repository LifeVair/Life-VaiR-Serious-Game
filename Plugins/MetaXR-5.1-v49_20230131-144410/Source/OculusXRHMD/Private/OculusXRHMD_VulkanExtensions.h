// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once
#include "OculusXRHMDPrivate.h"
#include "IHeadMountedDisplayVulkanExtensions.h"

#if OCULUS_HMD_SUPPORTED_PLATFORMS

namespace OculusXRHMD
{


//-------------------------------------------------------------------------------------------------
// FVulkanExtensions
//-------------------------------------------------------------------------------------------------

class FVulkanExtensions : public IHeadMountedDisplayVulkanExtensions, public TSharedFromThis<FVulkanExtensions, ESPMode::ThreadSafe>
{
public:
	FVulkanExtensions() {}
	virtual ~FVulkanExtensions() {}

	// IHeadMountedDisplayVulkanExtensions
	virtual bool GetVulkanInstanceExtensionsRequired(TArray<const ANSICHAR*>& Out) override;
	virtual bool GetVulkanDeviceExtensionsRequired(struct VkPhysicalDevice_T *pPhysicalDevice, TArray<const ANSICHAR*>& Out) override;
};

} // namespace OculusXRHMD

#endif //OCULUS_HMD_SUPPORTED_PLATFORMS