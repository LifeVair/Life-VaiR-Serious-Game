/*
Copyright (c) Meta Platforms, Inc. and affiliates.
All rights reserved.

This source code is licensed under the license found in the
LICENSE file in the root directory of this source tree.
*/

#include "OculusXRAnchors.h"
#include "OculusXRAnchorsModule.h"
#include "OculusXRAnchorDelegates.h"
#include "OculusXRHMDModule.h"
#include "OculusXRAnchorManager.h"
#include "OculusXRSpatialAnchorComponent.h"

namespace OculusXRAnchors
{

void FOculusXRAnchors::Initialize()
{
	DelegateHandleAnchorCreate = FOculusXRAnchorEventDelegates::OculusSpatialAnchorCreateComplete.AddRaw(this, &FOculusXRAnchors::HandleSpatialAnchorCreateComplete);
	DelegateHandleAnchorErase = FOculusXRAnchorEventDelegates::OculusSpaceEraseComplete.AddRaw(this, &FOculusXRAnchors::HandleAnchorEraseComplete);
	DelegateHandleSetComponentStatus = FOculusXRAnchorEventDelegates::OculusSpaceSetComponentStatusComplete.AddRaw(this, &FOculusXRAnchors::HandleSetComponentStatusComplete);
	DelegateHandleAnchorSave = FOculusXRAnchorEventDelegates::OculusSpaceSaveComplete.AddRaw(this, &FOculusXRAnchors::HandleAnchorSaveComplete);
	DelegateHandleQueryResultsBegin = FOculusXRAnchorEventDelegates::OculusSpaceQueryResults.AddRaw(this, &FOculusXRAnchors::HandleAnchorQueryResultsBegin);
	DelegateHandleQueryResultElement = FOculusXRAnchorEventDelegates::OculusSpaceQueryResult.AddRaw(this, &FOculusXRAnchors::HandleAnchorQueryResultElement);
	DelegateHandleQueryComplete = FOculusXRAnchorEventDelegates::OculusSpaceQueryComplete.AddRaw(this, &FOculusXRAnchors::HandleAnchorQueryComplete);
}

void FOculusXRAnchors::Teardown()
{
	FOculusXRAnchorEventDelegates::OculusSpatialAnchorCreateComplete.Remove(DelegateHandleAnchorCreate);
	FOculusXRAnchorEventDelegates::OculusSpaceEraseComplete.Remove(DelegateHandleAnchorErase);
	FOculusXRAnchorEventDelegates::OculusSpaceSetComponentStatusComplete.Remove(DelegateHandleSetComponentStatus);
	FOculusXRAnchorEventDelegates::OculusSpaceSaveComplete.Remove(DelegateHandleAnchorSave);
	FOculusXRAnchorEventDelegates::OculusSpaceQueryResults.Remove(DelegateHandleQueryResultsBegin);
	FOculusXRAnchorEventDelegates::OculusSpaceQueryResult.Remove(DelegateHandleQueryResultElement);
	FOculusXRAnchorEventDelegates::OculusSpaceQueryComplete.Remove(DelegateHandleQueryComplete);
}

FOculusXRAnchors* FOculusXRAnchors::GetInstance()
{
	return FOculusXRAnchorsModule::GetOculusAnchors();
}

bool FOculusXRAnchors::CreateSpatialAnchor(const FTransform& InTransform, AActor* TargetActor, const FOculusXRSpatialAnchorCreateDelegate& ResultCallback)
{
	if (!IsValid(TargetActor))
	{
		UE_LOG(LogOculusXRAnchors, Warning, TEXT("Invalid actor provided when attempting to create a spatial anchor."));
		return false;
	}

	UWorld* World = TargetActor->GetWorld();
	if (!IsValid(World))
	{
		UE_LOG(LogOculusXRAnchors, Warning, TEXT("Unable to retrieve World Context while creating spatial anchor."));
		return false;
	}

	APlayerController* PlayerController = World->GetFirstPlayerController();
	if (!IsValid(PlayerController))
	{
		UE_LOG(LogOculusXRAnchors, Warning, TEXT("Unable to retrieve Player Controller while creating spatial anchor"));
		return false;
	}

	APlayerCameraManager* PlayerCameraManager = PlayerController->PlayerCameraManager;
	FTransform MainCameraTransform = FTransform::Identity;
	if (IsValid(PlayerCameraManager))
	{
		MainCameraTransform.SetLocation(PlayerCameraManager->GetCameraLocation());
		MainCameraTransform.SetRotation(FQuat(PlayerCameraManager->GetCameraRotation()));
	}

	UOculusXRAnchorComponent* Anchor = Cast<UOculusXRAnchorComponent>(TargetActor->GetComponentByClass(UOculusXRAnchorComponent::StaticClass()));
	if (IsValid(Anchor) && Anchor->HasValidHandle())
	{
		UE_LOG(LogOculusXRAnchors, Warning, TEXT("Actor targeted to create anchor already has an anchor component with a valid handle."));
		return false;
	}

	uint64 RequestId = 0;
	bool started = FOculusXRAnchorManager::CreateAnchor(InTransform, RequestId, MainCameraTransform);
	if (!started)
	{
		UE_LOG(LogOculusXRAnchors, Warning, TEXT("Failed to start async call to create anchor."));
		return false;
	}

	CreateAnchorBinding AnchorData;
	AnchorData.RequestId = RequestId;
	AnchorData.Actor = TargetActor;
	AnchorData.Binding = ResultCallback;

	FOculusXRAnchors* SDKInstance = GetInstance();
	SDKInstance->CreateSpatialAnchorBindings.Add(RequestId, AnchorData);

	return true;
}

bool FOculusXRAnchors::EraseAnchor(UOculusXRAnchorComponent* Anchor, const FOculusXRAnchorEraseDelegate& ResultCallback)
{
	if (!IsValid(Anchor))
	{
		UE_LOG(LogOculusXRAnchors, Warning, TEXT("Invalid anchor provided when attempting to erase an anchor."));
		return false;
	}

	if (!Anchor->HasValidHandle())
	{
		UE_LOG(LogOculusXRAnchors, Warning, TEXT("Cannot erase anchor with invalid handle."));
		return false;
	}

	if (!Anchor->IsStoredAtLocation(EOculusXRSpaceStorageLocation::Local))
	{
		UE_LOG(LogOculusXRAnchors, Warning, TEXT("Only local anchors can be erased."));
		return false;
	}

	uint64 RequestId = 0;

	// Erase only supports local anchors
	bool started = FOculusXRAnchorManager::EraseAnchor(Anchor->GetHandle(), EOculusXRSpaceStorageLocation::Local, RequestId);
	if (!started)
	{
		UE_LOG(LogOculusXRAnchors, Warning, TEXT("Failed to start async erase."));
		return false;
	}

	EraseAnchorBinding EraseData;
	EraseData.RequestId = RequestId;
	EraseData.Binding = ResultCallback;
	EraseData.Anchor = Anchor;

	FOculusXRAnchors* SDKInstance = GetInstance();
	SDKInstance->EraseAnchorBindings.Add(RequestId, EraseData);

	return true;
}

bool FOculusXRAnchors::DestroyAnchor(uint64 AnchorHandle)
{
	return FOculusXRAnchorManager::DestroySpace(AnchorHandle);
}

bool FOculusXRAnchors::SetAnchorComponentStatus(UOculusXRAnchorComponent* Anchor, EOculusXRSpaceComponentType SpaceComponentType, bool Enable, float Timeout, const FOculusXRAnchorSetComponentStatusDelegate& ResultCallback)
{
	if (!IsValid(Anchor))
	{
		UE_LOG(LogOculusXRAnchors, Warning, TEXT("Invalid anchor provided when attempting to set anchor component status."));
		return false;
	}

	if (!Anchor->HasValidHandle())
	{
		UE_LOG(LogOculusXRAnchors, Warning, TEXT("Anchor provided to set anchor component status has invalid handle."));
		return false;
	}

	uint64 RequestId = 0;
	bool started = FOculusXRAnchorManager::SetSpaceComponentStatus(Anchor->GetHandle(), SpaceComponentType, Enable, Timeout, RequestId);
	if (!started)
	{
		UE_LOG(LogOculusXRAnchors, Warning, TEXT("Failed to start async call to set anchor component status."));
		return false;
	}

	SetComponentStatusBinding SetComponentStatusData;
	SetComponentStatusData.RequestId = RequestId;
	SetComponentStatusData.Binding = ResultCallback;
	SetComponentStatusData.Anchor = Anchor;

	FOculusXRAnchors* SDKInstance = GetInstance();
	SDKInstance->SetComponentStatusBindings.Add(RequestId, SetComponentStatusData);

	return true;
}

bool FOculusXRAnchors::GetAnchorComponentStatus(UOculusXRAnchorComponent* Anchor, EOculusXRSpaceComponentType SpaceComponentType, bool& OutEnabled, bool& OutChangePending)
{
	if (!IsValid(Anchor))
	{
		UE_LOG(LogOculusXRAnchors, Warning, TEXT("Invalid anchor provided when attempting to get space component status."));
		return false;
	}

	if (!Anchor->HasValidHandle())
	{
		UE_LOG(LogOculusXRAnchors, Warning, TEXT("Anchor provided to get space component status has invalid handle."));
		return false;
	}

	return FOculusXRAnchorManager::GetSpaceComponentStatus(Anchor->GetHandle(), SpaceComponentType, OutEnabled, OutChangePending);
}

bool FOculusXRAnchors::SaveAnchor(UOculusXRAnchorComponent* Anchor, EOculusXRSpaceStorageLocation StorageLocation, const FOculusXRAnchorSaveDelegate& ResultCallback)
{
	if (!IsValid(Anchor))
	{
		UE_LOG(LogOculusXRAnchors, Warning, TEXT("Invalid anchor provided when attempting to save anchor."));
		return false;
	}

	if (!Anchor->HasValidHandle())
	{
		UE_LOG(LogOculusXRAnchors, Warning, TEXT("Anchor provided to save anchor has invalid handle."));
		return false;
	}

	uint64 RequestId = 0;
	bool started = FOculusXRAnchorManager::SaveAnchor(Anchor->GetHandle(), StorageLocation, EOculusXRSpaceStoragePersistenceMode::Indefinite, RequestId);
	if (!started)
	{
		UE_LOG(LogOculusXRAnchors, Warning, TEXT("Failed to start async call to save anchor."));
		return false;
	}

	SaveAnchorBinding SaveAnchorData;
	SaveAnchorData.RequestId = RequestId;
	SaveAnchorData.Binding = ResultCallback;
	SaveAnchorData.Location = StorageLocation;
	SaveAnchorData.Anchor = Anchor;

	FOculusXRAnchors* SDKInstance = GetInstance();
	SDKInstance->AnchorSaveBindings.Add(RequestId, SaveAnchorData);

	return true;
}

bool FOculusXRAnchors::QueryAnchors(const TArray<FOculusXRUUID>& AnchorUUIDs, EOculusXRSpaceStorageLocation Location, int32 MaxAnchors, const FOculusXRAnchorQueryDelegate& ResultCallback)
{
	FOculusXRSpaceQueryInfo QueryInfo;
	QueryInfo.FilterType = EOculusXRSpaceQueryFilterType::FilterByIds;
	QueryInfo.IDFilter = AnchorUUIDs;
	QueryInfo.Location = Location;
	QueryInfo.MaxQuerySpaces = MaxAnchors;

	return QueryAnchorsAdvanced(QueryInfo, ResultCallback);
}

bool FOculusXRAnchors::QueryAnchorsAdvanced(const FOculusXRSpaceQueryInfo& QueryInfo, const FOculusXRAnchorQueryDelegate& ResultCallback)
{
	uint64 RequestId = 0;
	bool started = FOculusXRAnchorManager::QuerySpaces(QueryInfo, RequestId);
	if (!started)
	{
		ResultCallback.ExecuteIfBound(false, TArray<FOculusXRSpaceQueryResult>());
		return false;
	}

	AnchorQueryBinding QueryResults;
	QueryResults.RequestId = RequestId;
	QueryResults.Binding = ResultCallback;
	QueryResults.Location = QueryInfo.Location;

	FOculusXRAnchors* SDKInstance = GetInstance();
	SDKInstance->AnchorQueryBindings.Add(RequestId, QueryResults);

	return true;
}

bool FOculusXRAnchors::GetSpaceScenePlane(uint64 Space, FVector& OutPos, FVector& OutSize)
{
	return FOculusXRAnchorManager::GetSpaceScenePlane(Space, OutPos, OutSize);
}

bool FOculusXRAnchors::GetSpaceSceneVolume(uint64 Space, FVector& OutPos, FVector& OutSize)
{
	return FOculusXRAnchorManager::GetSpaceSceneVolume(Space, OutPos, OutSize);
}

bool FOculusXRAnchors::GetSpaceSemanticClassification(uint64 Space, TArray<FString>& OutSemanticClassifications)
{
	return FOculusXRAnchorManager::GetSpaceSemanticClassification(Space, OutSemanticClassifications);
}

void FOculusXRAnchors::HandleSpatialAnchorCreateComplete(FOculusXRUInt64 RequestId, int Result, FOculusXRUInt64 Space, FOculusXRUUID UUID)
{
	CreateAnchorBinding* AnchorDataPtr = CreateSpatialAnchorBindings.Find(RequestId.GetValue());
	if (AnchorDataPtr == nullptr)
	{
		UE_LOG(LogOculusXRAnchors, Error, TEXT("Couldn't find anchor data binding for create spatial anchor! Request: %llu"), RequestId.GetValue());
		return;
	}

	if (!OVRP_SUCCESS(Result))
	{
		UE_LOG(LogOculusXRAnchors, Warning, TEXT("Failed to create Spatial Anchor. Request: %llu  --  Result: %d"), RequestId.GetValue(), Result);
		AnchorDataPtr->Binding.ExecuteIfBound(false, nullptr);
		return;
	}

	if (!AnchorDataPtr->Actor.IsValid())
	{
		UE_LOG(LogOculusXRAnchors, Warning, TEXT("Actor has been invalidated while creating actor. Request: %llu"), RequestId.GetValue());

		// Clean up the orphaned space
		FOculusXRAnchors::DestroyAnchor(Space);

		AnchorDataPtr->Binding.ExecuteIfBound(false, nullptr);
		return;
	}

	AActor* TargetActor = AnchorDataPtr->Actor.Get();

	UOculusXRSpatialAnchorComponent* SpatialAnchorComponent = TargetActor->FindComponentByClass<UOculusXRSpatialAnchorComponent>();
	if (SpatialAnchorComponent == nullptr)
	{
		SpatialAnchorComponent = Cast<UOculusXRSpatialAnchorComponent>(TargetActor->AddComponentByClass(UOculusXRSpatialAnchorComponent::StaticClass(), false, FTransform::Identity, false));
	}

	SpatialAnchorComponent->SetHandle(Space);
	SpatialAnchorComponent->SetUUID(UUID);

	uint64 tempOut;
	FOculusXRAnchorManager::SetSpaceComponentStatus(Space, EOculusXRSpaceComponentType::Locatable, true, 0.0f, tempOut);
	FOculusXRAnchorManager::SetSpaceComponentStatus(Space, EOculusXRSpaceComponentType::Storable, true, 0.0f, tempOut);

	AnchorDataPtr->Binding.ExecuteIfBound(OVRP_SUCCESS(Result), SpatialAnchorComponent);
	CreateSpatialAnchorBindings.Remove(RequestId.GetValue());
}

void FOculusXRAnchors::HandleAnchorEraseComplete(FOculusXRUInt64 RequestId, int Result, FOculusXRUUID UUID, EOculusXRSpaceStorageLocation Location)
{
	EraseAnchorBinding* EraseDataPtr = EraseAnchorBindings.Find(RequestId.GetValue());
	if (EraseDataPtr == nullptr)
	{
		UE_LOG(LogOculusXRAnchors, Error, TEXT("Couldn't find binding for space erase! Request: %llu"), RequestId.GetValue());
		return;
	}

	if (!OVRP_SUCCESS(Result))
	{
		UE_LOG(LogOculusXRAnchors, Warning, TEXT("Failed to erase Spatial Anchor. Request: %llu  --  Result: %d"), RequestId.GetValue(), Result);
		EraseDataPtr->Binding.ExecuteIfBound(false, UUID);
		return;
	}

	if (EraseDataPtr->Anchor.IsValid())
	{
		// Since you can only erase local anchors, just unset local anchor storage
		EraseDataPtr->Anchor->SetStoredLocation(EOculusXRSpaceStorageLocation::Local, false);
	}

	EraseDataPtr->Binding.ExecuteIfBound(OVRP_SUCCESS(Result), UUID);
	EraseAnchorBindings.Remove(RequestId.GetValue());
}

void FOculusXRAnchors::HandleSetComponentStatusComplete(FOculusXRUInt64 RequestId, int Result, FOculusXRUInt64 Space, FOculusXRUUID UUID, EOculusXRSpaceComponentType ComponentType, bool Enabled)
{
	SetComponentStatusBinding* SetStatusBinding = SetComponentStatusBindings.Find(RequestId.GetValue());
	if (SetStatusBinding == nullptr)
	{
		UE_LOG(LogOculusXRAnchors, Verbose, TEXT("Couldn't find binding for set component status! Request: %llu"), RequestId.GetValue());
		return;
	}

	SetStatusBinding->Binding.ExecuteIfBound(OVRP_SUCCESS(Result), SetStatusBinding->Anchor.Get(), ComponentType, Enabled);
	SetComponentStatusBindings.Remove(RequestId.GetValue());
}

void FOculusXRAnchors::HandleAnchorSaveComplete(FOculusXRUInt64 RequestId, FOculusXRUInt64 Space, bool Success, int Result, FOculusXRUUID UUID)
{
	SaveAnchorBinding* SaveAnchorData = AnchorSaveBindings.Find(RequestId.GetValue());
	if (SaveAnchorData == nullptr)
	{
		UE_LOG(LogOculusXRAnchors, Error, TEXT("Couldn't find binding for save anchor! Request: %llu"), RequestId.GetValue());
		return;
	}

	if (!OVRP_SUCCESS(Result))
	{
		UE_LOG(LogOculusXRAnchors, Warning, TEXT("Failed to save Spatial Anchor. Request: %llu  --  Result: %d  --  Space: %llu"), RequestId.GetValue(), Result, Space.GetValue());
		SaveAnchorData->Binding.ExecuteIfBound(false, SaveAnchorData->Anchor.Get());
		return;
	}

	if (SaveAnchorData->Anchor.IsValid())
	{
		SaveAnchorData->Anchor->SetStoredLocation(SaveAnchorData->Location, true);
	}

	SaveAnchorData->Binding.ExecuteIfBound(Success, SaveAnchorData->Anchor.Get());
	AnchorSaveBindings.Remove(RequestId.GetValue());
}

void FOculusXRAnchors::HandleAnchorQueryResultsBegin(FOculusXRUInt64 RequestId)
{
	// no op
}

void FOculusXRAnchors::HandleAnchorQueryResultElement(FOculusXRUInt64 RequestId, FOculusXRUInt64 Space, FOculusXRUUID UUID)
{
	AnchorQueryBinding* ResultPtr = AnchorQueryBindings.Find(RequestId.GetValue());
	if (ResultPtr)
	{
		uint64 tempOut;
		FOculusXRAnchorManager::SetSpaceComponentStatus(Space, EOculusXRSpaceComponentType::Locatable, true, 0.0f, tempOut);
		FOculusXRAnchorManager::SetSpaceComponentStatus(Space, EOculusXRSpaceComponentType::Storable, true, 0.0f, tempOut);

		ResultPtr->Results.Add(FOculusXRSpaceQueryResult(Space, UUID, ResultPtr->Location));
	}
}

void FOculusXRAnchors::HandleAnchorQueryComplete(FOculusXRUInt64 RequestId, bool Success)
{
	AnchorQueryBinding* ResultPtr = AnchorQueryBindings.Find(RequestId.GetValue());
	if (ResultPtr)
	{
		ResultPtr->Binding.ExecuteIfBound(Success, ResultPtr->Results);
		AnchorQueryBindings.Remove(RequestId.GetValue());
	}
}

}