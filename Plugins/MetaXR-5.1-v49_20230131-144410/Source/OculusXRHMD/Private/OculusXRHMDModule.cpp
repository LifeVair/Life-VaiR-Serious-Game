// Copyright Epic Games, Inc. All Rights Reserved.

#include "OculusXRHMDModule.h"
#include "OculusXRHMD.h"
#include "OculusXRHMDPrivateRHI.h"
#include "OculusXRHMDRuntimeSettings.h"
#include "Containers/StringConv.h"
#include "Misc/EngineVersion.h"
#include "Misc/Paths.h"
#if PLATFORM_ANDROID
	#include "Android/AndroidApplication.h"
	#include "Android/AndroidPlatformMisc.h"
#endif
#include "Interfaces/IPluginManager.h"
#include "ShaderCore.h"

//-------------------------------------------------------------------------------------------------
// FOculusXRHMDModule
//-------------------------------------------------------------------------------------------------

OculusPluginWrapper FOculusXRHMDModule::PluginWrapper;

FOculusXRHMDModule::FOculusXRHMDModule()
{
#if OCULUS_HMD_SUPPORTED_PLATFORMS
	bPreInit = false;
	bPreInitCalled = false;
	OVRPluginHandle = nullptr;
	GraphicsAdapterLuid = 0;
#endif
}

void FOculusXRHMDModule::StartupModule()
{
	IHeadMountedDisplayModule::StartupModule();
}

void FOculusXRHMDModule::ShutdownModule()
{
#if OCULUS_HMD_SUPPORTED_PLATFORMS
	if (PluginWrapper.IsInitialized())
	{
		PluginWrapper.Shutdown2();
		OculusPluginWrapper::DestroyOculusPluginWrapper(&PluginWrapper);
	}

	if (OVRPluginHandle)
	{
		FPlatformProcess::FreeDllHandle(OVRPluginHandle);
		OVRPluginHandle = nullptr;
	}
#endif
}

#if PLATFORM_ANDROID
extern bool AndroidThunkCpp_IsOculusMobileApplication();
#endif

FString FOculusXRHMDModule::GetModuleKeyName() const
{
	return FString(TEXT("OculusXRHMD"));
}

void FOculusXRHMDModule::GetModuleAliases(TArray<FString>& AliasesOut) const
{
	// Pre-OculusXR rename (5.0.3 v44)
	AliasesOut.Add(TEXT("OculusHMD"));
}

bool FOculusXRHMDModule::PreInit()
{
#if OCULUS_HMD_SUPPORTED_PLATFORMS
	if (!bPreInitCalled)
	{
		bPreInit = false;

	#if PLATFORM_ANDROID
		bPreInitCalled = true;
		if (!AndroidThunkCpp_IsOculusMobileApplication())
		{
			UE_LOG(LogHMD, Log, TEXT("App is not packaged for Oculus Mobile"));
			return false;
		}
	#endif

		// Init module if app can render
		if (FApp::CanEverRender())
		{
			// Load OVRPlugin
			OVRPluginHandle = GetOVRPluginHandle();

			if (!OVRPluginHandle)
			{
				UE_LOG(LogHMD, Log, TEXT("Failed loading OVRPlugin %s"), TEXT(OVRP_VERSION_STR));
				return false;
			}

			if (!OculusPluginWrapper::InitializeOculusPluginWrapper(&PluginWrapper))
			{
				UE_LOG(LogHMD, Log, TEXT("Failed InitializeOculusPluginWrapper"));
				return false;
			}

			// Initialize OVRPlugin
			ovrpRenderAPIType PreinitApiType = ovrpRenderAPI_None;
	#if PLATFORM_ANDROID
			void* Activity = (void*)FAndroidApplication::GetGameActivityThis();
			PreinitApiType = ovrpRenderAPI_Vulkan;
	#else
			void* Activity = nullptr;
	#endif

			if (OVRP_FAILURE(PluginWrapper.PreInitialize5(Activity, PreinitApiType, ovrpPreinitializeFlags::ovrpPreinitializeFlag_None)))
			{
				UE_LOG(LogHMD, Log, TEXT("Failed initializing OVRPlugin %s"), TEXT(OVRP_VERSION_STR));
#if PLATFORM_WINDOWS
				return true;
#else
				return false;
#endif
			}

	#if PLATFORM_WINDOWS
			bPreInitCalled = true;
			const LUID* DisplayAdapterId;
			if (OVRP_SUCCESS(PluginWrapper.GetDisplayAdapterId2((const void**)&DisplayAdapterId)) && DisplayAdapterId)
			{
				SetGraphicsAdapterLuid(*(const uint64*)DisplayAdapterId);
			}
			else
			{
				UE_LOG(LogHMD, Log, TEXT("Could not determine HMD display adapter"));
			}

			const WCHAR* AudioInDeviceId;
			if (OVRP_SUCCESS(PluginWrapper.GetAudioInDeviceId2((const void**)&AudioInDeviceId)) && AudioInDeviceId)
			{
				GConfig->SetString(TEXT("Oculus.Settings"), TEXT("AudioInputDevice"), AudioInDeviceId, GEngineIni);
			}
			else
			{
				UE_LOG(LogHMD, Log, TEXT("Could not determine HMD audio input device"));
			}

			const WCHAR* AudioOutDeviceId;
			if (OVRP_SUCCESS(PluginWrapper.GetAudioOutDeviceId2((const void**)&AudioOutDeviceId)) && AudioOutDeviceId)
			{
				GConfig->SetString(TEXT("Oculus.Settings"), TEXT("AudioOutputDevice"), AudioOutDeviceId, GEngineIni);
			}
			else
			{
				UE_LOG(LogHMD, Log, TEXT("Could not determine HMD audio output device"));
			}
	#endif

			float ModulePriority;
			if (!GConfig->GetFloat(TEXT("HMDPluginPriority"), *GetModuleKeyName(), ModulePriority, GEngineIni))
			{
				// if user doesn't set priority set it for them to allow this hmd to be used if enabled
				ModulePriority = 45.0f;
				GConfig->SetFloat(TEXT("HMDPluginPriority"), *GetModuleKeyName(), ModulePriority, GEngineIni);
			}

			UE_LOG(LogHMD, Log, TEXT("FOculusXRHMDModule PreInit successfully"));

			bPreInit = true;
		}
	}

	return bPreInit;
#else
	return false;
#endif // OCULUS_HMD_SUPPORTED_PLATFORMS
}

bool FOculusXRHMDModule::IsHMDConnected()
{
#if OCULUS_HMD_SUPPORTED_PLATFORMS
	UOculusXRHMDRuntimeSettings* HMDSettings = GetMutableDefault<UOculusXRHMDRuntimeSettings>();
	if (FApp::CanEverRender() && HMDSettings->XrApi != EOculusXRXrApi::NativeOpenXR)
	{
		return true;
	}
#endif // OCULUS_HMD_SUPPORTED_PLATFORMS
	return false;
}

uint64 FOculusXRHMDModule::GetGraphicsAdapterLuid()
{
#if OCULUS_HMD_SUPPORTED_PLATFORMS_D3D11 || OCULUS_HMD_SUPPORTED_PLATFORMS_D3D12
	if (!GraphicsAdapterLuid)
	{
		int GraphicsAdapter;

		if (GConfig->GetInt(TEXT("Oculus.Settings"), TEXT("GraphicsAdapter"), GraphicsAdapter, GEngineIni) && GraphicsAdapter >= 0)
		{
			TRefCountPtr<IDXGIFactory> DXGIFactory;
			TRefCountPtr<IDXGIAdapter> DXGIAdapter;
			DXGI_ADAPTER_DESC DXGIAdapterDesc;

			if (SUCCEEDED(CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)DXGIFactory.GetInitReference())) && SUCCEEDED(DXGIFactory->EnumAdapters(GraphicsAdapter, DXGIAdapter.GetInitReference())) && SUCCEEDED(DXGIAdapter->GetDesc(&DXGIAdapterDesc)))
			{
				FMemory::Memcpy(&GraphicsAdapterLuid, &DXGIAdapterDesc.AdapterLuid, sizeof(GraphicsAdapterLuid));
			}
		}
	}
#endif

#if OCULUS_HMD_SUPPORTED_PLATFORMS
	return GraphicsAdapterLuid;
#else
	return 0;
#endif
}

FString FOculusXRHMDModule::GetAudioInputDevice()
{
	FString AudioInputDevice;
#if OCULUS_HMD_SUPPORTED_PLATFORMS
	GConfig->GetString(TEXT("Oculus.Settings"), TEXT("AudioInputDevice"), AudioInputDevice, GEngineIni);
#endif
	return AudioInputDevice;
}

FString FOculusXRHMDModule::GetAudioOutputDevice()
{
	FString AudioOutputDevice;
#if OCULUS_HMD_SUPPORTED_PLATFORMS
	#if PLATFORM_WINDOWS
	if (bPreInit)
	{
		if (FApp::CanEverRender())
		{
			const WCHAR* audioOutDeviceId;
			if (OVRP_SUCCESS(PluginWrapper.GetAudioOutDeviceId2((const void**)&audioOutDeviceId)) && audioOutDeviceId)
			{
				AudioOutputDevice = audioOutDeviceId;
			}
		}
	}
	#else
	GConfig->GetString(TEXT("Oculus.Settings"), TEXT("AudioOutputDevice"), AudioOutputDevice, GEngineIni);
	#endif
#endif
	return AudioOutputDevice;
}

TSharedPtr<class IXRTrackingSystem, ESPMode::ThreadSafe> FOculusXRHMDModule::CreateTrackingSystem()
{
#if OCULUS_HMD_SUPPORTED_PLATFORMS
	if (PreInit())
	{
		OculusXRHMD::FOculusXRHMDPtr OculusXRHMD = FSceneViewExtensions::NewExtension<OculusXRHMD::FOculusXRHMD>();

		if (OculusXRHMD->Startup())
		{
			HeadMountedDisplay = OculusXRHMD;
			return OculusXRHMD;
		}
	}
	HeadMountedDisplay = nullptr;
#endif
	return nullptr;
}

TSharedPtr<IHeadMountedDisplayVulkanExtensions, ESPMode::ThreadSafe> FOculusXRHMDModule::GetVulkanExtensions()
{
#if OCULUS_HMD_SUPPORTED_PLATFORMS
	if (PreInit())
	{
		if (!VulkanExtensions.IsValid())
		{
			VulkanExtensions = MakeShareable(new OculusXRHMD::FVulkanExtensions);
		}
	}
	return VulkanExtensions;
#endif
	return nullptr;
}

FString FOculusXRHMDModule::GetDeviceSystemName()
{
#if OCULUS_HMD_SUPPORTED_PLATFORMS
	ovrpSystemHeadset SystemHeadset;
	if (PluginWrapper.IsInitialized() && OVRP_SUCCESS(PluginWrapper.GetSystemHeadsetType2(&SystemHeadset)))
	{
		switch (SystemHeadset)
		{
			case ovrpSystemHeadset_Oculus_Quest:
				return FString("Oculus Quest");

			case ovrpSystemHeadset_Oculus_Quest_2:
			default:
				return FString("Oculus Quest2");

	#ifdef WITH_OCULUS_BRANCH
			case ovrpSystemHeadset_Meta_Quest_Pro:
				return FString("Meta Quest Pro");
	#endif // WITH_OCULUS_BRANCH
		}
	}
	return FString();
#else
	return FString();
#endif
}

bool FOculusXRHMDModule::IsStandaloneStereoOnlyDevice()
{
#if PLATFORM_ANDROID
	return FAndroidMisc::GetDeviceMake() == FString("Oculus");
#else
	return false;
#endif
}

#if OCULUS_HMD_SUPPORTED_PLATFORMS
void* FOculusXRHMDModule::GetOVRPluginHandle()
{
	void* OVRPluginHandle = nullptr;

	#if PLATFORM_WINDOWS
	FString XrApi;
	if (!GConfig->GetString(TEXT("/Script/OculusXRHMD.OculusXRHMDRuntimeSettings"), TEXT("XrApi"), XrApi, GEngineIni) || XrApi.Equals(FString("OVRPluginOpenXR")))
	{
		FString BinariesPath = FPaths::Combine(IPluginManager::Get().FindPlugin(TEXT("OculusXR"))->GetBaseDir(), TEXT("/Source/ThirdParty/OVRPlugin/OVRPlugin/Lib/Win64"));
		FPlatformProcess::PushDllDirectory(*BinariesPath);
		OVRPluginHandle = FPlatformProcess::GetDllHandle(*(BinariesPath / "OpenXR/OVRPlugin.dll"));
		FPlatformProcess::PopDllDirectory(*BinariesPath);
	}
	#elif PLATFORM_ANDROID
	OVRPluginHandle = FPlatformProcess::GetDllHandle(TEXT("libOVRPlugin.so"));
	#endif // PLATFORM_ANDROID

	return OVRPluginHandle;
}

bool FOculusXRHMDModule::PoseToOrientationAndPosition(const FQuat& InOrientation, const FVector& InPosition, FQuat& OutOrientation, FVector& OutPosition) const
{
	OculusXRHMD::CheckInGameThread();

	OculusXRHMD::FOculusXRHMD* OculusXRHMD = static_cast<OculusXRHMD::FOculusXRHMD*>(HeadMountedDisplay.Pin().Get());

	if (OculusXRHMD)
	{
		ovrpPosef InPose;
		InPose.Orientation = OculusXRHMD::ToOvrpQuatf(InOrientation);
		InPose.Position = OculusXRHMD::ToOvrpVector3f(InPosition);
		OculusXRHMD::FPose OutPose;

		if (OculusXRHMD->ConvertPose(InPose, OutPose))
		{
			OutOrientation = OutPose.Orientation;
			OutPosition = OutPose.Position;
			return true;
		}
	}

	return false;
}

void FOculusXRHMDModule::SetGraphicsAdapterLuid(uint64 InLuid)
{
	GraphicsAdapterLuid = InLuid;

	#if OCULUS_HMD_SUPPORTED_PLATFORMS_D3D11 || OCULUS_HMD_SUPPORTED_PLATFORMS_D3D12
	TRefCountPtr<IDXGIFactory> DXGIFactory;

	if (SUCCEEDED(CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)DXGIFactory.GetInitReference())))
	{
		for (int32 adapterIndex = 0;; adapterIndex++)
		{
			TRefCountPtr<IDXGIAdapter> DXGIAdapter;
			DXGI_ADAPTER_DESC DXGIAdapterDesc;

			if (FAILED(DXGIFactory->EnumAdapters(adapterIndex, DXGIAdapter.GetInitReference())) || FAILED(DXGIAdapter->GetDesc(&DXGIAdapterDesc)))
			{
				break;
			}

			if (!FMemory::Memcmp(&GraphicsAdapterLuid, &DXGIAdapterDesc.AdapterLuid, sizeof(GraphicsAdapterLuid)))
			{
				// Remember this adapterIndex so we use the right adapter, even when we startup without HMD connected
				GConfig->SetInt(TEXT("Oculus.Settings"), TEXT("GraphicsAdapter"), adapterIndex, GEngineIni);
				break;
			}
		}
	}
	#endif // OCULUS_HMD_SUPPORTED_PLATFORMS_D3D11 || OCULUS_HMD_SUPPORTED_PLATFORMS_D3D12
}
#endif // OCULUS_HMD_SUPPORTED_PLATFORMS

IMPLEMENT_MODULE(FOculusXRHMDModule, OculusXRHMD)
