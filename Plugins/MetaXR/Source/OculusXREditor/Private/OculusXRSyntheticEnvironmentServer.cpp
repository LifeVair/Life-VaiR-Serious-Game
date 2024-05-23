// Copyright Epic Games, Inc. All Rights Reserved.

#include "OculusXRSyntheticEnvironmentServer.h"
#include "OculusXRHMDRuntimeSettings.h"
#include "Misc/MessageDialog.h"

#include "Windows/WindowsPlatformMisc.h"

const FString SynthEnvServer = "Synthetic Environment Server";
const FString LocalSharingServer = "Local Sharing Server";

FProcHandle FMetaXRSES::EnvProcHandle;
FProcHandle FMetaXRSES::LSSProcHandle;

void FMetaXRSES::LaunchOffice()
{
	if (GetMetaXRSimPackagePath().IsEmpty())
	{
		return;
	}
	StopServer();
	LaunchEnvironment("Office");
	LaunchLocalSharingServer();
}

void FMetaXRSES::LaunchLivingRoom()
{
	if (GetMetaXRSimPackagePath().IsEmpty())
	{
		return;
	}
	StopServer();
	LaunchEnvironment("LivingRoom");
	LaunchLocalSharingServer();
}

void FMetaXRSES::LaunchBedroom()
{
	if (GetMetaXRSimPackagePath().IsEmpty())
	{
		return;
	}
	StopServer();
	LaunchEnvironment("Bedroom");
	LaunchLocalSharingServer();
}

void FMetaXRSES::StopServer()
{
	StopProcess(EnvProcHandle, SynthEnvServer);
	StopProcess(LSSProcHandle, LocalSharingServer);
}

void FMetaXRSES::LaunchEnvironment(FString EnvironmentName)
{
	FString SESPath = GetSynthEnvServerPath();
	LaunchProcess(SESPath, EnvironmentName, LocalSharingServer, EnvProcHandle);
}

void FMetaXRSES::LaunchLocalSharingServer()
{
	FString LSSPath = GetLocalSharingServerPath();
	LaunchProcess(LSSPath, "", LocalSharingServer, LSSProcHandle);
}

void FMetaXRSES::LaunchProcess(FString BinaryPath, FString Arguments, FString LogContext, FProcHandle& OutProcHandle)
{
	if (!IFileManager::Get().FileExists(*BinaryPath))
	{
		UE_LOG(LogMetaXRSES, Error, TEXT("Failed to find %s."), *BinaryPath);
		return;
	}
	UE_LOG(LogMetaXRSES, Log, TEXT("Launching %s."), *BinaryPath);

	uint32 OutProcessId = 0;
	OutProcHandle = FPlatformProcess::CreateProc(*BinaryPath, *Arguments, false, false, false, &OutProcessId, 0, NULL, NULL);
	if (!OutProcHandle.IsValid())
	{
		UE_LOG(LogMetaXRSES, Error, TEXT("Failed to launch %s."), *BinaryPath);
		FPlatformProcess::CloseProc(OutProcHandle);
		return;
	}

	UE_LOG(LogMetaXRSES, Log, TEXT("Launched %s."), *BinaryPath);
}

void FMetaXRSES::StopProcess(FProcHandle& ProcHandle, FString LogContext)
{
	if (ProcHandle.IsValid())
	{
		if (FPlatformProcess::IsProcRunning(ProcHandle))
		{
			UE_LOG(LogMetaXRSES, Log, TEXT("Stopping %s."), *LogContext);
			FPlatformProcess::TerminateProc(ProcHandle);
		}
		FPlatformProcess::CloseProc(ProcHandle);
	}
	else
	{
		UE_LOG(LogMetaXRSES, Warning, TEXT("Failed to stop process %s because it is not active anymore."), *LogContext);
	}
}

FString FMetaXRSES::GetMetaXRSimPackagePath()
{
	FString PackagePath = GetMutableDefault<UOculusXRHMDRuntimeSettings>()->MetaXRPackagePath.Path;
	if (PackagePath.IsEmpty())
	{
		FMessageDialog::Open(EAppMsgType::Ok, FText::FromString("Meta XR Simulator Package Path Not Found.\nPlease set its path in Project Settings/Meta XR Plugin/PC."));
	}
	return PackagePath;
}

FString FMetaXRSES::GetSynthEnvServerPath()
{
	return GetMetaXRSimPackagePath() + "/MetaXRSimulator/.synth_env_server/synth_env_server.exe";
}

FString FMetaXRSES::GetLocalSharingServerPath()
{
	return GetMetaXRSimPackagePath() + "/MetaXRSimulator/.local_sharing_server/local_sharing_server.exe";
}
