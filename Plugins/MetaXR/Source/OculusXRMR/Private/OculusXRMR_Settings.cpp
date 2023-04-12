// Copyright Epic Games, Inc. All Rights Reserved.

#include "OculusXRMR_Settings.h"
#include "OculusXRMRPrivate.h"
#include "OculusXRHMD.h"
#include "Engine/Engine.h"

UOculusXRMR_Settings::UOculusXRMR_Settings(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, ClippingReference(EOculusXRMR_ClippingReference::CR_Head)
	, bUseTrackedCameraResolution(true)
	, WidthPerView(960)
	, HeightPerView(540)
	, CastingLatency(0.0f)
	, BackdropColor(FColor::Green)
	, HandPoseStateLatency(0.0f)
	, ChromaKeyColor(FColor::Green)
	, ChromaKeySimilarity(0.6f)
	, ChromaKeySmoothRange(0.03f)
	, ChromaKeySpillRange(0.04f)
	, ExternalCompositionPostProcessEffects(EOculusXRMR_PostProcessEffects::PPE_Off)
	, bIsCasting(false)
	, CompositionMethod(EOculusXRMR_CompositionMethod::ExternalComposition)
	, CapturingCamera(EOculusXRMR_CameraDeviceEnum::CD_WebCamera0)
	, BindToTrackedCameraIndex(-1)
{
}

void UOculusXRMR_Settings::SetCompositionMethod(EOculusXRMR_CompositionMethod val)
{
	if (CompositionMethod == val)
	{
		return;
	}
	auto old = CompositionMethod;
	CompositionMethod = val;
	CompositionMethodChangeDelegate.Execute(old, val);
}

void UOculusXRMR_Settings::SetCapturingCamera(EOculusXRMR_CameraDeviceEnum val)
{
	if (CapturingCamera == val)
	{
		return;
	}
	auto old = CapturingCamera;
	CapturingCamera = val;
	CapturingCameraChangeDelegate.Execute(old, val);
}

void UOculusXRMR_Settings::SetIsCasting(bool val)
{
	if (bIsCasting == val)
	{
		return;
	}
	auto old = bIsCasting;
	bIsCasting = val;
	IsCastingChangeDelegate.Execute(old, val);
}

void UOculusXRMR_Settings::BindToTrackedCameraIndexIfAvailable(int InTrackedCameraIndex)
{
	if (BindToTrackedCameraIndex == InTrackedCameraIndex)
	{
		return;
	}
	auto old = BindToTrackedCameraIndex;
	BindToTrackedCameraIndex = InTrackedCameraIndex;
	TrackedCameraIndexChangeDelegate.Execute(old, InTrackedCameraIndex);
}

void UOculusXRMR_Settings::LoadFromIni()
{
	if (!GConfig)
	{
		UE_LOG(LogMR, Warning, TEXT("GConfig is NULL"));
		return;
	}

	// Flushing the GEngineIni is necessary to get the settings reloaded at the runtime, but the manual flushing
	// could cause an assert when loading audio settings if launching through editor at the 2nd time. Disabled temporarily.
	//GConfig->Flush(true, GEngineIni);

	const TCHAR* OculusXRMRSettings = TEXT("Oculus.Settings.MixedReality");
	bool v;
	float f;
	int32 i;
	FVector vec;
	FColor color;
	if (GConfig->GetInt(OculusXRMRSettings, TEXT("CompositionMethod"), i, GEngineIni))
	{
		SetCompositionMethod((EOculusXRMR_CompositionMethod)i);
	}
	if (GConfig->GetInt(OculusXRMRSettings, TEXT("ClippingReference"), i, GEngineIni))
	{
		ClippingReference = (EOculusXRMR_ClippingReference)i;
	}
	if (GConfig->GetBool(OculusXRMRSettings, TEXT("bUseTrackedCameraResolution"), v, GEngineIni))
	{
		bUseTrackedCameraResolution = v;
	}
	if (GConfig->GetInt(OculusXRMRSettings, TEXT("WidthPerView"), i, GEngineIni))
	{
		WidthPerView = i;
	}
	if (GConfig->GetInt(OculusXRMRSettings, TEXT("HeightPerView"), i, GEngineIni))
	{
		HeightPerView = i;
	}
	if (GConfig->GetInt(OculusXRMRSettings, TEXT("CapturingCamera"), i, GEngineIni))
	{
		CapturingCamera = (EOculusXRMR_CameraDeviceEnum)i;
	}
	if (GConfig->GetFloat(OculusXRMRSettings, TEXT("CastingLatency"), f, GEngineIni))
	{
		CastingLatency = f;
	}
	if (GConfig->GetColor(OculusXRMRSettings, TEXT("BackdropColor"), color, GEngineIni))
	{
		BackdropColor = color;
	}
	if (GConfig->GetFloat(OculusXRMRSettings, TEXT("HandPoseStateLatency"), f, GEngineIni))
	{
		HandPoseStateLatency = f;
	}
	if (GConfig->GetColor(OculusXRMRSettings, TEXT("ChromaKeyColor"), color, GEngineIni))
	{
		ChromaKeyColor = color;
	}
	if (GConfig->GetFloat(OculusXRMRSettings, TEXT("ChromaKeySimilarity"), f, GEngineIni))
	{
		ChromaKeySimilarity = f;
	}
	if (GConfig->GetFloat(OculusXRMRSettings, TEXT("ChromaKeySmoothRange"), f, GEngineIni))
	{
		ChromaKeySmoothRange = f;
	}
	if (GConfig->GetFloat(OculusXRMRSettings, TEXT("ChromaKeySpillRange"), f, GEngineIni))
	{
		ChromaKeySpillRange = f;
	}
	if (GConfig->GetInt(OculusXRMRSettings, TEXT("BindToTrackedCameraIndex"), i, GEngineIni))
	{
		BindToTrackedCameraIndexIfAvailable(i);
	}
	if (GConfig->GetInt(OculusXRMRSettings, TEXT("ExternalCompositionPostProcessEffects"), i, GEngineIni))
	{
		ExternalCompositionPostProcessEffects = (EOculusXRMR_PostProcessEffects)i;
	}

	UE_LOG(LogMR, Log, TEXT("MixedReality settings loaded from Engine.ini"));
}

void UOculusXRMR_Settings::SaveToIni() const
{
	if (!GConfig)
	{
		UE_LOG(LogMR, Warning, TEXT("GConfig is NULL"));
		return;
	}

	const TCHAR* OculusXRMRSettings = TEXT("Oculus.Settings.MixedReality");
	GConfig->SetInt(OculusXRMRSettings, TEXT("CompositionMethod"), (int32)CompositionMethod, GEngineIni);
	GConfig->SetInt(OculusXRMRSettings, TEXT("ClippingReference"), (int32)ClippingReference, GEngineIni);
	GConfig->SetBool(OculusXRMRSettings, TEXT("bUseTrackedCameraResolution"), bUseTrackedCameraResolution, GEngineIni);
	GConfig->SetInt(OculusXRMRSettings, TEXT("WidthPerView"), WidthPerView, GEngineIni);
	GConfig->SetInt(OculusXRMRSettings, TEXT("HeightPerView"), HeightPerView, GEngineIni);
	GConfig->SetInt(OculusXRMRSettings, TEXT("CapturingCamera"), (int32)CapturingCamera, GEngineIni);
	GConfig->SetFloat(OculusXRMRSettings, TEXT("CastingLatency"), CastingLatency, GEngineIni);
	GConfig->SetColor(OculusXRMRSettings, TEXT("BackdropColor"), BackdropColor, GEngineIni);
	GConfig->SetFloat(OculusXRMRSettings, TEXT("HandPoseStateLatency"), HandPoseStateLatency, GEngineIni);
	GConfig->SetColor(OculusXRMRSettings, TEXT("ChromaKeyColor"), ChromaKeyColor, GEngineIni);
	GConfig->SetFloat(OculusXRMRSettings, TEXT("ChromaKeySimilarity"), ChromaKeySimilarity, GEngineIni);
	GConfig->SetFloat(OculusXRMRSettings, TEXT("ChromaKeySmoothRange"), ChromaKeySmoothRange, GEngineIni);
	GConfig->SetFloat(OculusXRMRSettings, TEXT("ChromaKeySpillRange"), ChromaKeySpillRange, GEngineIni);
	GConfig->SetInt(OculusXRMRSettings, TEXT("BindToTrackedCameraIndex"), (int32)BindToTrackedCameraIndex, GEngineIni);
	GConfig->SetInt(OculusXRMRSettings, TEXT("ExternalCompositionPostProcessEffects"), (int32)ExternalCompositionPostProcessEffects, GEngineIni);

	GConfig->Flush(false, GEngineIni);

	UE_LOG(LogMR, Log, TEXT("MixedReality settings saved to Engine.ini"));
}
