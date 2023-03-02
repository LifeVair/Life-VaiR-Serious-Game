/*
Copyright (c) Meta Platforms, Inc. and affiliates.
All rights reserved.

This source code is licensed under the license found in the
LICENSE file in the root directory of this source tree.
*/

#pragma once

#include "Kismet/BlueprintAsyncActionBase.h"
#include "OculusXRAnchorTypes.h"
#include "OculusXRAnchorComponent.h"
#include "OculusXRAnchorLatentActions.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOculusXR_LatentAction_CreateSpatialAnchor_Success, UOculusXRAnchorComponent*, Anchor);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOculusXR_LatentAction_CreateSpatialAnchor_Failure);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOculusXR_LatentAction_EraseAnchor_Success, AActor*, Actor, FOculusXRUUID, UUID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOculusXR_LatentAction_EraseAnchor_Failure);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOculusXR_LatentAction_SaveAnchor_Success, UOculusXRAnchorComponent*, Anchor);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOculusXR_LatentAction_SaveAnchor_Failure);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOculusXR_LatentAction_QueryAnchors_Success, const TArray<FOculusXRSpaceQueryResult>&, QueryResults);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOculusXR_LatentAction_QueryAnchors_Failure);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOculusXR_LatentAction_SetComponentStatus_Success, UOculusXRAnchorComponent*, Anchor, EOculusXRSpaceComponentType, ComponentType, bool, Enabled);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOculusXR_LatentAction_SetComponentStatus_Failure);

//
// Create Anchor
//
UCLASS()
class OCULUSXRANCHORS_API UOculusXRAsyncAction_CreateSpatialAnchor : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()
public:
	virtual void Activate() override;

	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true"))
	static UOculusXRAsyncAction_CreateSpatialAnchor* OculusXRAsyncCreateSpatialAnchor(AActor* TargetActor, const FTransform& AnchorTransform);

	UPROPERTY(BlueprintAssignable)
	FOculusXR_LatentAction_CreateSpatialAnchor_Success Success;

	UPROPERTY(BlueprintAssignable)
	FOculusXR_LatentAction_CreateSpatialAnchor_Failure Failure;

	// Target actor
	UPROPERTY(Transient)
	AActor* TargetActor;

	FTransform AnchorTransform;

private:
	void HandleCreateComplete(bool CreateSuccess, UOculusXRAnchorComponent* Anchor);
};


//
// Erase Anchor
//
UCLASS()
class OCULUSXRANCHORS_API UOculusXRAsyncAction_EraseAnchor : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()
public:
	virtual void Activate() override;

	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true"))
	static UOculusXRAsyncAction_EraseAnchor* OculusXRAsyncEraseAnchor(AActor* TargetActor);

	UPROPERTY(BlueprintAssignable)
	FOculusXR_LatentAction_EraseAnchor_Success Success;

	UPROPERTY(BlueprintAssignable)
	FOculusXR_LatentAction_EraseAnchor_Failure Failure;

	// Target actor
	UPROPERTY(Transient)
	AActor* TargetActor;

	FOculusXRUInt64 DeleteRequestId;

private:
	void HandleEraseAnchorComplete(bool EraseSuccess, FOculusXRUUID UUID);
};


//
// Save Anchor
//
UCLASS()
class OCULUSXRANCHORS_API UOculusXRAsyncAction_SaveAnchor : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()
public:
	virtual void Activate() override;

	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true"))
	static UOculusXRAsyncAction_SaveAnchor* OculusXRAsyncSaveAnchor(AActor* TargetActor, EOculusXRSpaceStorageLocation StorageLocation);

	UPROPERTY(BlueprintAssignable)
	FOculusXR_LatentAction_SaveAnchor_Success Success;

	UPROPERTY(BlueprintAssignable)
	FOculusXR_LatentAction_SaveAnchor_Failure Failure;

	// Target actor
	UPROPERTY(Transient)
	AActor* TargetActor;

	EOculusXRSpaceStorageLocation StorageLocation;

private:
	void HandleSaveAnchorComplete(bool SaveSuccess, UOculusXRAnchorComponent* Anchor);
};


//
// Query Anchors
//
UCLASS()
class OCULUSXRANCHORS_API UOculusXRAsyncAction_QueryAnchors : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()
public:
	virtual void Activate() override;

	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true"))
	static UOculusXRAsyncAction_QueryAnchors* OculusXRAsyncQueryAnchors(EOculusXRSpaceStorageLocation Location, const TArray<FOculusXRUUID>& UUIDs, int32 MaxAnchors);

	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true"))
	static UOculusXRAsyncAction_QueryAnchors* OculusXRAsyncQueryAnchorsAdvanced(const FOculusXRSpaceQueryInfo& QueryInfo);

	UPROPERTY(BlueprintAssignable)
	FOculusXR_LatentAction_QueryAnchors_Success Success;

	UPROPERTY(BlueprintAssignable)
	FOculusXR_LatentAction_QueryAnchors_Failure Failure;

	FOculusXRSpaceQueryInfo QueryInfo;
	TArray<FOculusXRSpaceQueryResult> QueryResults;

private:
	void HandleQueryAnchorsResults(bool QuerySuccess, const TArray<FOculusXRSpaceQueryResult>& Results);
};


//
// Set Component Status
//
UCLASS()
class OCULUSXRANCHORS_API UOculusXRAsyncAction_SetAnchorComponentStatus : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()
public:
	virtual void Activate() override;

	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true"))
	static UOculusXRAsyncAction_SetAnchorComponentStatus* OculusXRAsyncSetAnchorComponentStatus(AActor* TargetActor, EOculusXRSpaceComponentType ComponentType, bool bEnabled);

	UPROPERTY(BlueprintAssignable)
	FOculusXR_LatentAction_SetComponentStatus_Success Success;

	UPROPERTY(BlueprintAssignable)
	FOculusXR_LatentAction_SetComponentStatus_Failure Failure;

	// Target actor
	UPROPERTY(Transient)
	AActor* TargetActor;

	EOculusXRSpaceComponentType ComponentType;
	bool bEnabled;

private:
	void HandleSetComponentStatusComplete(bool SetStatusSuccess, UOculusXRAnchorComponent* Anchor, EOculusXRSpaceComponentType SpaceComponentType, bool bResultEnabled);
};