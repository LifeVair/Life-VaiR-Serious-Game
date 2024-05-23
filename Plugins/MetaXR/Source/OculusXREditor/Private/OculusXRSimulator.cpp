// Copyright Epic Games, Inc. All Rights Reserved.

#include "OculusXRSimulator.h"
#include "OculusXRHMDRuntimeSettings.h"
#include "Misc/MessageDialog.h"

#include "Windows/WindowsPlatformMisc.h"

const FString OpenXrRuntimeEnvKey = "XR_RUNTIME_JSON";
const FString PreviousOpenXrRuntimeEnvKey = "XR_RUNTIME_JSON_PREV";
const FString XrSimConfigEnvKey = "META_XRSIM_CONFIG_JSON";
const FString PreviousXrSimConfigEnvKey = "META_XRSIM_CONFIG_JSON_PREV";
const FString MetaXRSimFileName = "meta_openxr_simulator.json";
const FString MetaXRSimCfgName = "sim_core_configuration.json";

bool FMetaXRSimulator::IsSimulatorActivated()
{
	FString MetaXRSimPath = GetSimulatorJsonPath();
	FString CurRuntimePath = FWindowsPlatformMisc::GetEnvironmentVariable(*OpenXrRuntimeEnvKey);
	return (!MetaXRSimPath.IsEmpty() && MetaXRSimPath == CurRuntimePath);
}

void FMetaXRSimulator::ToggleOpenXRRuntime()
{
	FString MetaXRSimPath = GetSimulatorJsonPath();
	FString MetaXRSimCfgPath = GetSimulatorConfigPath();
	if (MetaXRSimPath.IsEmpty() || MetaXRSimCfgPath.IsEmpty())
	{
		return;
	}

	if (IsSimulatorActivated())
	{
		//Deactivate MetaXR Simulator
		FString PrevOpenXrRuntimeEnvKey = FWindowsPlatformMisc::GetEnvironmentVariable(*PreviousOpenXrRuntimeEnvKey);
		FString PrevXrSimConfigEnvKey = FWindowsPlatformMisc::GetEnvironmentVariable(*PreviousXrSimConfigEnvKey);

		FWindowsPlatformMisc::SetEnvironmentVar(*PreviousOpenXrRuntimeEnvKey,
			TEXT(""));
		FWindowsPlatformMisc::SetEnvironmentVar(*OpenXrRuntimeEnvKey, *PrevOpenXrRuntimeEnvKey);

		FWindowsPlatformMisc::SetEnvironmentVar(*PreviousXrSimConfigEnvKey,
			TEXT(""));
		FWindowsPlatformMisc::SetEnvironmentVar(*XrSimConfigEnvKey, *PrevXrSimConfigEnvKey);

		UE_LOG(LogMetaXRSim, Log, TEXT("Meta XR Simulator is deactivated. (%s : %s), (%s : %s)"), *OpenXrRuntimeEnvKey, *PrevOpenXrRuntimeEnvKey, *XrSimConfigEnvKey, *PrevXrSimConfigEnvKey);
	}
	else
	{
		//Activate MetaXR Simulator
		FString CurOpenXrRuntimeEnvKey = FWindowsPlatformMisc::GetEnvironmentVariable(*OpenXrRuntimeEnvKey);
		FString CurXrSimConfigEnvKey = FWindowsPlatformMisc::GetEnvironmentVariable(*XrSimConfigEnvKey);

		FWindowsPlatformMisc::SetEnvironmentVar(*PreviousOpenXrRuntimeEnvKey,
			*CurOpenXrRuntimeEnvKey);
		FWindowsPlatformMisc::SetEnvironmentVar(*OpenXrRuntimeEnvKey, *MetaXRSimPath);

		FWindowsPlatformMisc::SetEnvironmentVar(*PreviousXrSimConfigEnvKey,
			*CurXrSimConfigEnvKey);
		FWindowsPlatformMisc::SetEnvironmentVar(*XrSimConfigEnvKey, *MetaXRSimCfgPath);

		UE_LOG(LogMetaXRSim, Log, TEXT("Meta XR Simulator is activated. (%s : %s), (%s : %s)"), *OpenXrRuntimeEnvKey, *MetaXRSimPath, *XrSimConfigEnvKey, *MetaXRSimCfgPath);
	}
}

FString FMetaXRSimulator::GetMetaXRSimPackagePath()
{
	FString PackagePath = GetMutableDefault<UOculusXRHMDRuntimeSettings>()->MetaXRPackagePath.Path;
	if (PackagePath.IsEmpty())
	{
		FMessageDialog::Open(EAppMsgType::Ok, FText::FromString("Meta XR Simulator Package Path Not Found.\nPlease set its path in Project Settings/Meta XR Plugin/PC."));
	}
	return PackagePath;
}

FString FMetaXRSimulator::GetSimulatorJsonPath()
{
	return GetMetaXRSimPackagePath() + "/MetaXRSimulator/meta_openxr_simulator.json";
}

FString FMetaXRSimulator::GetSimulatorConfigPath()
{
	return GetMetaXRSimPackagePath() + "/MetaXRSimulator/config/sim_core_configuration.json";
}
