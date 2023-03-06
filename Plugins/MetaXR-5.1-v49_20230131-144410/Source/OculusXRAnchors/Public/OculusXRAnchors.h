/*
Copyright (c) Meta Platforms, Inc. and affiliates.
All rights reserved.

This source code is licensed under the license found in the
LICENSE file in the root directory of this source tree.
*/
#pragma once

#include "CoreMinimal.h"
#include "OculusXRAnchorComponent.h"
#include "OculusXRAnchorTypes.h"

DECLARE_DELEGATE_TwoParams(FOculusXRSpatialAnchorCreateDelegate, bool /*Success*/, UOculusXRAnchorComponent* /*Anchor*/);
DECLARE_DELEGATE_TwoParams(FOculusXRAnchorEraseDelegate, bool /*Success*/, FOculusXRUUID /*AnchorUUID*/);
DECLARE_DELEGATE_FourParams(FOculusXRAnchorSetComponentStatusDelegate, bool /*Success*/, UOculusXRAnchorComponent* /*Anchor*/, EOculusXRSpaceComponentType /*ComponentType*/, bool /*Enabled*/);
DECLARE_DELEGATE_TwoParams(FOculusXRAnchorSaveDelegate, bool /*Success*/, UOculusXRAnchorComponent* /*Anchor*/);
DECLARE_DELEGATE_TwoParams(FOculusXRAnchorQueryDelegate, bool /*Success*/, const TArray<FOculusXRSpaceQueryResult>& /*Results*/);

namespace OculusXRAnchors
{

struct OCULUSXRANCHORS_API FOculusXRAnchors
{
	void Initialize();
	void Teardown();

	static FOculusXRAnchors* GetInstance();

	static bool CreateSpatialAnchor(const FTransform& InTransform, AActor* TargetActor, const FOculusXRSpatialAnchorCreateDelegate& ResultCallback);
	static bool EraseAnchor(UOculusXRAnchorComponent* Anchor, const FOculusXRAnchorEraseDelegate& ResultCallback);
	static bool DestroyAnchor(uint64 AnchorHandle);

	static bool SetAnchorComponentStatus(UOculusXRAnchorComponent* Anchor, EOculusXRSpaceComponentType SpaceComponentType, bool Enable, float Timeout, const FOculusXRAnchorSetComponentStatusDelegate& ResultCallback);
	static bool GetAnchorComponentStatus(UOculusXRAnchorComponent* Anchor, EOculusXRSpaceComponentType SpaceComponentType, bool& OutEnabled, bool& OutChangePending);

	static bool SaveAnchor(UOculusXRAnchorComponent* Anchor, EOculusXRSpaceStorageLocation StorageLocation, const FOculusXRAnchorSaveDelegate& ResultCallback);

	static bool QueryAnchors(const TArray<FOculusXRUUID>& AnchorUUIDs, EOculusXRSpaceStorageLocation Location, int32 MaxAnchors, const FOculusXRAnchorQueryDelegate& ResultCallback);
	static bool QueryAnchorsAdvanced(const FOculusXRSpaceQueryInfo& QueryInfo, const FOculusXRAnchorQueryDelegate& ResultCallback);

	static bool GetSpaceScenePlane(uint64 Space, FVector& OutPos, FVector& OutSize);
	static bool GetSpaceSceneVolume(uint64 Space, FVector& OutPos, FVector& OutSize);
	static bool GetSpaceSemanticClassification(uint64 Space, TArray<FString>& OutSemanticClassifications);

private:
	void HandleSpatialAnchorCreateComplete(FOculusXRUInt64 RequestId, int Result, FOculusXRUInt64 Space, FOculusXRUUID UUID);
	void HandleAnchorEraseComplete(FOculusXRUInt64 RequestId, int Result, FOculusXRUUID UUID, EOculusXRSpaceStorageLocation Location);

	void HandleSetComponentStatusComplete(FOculusXRUInt64 RequestId, int Result, FOculusXRUInt64 Space, FOculusXRUUID UUID, EOculusXRSpaceComponentType ComponentType, bool Enabled);

	void HandleAnchorSaveComplete(FOculusXRUInt64 RequestId, FOculusXRUInt64 Space, bool Success, int Result, FOculusXRUUID UUID);

	void HandleAnchorQueryResultsBegin(FOculusXRUInt64 RequestId);
	void HandleAnchorQueryResultElement(FOculusXRUInt64 RequestId, FOculusXRUInt64 Space, FOculusXRUUID UUID);
	void HandleAnchorQueryComplete(FOculusXRUInt64 RequestId, bool Success);

	struct EraseAnchorBinding
	{
		FOculusXRUInt64 RequestId;
		FOculusXRAnchorEraseDelegate Binding;
		TWeakObjectPtr<UOculusXRAnchorComponent> Anchor;
	};

	struct SetComponentStatusBinding
	{
		FOculusXRUInt64 RequestId;
		FOculusXRAnchorSetComponentStatusDelegate Binding;
		TWeakObjectPtr<UOculusXRAnchorComponent> Anchor;
	};

	struct CreateAnchorBinding
	{
		FOculusXRUInt64 RequestId;
		FOculusXRSpatialAnchorCreateDelegate Binding;
		TWeakObjectPtr<AActor> Actor;
	};

	struct SaveAnchorBinding
	{
		FOculusXRUInt64 RequestId;
		FOculusXRAnchorSaveDelegate Binding;
		EOculusXRSpaceStorageLocation Location;
		TWeakObjectPtr<UOculusXRAnchorComponent> Anchor;
	};

	struct AnchorQueryBinding
	{
		FOculusXRUInt64 RequestId;
		FOculusXRAnchorQueryDelegate Binding;
		EOculusXRSpaceStorageLocation Location;
		TArray<FOculusXRSpaceQueryResult> Results;
	};

	// Delegate bindings
	TMap<uint64, CreateAnchorBinding>		CreateSpatialAnchorBindings;
	TMap<uint64, EraseAnchorBinding>		EraseAnchorBindings;
	TMap<uint64, SetComponentStatusBinding>	SetComponentStatusBindings;
	TMap<uint64, SaveAnchorBinding>			AnchorSaveBindings;
	TMap<uint64, AnchorQueryBinding>		AnchorQueryBindings;

	// Delegate handles
	FDelegateHandle DelegateHandleAnchorCreate;
	FDelegateHandle DelegateHandleAnchorErase;
	FDelegateHandle DelegateHandleSetComponentStatus;
	FDelegateHandle DelegateHandleAnchorSave;
	FDelegateHandle DelegateHandleQueryResultsBegin;
	FDelegateHandle DelegateHandleQueryResultElement;
	FDelegateHandle DelegateHandleQueryComplete;
};

}