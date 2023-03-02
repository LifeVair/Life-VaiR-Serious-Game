/*
Copyright (c) Meta Platforms, Inc. and affiliates.
All rights reserved.

This source code is licensed under the license found in the
LICENSE file in the root directory of this source tree.
*/

#include "OculusXRAnchorLatentActions.h"
#include "OculusXRAnchorsPrivate.h"
#include "OculusXRHMD.h"

//
// Create Spatial Anchor
//
void UOculusXRAsyncAction_CreateSpatialAnchor::Activate()
{
	if (!IsValid(TargetActor))
	{
		UE_LOG(LogOculusXRAnchors, Warning, TEXT("Invalid Target Actor passed to CreateSpatialAnchor latent action."));

		Failure.Broadcast();
		return;
	}

	// Bind the callback delegate and create the anchor
	bool bStartedAsync = OculusXRAnchors::FOculusXRAnchors::CreateSpatialAnchor(
		AnchorTransform,
		TargetActor,
		FOculusXRSpatialAnchorCreateDelegate::CreateUObject(this, &UOculusXRAsyncAction_CreateSpatialAnchor::HandleCreateComplete)
	);

	if (!bStartedAsync)
	{
		UE_LOG(LogOculusXRAnchors, Warning, TEXT("Failed to start async OVR Plugin call for CreateSpatialAnchor latent action."));
		Failure.Broadcast();
	}
}

UOculusXRAsyncAction_CreateSpatialAnchor* UOculusXRAsyncAction_CreateSpatialAnchor::OculusXRAsyncCreateSpatialAnchor(AActor* TargetActor, const FTransform& AnchorTransform)
{
	UOculusXRAsyncAction_CreateSpatialAnchor* Action = NewObject<UOculusXRAsyncAction_CreateSpatialAnchor>();
	Action->TargetActor = TargetActor;
	Action->AnchorTransform = AnchorTransform;
	Action->RegisterWithGameInstance(TargetActor->GetWorld());

	return Action;
}

void UOculusXRAsyncAction_CreateSpatialAnchor::HandleCreateComplete(bool CreateSuccess, UOculusXRAnchorComponent* Anchor)
{
	if (CreateSuccess)
	{
		Success.Broadcast(Anchor);
	}
	else
	{
		Failure.Broadcast();
	}

	SetReadyToDestroy();
}


//
// Erase Space
//
void UOculusXRAsyncAction_EraseAnchor::Activate()
{
	if (!IsValid(TargetActor))
	{
		UE_LOG(LogOculusXRAnchors, Warning, TEXT("Invalid Target Actor passed to EraseSpace latent action."));

		Failure.Broadcast();
		return;
	}

	UOculusXRAnchorComponent* AnchorComponent = TargetActor->FindComponentByClass<UOculusXRAnchorComponent>();
	if (AnchorComponent == nullptr)
	{
		UE_LOG(LogOculusXRAnchors, Warning, TEXT("No anchor on actor in EraseSpace latent action."));

		Failure.Broadcast();
		return;
	}

	// Bind the callback delegate and start delete
	bool bStartedAsync = OculusXRAnchors::FOculusXRAnchors::EraseAnchor(
		AnchorComponent,
		FOculusXRAnchorEraseDelegate::CreateUObject(this, &UOculusXRAsyncAction_EraseAnchor::HandleEraseAnchorComplete)
	);

	if (!bStartedAsync)
	{
		UE_LOG(LogOculusXRAnchors, Warning, TEXT("Failed to start async OVR Plugin call for EraseSpace latent action."));

		Failure.Broadcast();
	}
}

UOculusXRAsyncAction_EraseAnchor* UOculusXRAsyncAction_EraseAnchor::OculusXRAsyncEraseAnchor(AActor* TargetActor)
{
	UOculusXRAsyncAction_EraseAnchor* Action = NewObject<UOculusXRAsyncAction_EraseAnchor>();
	Action->TargetActor = TargetActor;

	if (IsValid(TargetActor))
	{
		Action->RegisterWithGameInstance(TargetActor->GetWorld());
	}

	return Action;
}

void UOculusXRAsyncAction_EraseAnchor::HandleEraseAnchorComplete(bool EraseSuccess, FOculusXRUUID UUID)
{
	if (EraseSuccess)
	{
		Success.Broadcast(TargetActor, UUID);
	}
	else
	{
		Failure.Broadcast();
	}

	SetReadyToDestroy();
}


//
// Save Space
//
void UOculusXRAsyncAction_SaveAnchor::Activate()
{
	if (!IsValid(TargetActor))
	{
		UE_LOG(LogOculusXRAnchors, Warning, TEXT("Invalid Target Actor passed to SaveSpace latent action."));

		Failure.Broadcast();
		return;
	}

	UOculusXRAnchorComponent* AnchorComponent = TargetActor->FindComponentByClass<UOculusXRAnchorComponent>();
	if (AnchorComponent == nullptr)
	{
		Failure.Broadcast();
		return;
	}

	UE_LOG(LogOculusXRAnchors, Log, TEXT("Attempting to save anchor: %s to location %s"), IsValid(AnchorComponent) ? *AnchorComponent->GetName() : TEXT("INVALID ANCHOR"), *UEnum::GetValueAsString(StorageLocation));

	// Bind the callback delegate and start save
	bool bStartedAsync = OculusXRAnchors::FOculusXRAnchors::SaveAnchor(
		AnchorComponent,
		StorageLocation,
		FOculusXRAnchorSaveDelegate::CreateUObject(this, &UOculusXRAsyncAction_SaveAnchor::HandleSaveAnchorComplete));

	if (!bStartedAsync)
	{
		UE_LOG(LogOculusXRAnchors, Warning, TEXT("Failed to start async OVR Plugin call for SaveSpace latent action."));
		Failure.Broadcast();
	}
}

UOculusXRAsyncAction_SaveAnchor* UOculusXRAsyncAction_SaveAnchor::OculusXRAsyncSaveAnchor(AActor* TargetActor, EOculusXRSpaceStorageLocation StorageLocation)
{
	UOculusXRAsyncAction_SaveAnchor* Action = NewObject<UOculusXRAsyncAction_SaveAnchor>();
	Action->TargetActor = TargetActor;
	Action->StorageLocation = StorageLocation;
	Action->RegisterWithGameInstance(TargetActor->GetWorld());

	return Action;
}

void UOculusXRAsyncAction_SaveAnchor::HandleSaveAnchorComplete(bool SaveSuccess, UOculusXRAnchorComponent* Anchor)
{
	if (SaveSuccess)
	{
		Success.Broadcast(Anchor);
	}
	else
	{
		Failure.Broadcast();
	}

	SetReadyToDestroy();
}


//
// Query Spaces
//
void UOculusXRAsyncAction_QueryAnchors::Activate()
{
	// Bind the callback delegates and start the query
	bool bStartedAsync = OculusXRAnchors::FOculusXRAnchors::QueryAnchorsAdvanced(
		QueryInfo,
		FOculusXRAnchorQueryDelegate::CreateUObject(this, &UOculusXRAsyncAction_QueryAnchors::HandleQueryAnchorsResults));

	if (!bStartedAsync)
	{
		UE_LOG(LogOculusXRAnchors, Warning, TEXT("Failed to start async OVR Plugin call for QuerySpaces latent action."));

		Failure.Broadcast();
	}
}

UOculusXRAsyncAction_QueryAnchors* UOculusXRAsyncAction_QueryAnchors::OculusXRAsyncQueryAnchors(EOculusXRSpaceStorageLocation Location, const TArray<FOculusXRUUID>& UUIDs, int32 MaxAnchors)
{
	FOculusXRSpaceQueryInfo QueryInfo;
	QueryInfo.FilterType = EOculusXRSpaceQueryFilterType::FilterByIds;
	QueryInfo.IDFilter = UUIDs;
	QueryInfo.Location = Location;
	QueryInfo.MaxQuerySpaces = MaxAnchors;

	UOculusXRAsyncAction_QueryAnchors* Action = NewObject<UOculusXRAsyncAction_QueryAnchors>();
	Action->QueryInfo = QueryInfo;

	return Action;
}

UOculusXRAsyncAction_QueryAnchors* UOculusXRAsyncAction_QueryAnchors::OculusXRAsyncQueryAnchorsAdvanced(const FOculusXRSpaceQueryInfo& QueryInfo)
{
	UOculusXRAsyncAction_QueryAnchors* Action = NewObject<UOculusXRAsyncAction_QueryAnchors>();
	Action->QueryInfo = QueryInfo;

	return Action;
}

void UOculusXRAsyncAction_QueryAnchors::HandleQueryAnchorsResults(bool QuerySuccess, const TArray<FOculusXRSpaceQueryResult>& Results)
{
	QueryResults = Results;

	if (QuerySuccess)
	{
		Success.Broadcast(QueryResults);
	}
	else
	{
		Failure.Broadcast();
	}

	SetReadyToDestroy();
}


//
// Set Component Status
//
void UOculusXRAsyncAction_SetAnchorComponentStatus::Activate()
{
	if (!IsValid(TargetActor))
	{
		UE_LOG(LogOculusXRAnchors, Warning, TEXT("Invalid Target Actor passed to SetComponentStatus latent action."));

		Failure.Broadcast();
		return;
	}

	UOculusXRAnchorComponent* AnchorComponent = TargetActor->FindComponentByClass<UOculusXRAnchorComponent>();
	if (AnchorComponent == nullptr)
	{
		Failure.Broadcast();
		return;
	}

	// Bind the callback delegates and start the query
	bool bStartedAsync = OculusXRAnchors::FOculusXRAnchors::SetAnchorComponentStatus(
		AnchorComponent,
		ComponentType,
		bEnabled,
		0,
		FOculusXRAnchorSetComponentStatusDelegate::CreateUObject(this, &UOculusXRAsyncAction_SetAnchorComponentStatus::HandleSetComponentStatusComplete));

	if (!bStartedAsync)
	{
		UE_LOG(LogOculusXRAnchors, Warning, TEXT("Failed to start async OVR Plugin call for SetComponentStatus latent action."));

		Failure.Broadcast();
	}
}

UOculusXRAsyncAction_SetAnchorComponentStatus* UOculusXRAsyncAction_SetAnchorComponentStatus::OculusXRAsyncSetAnchorComponentStatus(AActor* TargetActor, EOculusXRSpaceComponentType ComponentType, bool bEnabled)
{
	UOculusXRAsyncAction_SetAnchorComponentStatus* Action = NewObject<UOculusXRAsyncAction_SetAnchorComponentStatus>();
	Action->TargetActor = TargetActor;
	Action->ComponentType = ComponentType;
	Action->bEnabled = bEnabled;
	Action->RegisterWithGameInstance(TargetActor);

	return Action;
}

void UOculusXRAsyncAction_SetAnchorComponentStatus::HandleSetComponentStatusComplete(bool SetStatusSuccess, UOculusXRAnchorComponent* Anchor, EOculusXRSpaceComponentType SpaceComponentType, bool bResultEnabled)
{
	if (SetStatusSuccess)
	{
		Success.Broadcast(Anchor, SpaceComponentType, bResultEnabled);
	}
	else
	{
		Failure.Broadcast();
	}

	SetReadyToDestroy();
}
