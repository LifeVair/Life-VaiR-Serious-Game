/*
Copyright (c) Meta Platforms, Inc. and affiliates.
All rights reserved.

This source code is licensed under the license found in the
LICENSE file in the root directory of this source tree.
*/

#include "OculusXRSpatialAnchorComponent.h"

DEFINE_LOG_CATEGORY(LogOculusSpatialAnchor);

UOculusXRSpatialAnchorComponent::UOculusXRSpatialAnchorComponent(const FObjectInitializer& ObjectInitializer) 
	: Super(ObjectInitializer)
{
}

bool UOculusXRSpatialAnchorComponent::Create(const FTransform& NewAnchorTransform, AActor* OwningActor, const FOculusXRSpatialAnchorCreateDelegate& Callback)
{
	return OculusXRAnchors::FOculusXRAnchors::CreateSpatialAnchor(NewAnchorTransform, OwningActor, Callback);
}

bool UOculusXRSpatialAnchorComponent::Erase(const FOculusXRAnchorEraseDelegate& Callback)
{
	return OculusXRAnchors::FOculusXRAnchors::EraseAnchor(this, Callback);
}

bool UOculusXRSpatialAnchorComponent::Save(EOculusXRSpaceStorageLocation Location, const FOculusXRAnchorSaveDelegate& Callback)
{
	return OculusXRAnchors::FOculusXRAnchors::SaveAnchor(this, Location, Callback);
}
