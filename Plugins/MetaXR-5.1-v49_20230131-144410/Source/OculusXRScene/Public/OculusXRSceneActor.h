// Copyright Epic Games, Inc. All Rights Reserved.
#pragma once

#include "GameFramework/Actor.h"
#include "OculusXRRoomLayoutManagerComponent.h"
#include "OculusXRAnchorComponent.h"
#include "OculusXRSceneAnchorComponent.h"
#include "OculusXRSceneActor.generated.h"


/** EOculusXRLaunchCaptureFlowWhenMissingScene
* Used to dictate whether the actor should launch the Capture Flow application when a scene is not detected on the device.
* The Actor will check if a scene capture is either non-existent or invalid (ie. missing walls/ceiling/floor) before checking if Capture Flow 
* should be launched.
* 
* NEVER:	will never launch Flow Capture.
* ONCE:		will only launch it once.  If the actor still doesn't detect that a scene was captured, it will not launch Capture Flow again.
* ALWAYS:	will always re-launch Flow Capture if a scene was not detected on the device.
*/
UENUM(BlueprintType)
enum EOculusXRLaunchCaptureFlowWhenMissingScene
{
	NEVER	UMETA(DisplayName = "Never"),
	ONCE	UMETA(DisplayName = "Once"),
	ALWAYS	UMETA(DisplayName = "Always")
};

/** FOculusXRSpawnedSceneAnchorProperties
* Properties/Components that a spawned scene anchor will use.
*/
USTRUCT(BlueprintType)
struct FOculusXRSpawnedSceneAnchorProperties
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Spawned Scene Anchor Properties")
	TSoftClassPtr<UOculusXRSceneAnchorComponent> ActorComponent = TSoftClassPtr<UOculusXRSceneAnchorComponent>(FSoftClassPath(UOculusXRSceneAnchorComponent::StaticClass()));

	UPROPERTY(EditAnywhere, Category = "Spawned Scene Anchor Properties")
	TSoftObjectPtr<UStaticMesh> StaticMesh;

	UPROPERTY(EditAnywhere, Category = "Spawned Scene Anchor Properties")
	bool ForceParallelToFloor = false;

	UPROPERTY(EditAnywhere, Category = "Spawned Scene Anchor Properties")
	FVector AddOffset = FVector::ZeroVector;
};


/**
* AOculusXRSceneActor
* The purpose of this actor is to be able to spawn "scene anchor" actors.
* 
* Each actor type (based on their semantic label) can be configured to be spawned with a specific mesh and actor component.
* 
* Overall, it provides a simple interface to be able to quickly get a captured scene from Capture Flow populated at runtime.
* It also provides a basic and flexible template to making use of the OculusAnchorSDK and UOculusXRRoomLayoutManagerComponent 
* to drive the actor's logic.  This removes the need for the developer to implement a system from scratch that makes use of 
* the native methods and components.
*
* TLDR: 
* - This actor populates a captured scene (created in Capture Flow) by spawning child actors with predefined actor and mesh components.
* - Can be used as is, or can be derived or modified as needed depending on the application's needs.
*/
UCLASS(ClassGroup = OculusXRScene)
class OCULUSXRSCENE_API AOculusXRSceneActor : public AActor
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "OculusXR|Scene Actor")
	void LaunchCaptureFlow();

	UFUNCTION(BlueprintCallable, Category = "OculusXR|Scene Actor")
	bool IsScenePopulated();

	UFUNCTION(BlueprintCallable, Category = "OculusXR|Scene Actor")
	bool IsRoomLayoutValid();

	UFUNCTION(BlueprintCallable, Category = "OculusXR|Scene Actor")
	void PopulateScene();

	UFUNCTION(BlueprintCallable, Category = "OculusXR|Scene Actor")
	void ClearScene();

	UFUNCTION(BlueprintCallable, Category = "OculusXR|Scene Actor")
	void SetVisibilityToAllSceneAnchors(const bool bIsVisible);

	UFUNCTION(BlueprintCallable, Category = "OculusXR|Scene Actor")
	void SetVisibilityToSceneAnchorsBySemanticLabel(const FString SemanticLabel, const bool bIsVisible);

	UFUNCTION(BlueprintCallable, Category = "OculusXR|Scene Actor")
	TArray<AActor*> GetActorsBySemanticLabel(const FString SemanticLabel);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OculusXR|Scene Actor")
	bool bEnsureRoomIsValid = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OculusXR|Scene Actor")
	TEnumAsByte<EOculusXRLaunchCaptureFlowWhenMissingScene> LauchCaptureFlowWhenMissingScene = EOculusXRLaunchCaptureFlowWhenMissingScene::ALWAYS;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OculusXR|Scene Actor", meta = (UIMin = 1, ClampMin = 1, UIMax = 1024, ClampMax = 1024))
	int32 MaxQueries = 64;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OculusXR|Scene Actor")
	bool bVerboseLog = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OculusXR|Scene Actor")
	bool bPopulateSceneOnBeginPlay = true;

	UPROPERTY(EditAnywhere, Category = "OculusXR|Scene Actor")
	TMap<FString, FOculusXRSpawnedSceneAnchorProperties> ScenePlaneSpawnedSceneAnchorProperties;

	UPROPERTY(EditAnywhere, Category = "OculusXR|Scene Actor")
	TMap<FString, FOculusXRSpawnedSceneAnchorProperties> SceneVolumeSpawnedSceneAnchorProperties;

public:
	AOculusXRSceneActor(const FObjectInitializer& ObjectInitializer);

	virtual void BeginPlay() override;
	virtual void EndPlay(EEndPlayReason::Type Reason) override;
	virtual void Tick(float DeltaTime) override;


private:
	// Event delegate handlers
	void AnchorQueryComplete_Handler(bool Success, const TArray<FOculusXRSpaceQueryResult>& Results);

	void SpatialAnchorQueryResult_Handler(FOculusXRUInt64 RequestId, FOculusXRUInt64 Space, FOculusXRUUID Uuid);
	void SpatialAnchorQueryComplete_Handler(FOculusXRUInt64 RequestId, bool bResult);
	void SceneCaptureComplete_Handler(FOculusXRUInt64 RequestId, bool bResult);

	// Launches Capture Flow if (based on LauchCaptureFlowWhenMissingScene member value)
	void LaunchCaptureFlowIfNeeded();

	// Resets states of the Actor
	void ResetStates();

	// Handles logic for making a spatial anchors request
	bool QuerySpatialAnchors(const bool bRoomLayoutOnly);

	// Validates UUID
	bool IsValidUuid(const FOculusXRUUID& Uuid);

	// Spawns a scene anchor
	bool SpawnSceneAnchor(const FOculusXRUInt64& Space, const FVector& BoundedSize, const TArray<FString>& SemanticClassifications, const EOculusXRSpaceComponentType AnchorComponentType);
	
	// Components for room layout and spatial anchors functionalities
	UOculusXRRoomLayoutManagerComponent* RoomLayoutManagerComponent = nullptr;

	// Whether Capture Flow was already launched once
	bool bCaptureFlowWasLaunched;

	// Whether last room layout was valid
	bool bRoomLayoutIsValid;

	// Whether we found a captured scene
	bool bFoundCapturedScene;
};
