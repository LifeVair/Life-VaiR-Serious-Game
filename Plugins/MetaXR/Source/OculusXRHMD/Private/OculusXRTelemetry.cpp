/*
Copyright (c) Meta Platforms, Inc. and affiliates.
All rights reserved.

This source code is licensed under the license found in the
LICENSE file in the root directory of this source tree.
*/

#include "OculusXRTelemetry.h"
#include "OculusXRHMDModule.h"
#include "OculusXRTelemetryPrivacySettings.h"
#include "Async/Async.h"

namespace OculusXRTelemetry
{
	bool IsActive()
	{
#if OCULUS_HMD_SUPPORTED_PLATFORMS
		if constexpr (FTelemetryBackend::IsNullBackend())
		{
			return false;
		}
		if (FOculusXRHMDModule::Get().IsOVRPluginAvailable() && FOculusXRHMDModule::GetPluginWrapper().IsInitialized())
		{
#if WITH_EDITOR
			if (const auto EditorPrivacySettings = GetDefault<UOculusXRTelemetryPrivacySettings>())
			{
				return EditorPrivacySettings->bIsEnabled;
			}
#else  // WITH_EDITOR
			return true;
#endif // WITH_EDITOR
		}
#endif // OCULUS_HMD_SUPPORTED_PLATFORMS
		return false;
	}

	void IfActiveThen(TUniqueFunction<void()> Function)
	{
		AsyncTask(ENamedThreads::GameThread, [F = MoveTemp(Function)]() {
			if (IsActive())
			{
				F();
			}
		});
	}
} // namespace OculusXRTelemetry
