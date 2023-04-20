// Copyright Epic Games, Inc. All Rights Reserved.
#include "OculusXRMR_State.h"
#include "OculusXRMRFunctionLibrary.h"

UOculusXRMR_State::UOculusXRMR_State(const FObjectInitializer& ObjectInitializer)
	: TrackedCamera()
	, TrackingReferenceComponent(nullptr)
	, ScalingFactor(1.0f)
	, CurrentCapturingCamera(ovrpCameraDevice_None)
	, ChangeCameraStateRequested(false)
	, BindToTrackedCameraIndexRequested(false)
{
}