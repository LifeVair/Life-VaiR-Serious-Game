// Copyright Epic Games, Inc. All Rights Reserved.

#include "OculusXRSimulator.h"
#include "OculusXRHMDRuntimeSettings.h"
#include "Misc/MessageDialog.h"

#include "Windows/WindowsPlatformMisc.h"

const FString OpenXrRuntimeEnvKey = "XR_RUNTIME_JSON";
const FString PreviousOpenXrRuntimeEnvKey = "XR_RUNTIME_JSON_PREV";

bool FMetaXRSimulator::IsSimulatorActivated()
{
	FString MetaXRSimPath = GetSimulatorJsonPath();
	FString CurRuntimePath = FWindowsPlatformMisc::GetEnvironmentVariable(*OpenXrRuntimeEnvKey);
	return (!MetaXRSimPath.IsEmpty() && MetaXRSimPath == CurRuntimePath);
}

void FMetaXRSimulator::ToggleOpenXRRuntime()
{
	FString MetaXRSimPath = GetSimulatorJsonPath();
	if (!IFileManager::Get().FileExists(*MetaXRSimPath))
	{
		FString Message("Meta XR Simulator Not Found.\nPlease set its path in Project Settings/Meta XR Plugin/PC.");
		FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(Message));
		UE_LOG(LogMetaXRSim, Error, TEXT("%s"), *Message);

		return;
	}

	if (IsSimulatorActivated())
	{
		//Deactivate MetaXR Simulator
		FString PrevOpenXrRuntimeEnvKey = FWindowsPlatformMisc::GetEnvironmentVariable(*PreviousOpenXrRuntimeEnvKey);

		FWindowsPlatformMisc::SetEnvironmentVar(*PreviousOpenXrRuntimeEnvKey,
			TEXT(""));
		FWindowsPlatformMisc::SetEnvironmentVar(*OpenXrRuntimeEnvKey, *PrevOpenXrRuntimeEnvKey);

		UE_LOG(LogMetaXRSim, Log, TEXT("Meta XR Simulator is deactivated. (%s : %s)"), *OpenXrRuntimeEnvKey, *PrevOpenXrRuntimeEnvKey);
	}
	else
	{
		//Activate MetaXR Simulator
		FString CurOpenXrRuntimeEnvKey = FWindowsPlatformMisc::GetEnvironmentVariable(*OpenXrRuntimeEnvKey);

		FWindowsPlatformMisc::SetEnvironmentVar(*PreviousOpenXrRuntimeEnvKey,
			*CurOpenXrRuntimeEnvKey);
		FWindowsPlatformMisc::SetEnvironmentVar(*OpenXrRuntimeEnvKey, *MetaXRSimPath);

		UE_LOG(LogMetaXRSim, Log, TEXT("Meta XR Simulator is activated. (%s : %s)"), *OpenXrRuntimeEnvKey, *MetaXRSimPath);
	}
}

FString FMetaXRSimulator::GetSimulatorJsonPath()
{
	return GetMutableDefault<UOculusXRHMDRuntimeSettings>()->MetaXRJsonPath.FilePath;
}
