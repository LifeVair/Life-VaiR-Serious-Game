// Copyright Epic Games, Inc. All Rights Reserved.

#include "OculusXRSceneActor.h"
#include "OculusXRSceneModule.h"
#include "OculusXRHMDModule.h"
#include "OculusXRAnchorManager.h"
#include "OculusXRDelegates.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/AssetManager.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/World.h"
#include "GameFramework/WorldSettings.h"

#define LOCTEXT_NAMESPACE "OculusXRSceneActor"

//////////////////////////////////////////////////////////////////////////
// ASceneActor

AOculusXRSceneActor::AOculusXRSceneActor(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ResetStates();

	// Create required components
	RoomLayoutManagerComponent = CreateDefaultSubobject<UOculusXRRoomLayoutManagerComponent>(TEXT("OculusXRRoomLayoutManagerComponent"));

	// Following are the semantic labels we want to support default properties for.  User can always add new ones through the properties panel if needed.
	const FString defaultSemanticClassification[] = {
		TEXT("WALL_FACE"),
		TEXT("CEILING"),
		TEXT("FLOOR"),
		TEXT("COUCH"),
		TEXT("DESK"),
		TEXT("DOOR_FRAME"),
		TEXT("WINDOW_FRAME"),
		TEXT("OTHER")
	};

	FOculusXRSpawnedSceneAnchorProperties spawnedAnchorProps;

	// Populate default UPROPERTY for the "ScenePlaneSpawnedSceneAnchorProperties" member
	spawnedAnchorProps.StaticMesh = TSoftObjectPtr<UStaticMesh>(FSoftObjectPath("/OculusVR/Meshes/ScenePlane.ScenePlane"));
	for (int32 i = 0; i < sizeof(defaultSemanticClassification) / sizeof(defaultSemanticClassification[0]); ++i)
	{
		FOculusXRSpawnedSceneAnchorProperties& props = ScenePlaneSpawnedSceneAnchorProperties.Add(defaultSemanticClassification[i], spawnedAnchorProps);
		
		// Orientation constraints
		if (defaultSemanticClassification[i] == "CEILING" ||
			defaultSemanticClassification[i] == "FLOOR" ||
			defaultSemanticClassification[i] == "COUCH" ||
			defaultSemanticClassification[i] == "DESK" ||
			defaultSemanticClassification[i] == "DOOR_FRAME" ||
			defaultSemanticClassification[i] == "WINDOW_FRAME" ||
			defaultSemanticClassification[i] == "OTHER")
		{
			props.ForceParallelToFloor = true;
		}
	}

	// Populate default UPROPERTY for the "SceneVolumeSpawnedSceneAnchorProperties" member
	// For the time being, only "OTHER" semantic label is used for volume scene anchors.
	spawnedAnchorProps.StaticMesh = TSoftObjectPtr<UStaticMesh>(FSoftObjectPath("/OculusVR/Meshes/SceneVolume.SceneVolume"));
	FOculusXRSpawnedSceneAnchorProperties& props = SceneVolumeSpawnedSceneAnchorProperties.Add(TEXT("OTHER"), spawnedAnchorProps);
	props.ForceParallelToFloor = true;
}

void AOculusXRSceneActor::ResetStates()
{		
	bCaptureFlowWasLaunched = false;
	ClearScene();
}

void AOculusXRSceneActor::BeginPlay()
{
	Super::BeginPlay();

	// Create a scene component as root so we can attach spawned actors to it
	USceneComponent* rootSceneComponent = NewObject<USceneComponent>(this, USceneComponent::StaticClass());
	rootSceneComponent->SetMobility(EComponentMobility::Static);
	rootSceneComponent->RegisterComponent();
	SetRootComponent(rootSceneComponent);

	// Register delegates
	RoomLayoutManagerComponent->OculusXRRoomLayoutSceneCaptureCompleteNative.AddUObject(this, &AOculusXRSceneActor::SceneCaptureComplete_Handler);

	// Make an initial request to query for the room layout if bPopulateSceneOnBeginPlay was set to true
	if (bPopulateSceneOnBeginPlay)
	{
		PopulateScene();
	}
}

void AOculusXRSceneActor::EndPlay(EEndPlayReason::Type Reason)
{
	// Unregister delegates
	RoomLayoutManagerComponent->OculusXRRoomLayoutSceneCaptureCompleteNative.RemoveAll(this);

	// Calling ResetStates will reset member variables to their default values (including the request IDs).
	ResetStates();

	Super::EndPlay(Reason);
}

void AOculusXRSceneActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

bool AOculusXRSceneActor::QuerySpatialAnchors(const bool bRoomLayoutOnly)
{
	bool bResult = false;

	FOculusXRSpaceQueryInfo queryInfo;
	queryInfo.MaxQuerySpaces = MaxQueries;
	queryInfo.FilterType = EOculusXRSpaceQueryFilterType::FilterByComponentType;

	if (bRoomLayoutOnly)
	{		
		queryInfo.ComponentFilter.Add(EOculusXRSpaceComponentType::RoomLayout);
		bResult = OculusXRAnchors::FOculusXRAnchors::QueryAnchorsAdvanced(queryInfo, FOculusXRAnchorQueryDelegate::CreateUObject(this, &AOculusXRSceneActor::AnchorQueryComplete_Handler));
	}	
	else
	{
		queryInfo.ComponentFilter.Add(EOculusXRSpaceComponentType::ScenePlane);
		bResult = OculusXRAnchors::FOculusXRAnchors::QueryAnchorsAdvanced(queryInfo, FOculusXRAnchorQueryDelegate::CreateUObject(this, &AOculusXRSceneActor::AnchorQueryComplete_Handler));

		queryInfo.ComponentFilter.Empty();
		queryInfo.ComponentFilter.Add(EOculusXRSpaceComponentType::SceneVolume);
		bResult &= OculusXRAnchors::FOculusXRAnchors::QueryAnchorsAdvanced(queryInfo, FOculusXRAnchorQueryDelegate::CreateUObject(this, &AOculusXRSceneActor::AnchorQueryComplete_Handler));
	}

	return bResult;
}

bool AOculusXRSceneActor::IsValidUuid(const FOculusXRUUID& Uuid)
{
	return Uuid.UUIDBytes != nullptr;
}

void AOculusXRSceneActor::LaunchCaptureFlow()
{
	UE_LOG(LogOculusXRScene, Error, TEXT("Launch capture flow"));

	if (RoomLayoutManagerComponent)
	{
		UE_LOG(LogOculusXRScene, Error, TEXT("Launch capture flow -- RoomLayoutManagerComponent"));

		const bool bResult = RoomLayoutManagerComponent->LaunchCaptureFlow();
		if (!bResult)
		{
			UE_LOG(LogOculusXRScene, Error, TEXT("LaunchCaptureFlow() failed!"));
		}
	}
}

void AOculusXRSceneActor::LaunchCaptureFlowIfNeeded()
{
#if WITH_EDITOR
	UE_LOG(LogOculusXRScene, Display, TEXT("Scene Capture does not work over Link. Please capture a scene with the HMD in standalone mode, then access the scene model over Link."));
#else
	// Depending on LauchCaptureFlowWhenMissingScene, we might not want to launch Capture Flow
	if (LauchCaptureFlowWhenMissingScene != EOculusXRLaunchCaptureFlowWhenMissingScene::NEVER)
	{
		if (LauchCaptureFlowWhenMissingScene == EOculusXRLaunchCaptureFlowWhenMissingScene::ALWAYS ||
			(!bCaptureFlowWasLaunched && LauchCaptureFlowWhenMissingScene == EOculusXRLaunchCaptureFlowWhenMissingScene::ONCE))
		{
			if (bVerboseLog)
			{
				UE_LOG(LogOculusXRScene, Display, TEXT("Requesting to launch Capture Flow."));
			}
			
			LaunchCaptureFlow();
		}
	}
#endif
}

bool AOculusXRSceneActor::SpawnSceneAnchor(const FOculusXRUInt64& Space, const FVector& BoundedSize, const TArray<FString>& SemanticClassifications, const EOculusXRSpaceComponentType AnchorComponentType)
{
	if (Space.Value == 0)
	{
		UE_LOG(LogOculusXRScene, Error, TEXT("AOculusXRSceneActor::SpawnSceneAnchor Invalid Space handle."));
		return false;
	}

	if (!(AnchorComponentType == EOculusXRSpaceComponentType::ScenePlane || AnchorComponentType == EOculusXRSpaceComponentType::SceneVolume))
	{
		UE_LOG(LogOculusXRScene, Error, TEXT("AOculusXRSceneActor::SpawnSceneAnchor Anchor doesn't have ScenePlane or SceneVolume component active."));
		return false;
	}

	if (0 == SemanticClassifications.Num())
	{
		UE_LOG(LogOculusXRScene, Error, TEXT("AOculusXRSceneActor::SpawnSceneAnchor No semantic classification found."));
		return false;
	}

	TSoftClassPtr<UOculusXRSceneAnchorComponent>* sceneAnchorComponentClassPtrRef = nullptr;
	TSoftObjectPtr<UStaticMesh>* staticMeshObjPtrRef = nullptr;

	FOculusXRSpawnedSceneAnchorProperties* foundProperties = (AnchorComponentType == EOculusXRSpaceComponentType::ScenePlane) ? ScenePlaneSpawnedSceneAnchorProperties.Find(SemanticClassifications[0]) : SceneVolumeSpawnedSceneAnchorProperties.Find(SemanticClassifications[0]);

	if (!foundProperties)
	{
		UE_LOG(LogOculusXRScene, Warning, TEXT("AOculusXRSceneActor::SpawnSceneAnchor Scene object has an unknown semantic label.  Will not be spawned."));
		return false;
	}

	sceneAnchorComponentClassPtrRef = &foundProperties->ActorComponent;
	staticMeshObjPtrRef = &foundProperties->StaticMesh;

	UClass* sceneAnchorComponentInstanceClass = sceneAnchorComponentClassPtrRef->LoadSynchronous();
	if (!sceneAnchorComponentInstanceClass)
	{
		UE_LOG(LogOculusXRScene, Error, TEXT("AOculusXRSceneActor::SpawnSceneAnchor Scene anchor component class is invalid!  Cannot spawn actor to populate the scene."));
		return false;
	}
	
	FActorSpawnParameters actorSpawnParams;
	actorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AStaticMeshActor* newActor = GetWorld()->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), FVector(), FRotator(), actorSpawnParams);
	newActor->GetRootComponent()->SetMobility(EComponentMobility::Movable);
	newActor->AttachToActor(this, FAttachmentTransformRules::KeepRelativeTransform);
	
	if (staticMeshObjPtrRef->IsPending())
	{
		staticMeshObjPtrRef->LoadSynchronous();
	}
	newActor->GetStaticMeshComponent()->SetStaticMesh(staticMeshObjPtrRef->Get());
	
	UOculusXRSceneAnchorComponent* sceneAnchorComponent = NewObject<UOculusXRSceneAnchorComponent>(newActor, sceneAnchorComponentInstanceClass);
	sceneAnchorComponent->CreationMethod = EComponentCreationMethod::Instance;
	newActor->AddOwnedComponent(sceneAnchorComponent);

	OculusXRAnchors::FOculusXRAnchors::SetAnchorComponentStatus(sceneAnchorComponent, EOculusXRSpaceComponentType::Locatable, true, 0.0f, FOculusXRAnchorSetComponentStatusDelegate());

	sceneAnchorComponent->RegisterComponent();
	sceneAnchorComponent->InitializeComponent();
	sceneAnchorComponent->Activate();
	sceneAnchorComponent->SetHandle(Space);
	sceneAnchorComponent->SemanticClassifications = SemanticClassifications;
	sceneAnchorComponent->ForceParallelToFloor = foundProperties->ForceParallelToFloor;
	sceneAnchorComponent->AddOffset = foundProperties->AddOffset;

	// Setup scale based on bounded size and the actual size of the mesh
	UStaticMesh* staticMesh = newActor->GetStaticMeshComponent()->GetStaticMesh();
	FBoxSphereBounds staticMeshBounds;
	staticMeshBounds.BoxExtent = FVector{1.f, 1.f, 1.f};
	if (staticMesh)
	{
		staticMeshBounds = staticMesh->GetBounds();
	}
	const float worldToMeters = GetWorld()->GetWorldSettings()->WorldToMeters;

	newActor->SetActorScale3D(FVector(
		(BoundedSize.X / (staticMeshBounds.BoxExtent.X * 2.f)) * worldToMeters,
		(BoundedSize.Y / (staticMeshBounds.BoxExtent.Y * 2.f)) * worldToMeters,
		(BoundedSize.Z / (staticMeshBounds.BoxExtent.Z * 2.f)) * worldToMeters)
	);

	return true;
}

bool AOculusXRSceneActor::IsScenePopulated()
{
	if (!RootComponent)
		return false;
	return RootComponent->GetNumChildrenComponents() > 0;
}

bool AOculusXRSceneActor::IsRoomLayoutValid()
{
	return bRoomLayoutIsValid;
}

void AOculusXRSceneActor::PopulateScene()
{
	if (!RootComponent)
		return;

	if (IsScenePopulated())
	{
		UE_LOG(LogOculusXRScene, Display, TEXT("PopulateScene Scene is already populated.  Clear it first."));
		return;
	}

	const bool bResult = QuerySpatialAnchors(true);
	if (bResult)
	{
		if (bVerboseLog)
		{
			UE_LOG(LogOculusXRScene, Display, TEXT("PopulateScene Made a request to query spatial anchors"));
		}
	}
	else
	{
		UE_LOG(LogOculusXRScene, Error, TEXT("PopulateScene Failed to query spatial anchors!"));
	}
}

void AOculusXRSceneActor::ClearScene()
{
	if (!RootComponent)
		return;

	TArray<USceneComponent*> childrenComponents = RootComponent->GetAttachChildren();
	for (USceneComponent* SceneComponent : childrenComponents)
	{
		Cast<AActor>(SceneComponent->GetOuter())->Destroy();
	}

	bRoomLayoutIsValid = false;
	bFoundCapturedScene = false;
}

void AOculusXRSceneActor::SetVisibilityToAllSceneAnchors(const bool bIsVisible)
{
	if (!RootComponent)
		return;

	TArray<USceneComponent*> childrenComponents = RootComponent->GetAttachChildren();
	for (USceneComponent* sceneComponent : childrenComponents)
	{
		sceneComponent->SetVisibility(bIsVisible);
	}
}

void AOculusXRSceneActor::SetVisibilityToSceneAnchorsBySemanticLabel(const FString SemanticLabel, const bool bIsVisible)
{
	if (!RootComponent)
		return;

	TArray<USceneComponent*> childrenComponents = RootComponent->GetAttachChildren();
	for (USceneComponent* sceneComponent : childrenComponents)
	{
		UObject* outerObject = sceneComponent->GetOuter();
		if (!outerObject)
		{
			continue;
		}

		AActor* outerActor = Cast<AActor>(outerObject);
		if (!outerActor)
		{
			continue;
		}

		UActorComponent* sceneAnchorComponent = outerActor->GetComponentByClass(UOculusXRSceneAnchorComponent::StaticClass());
		if (!sceneAnchorComponent)
		{
			continue;
		}
		
		if (Cast<UOculusXRSceneAnchorComponent>(sceneAnchorComponent)->SemanticClassifications.Contains(SemanticLabel))
		{
			sceneComponent->SetVisibility(bIsVisible);
		}
	}
}

TArray<AActor*> AOculusXRSceneActor::GetActorsBySemanticLabel(const FString SemanticLabel)
{
	TArray<AActor*> actors;

	if (!RootComponent)
		return actors;

	TArray<USceneComponent*> childrenComponents = RootComponent->GetAttachChildren();
	for (USceneComponent* sceneComponent : childrenComponents)
	{
		UObject* outerObject = sceneComponent->GetOuter();
		if (!outerObject)
		{
			continue;
		}

		AActor* outerActor = Cast<AActor>(outerObject);
		if (!outerActor)
		{
			continue;
		}

		UActorComponent* sceneAnchorComponent = outerActor->GetComponentByClass(UOculusXRSceneAnchorComponent::StaticClass());
		if (!sceneAnchorComponent)
		{
			continue;
		}

		if (Cast<UOculusXRSceneAnchorComponent>(sceneAnchorComponent)->SemanticClassifications.Contains(SemanticLabel))
		{
			actors.Add(outerActor);
		}
	}

	return actors;
}


// DELEGATE HANDLERS
void AOculusXRSceneActor::AnchorQueryComplete_Handler(bool Success, const TArray<FOculusXRSpaceQueryResult>& Results)
{
	for (auto& Result : Results)
	{
		// Call the existing logic for each result
		SpatialAnchorQueryResult_Handler(0, Result.Space, Result.UUID);
	}

	// Call the complete handler at the end to check if we need to do room layout stuff
	SpatialAnchorQueryComplete_Handler(0, Success);
}

void AOculusXRSceneActor::SpatialAnchorQueryResult_Handler(FOculusXRUInt64 RequestId, FOculusXRUInt64 Space, FOculusXRUUID Uuid)
{
	if (bVerboseLog)
	{
		UE_LOG(LogOculusXRScene, Display, TEXT("SpatialAnchorQueryResult_Handler (requestId = %llu, space = %llu, uuid = %s"), RequestId.Value, Space.Value, *BytesToHex(Uuid.UUIDBytes, OCULUSXR_UUID_SIZE));
	}

	bool bResult = false;
	bool bOutPending = false;
	bool bIsRoomLayout = false;

	bResult = OculusXRAnchors::FOculusXRAnchorManager::GetSpaceComponentStatus(Space.Value, EOculusXRSpaceComponentType::RoomLayout, bIsRoomLayout, bOutPending);

	if (bResult && bIsRoomLayout)
	{
		if (bVerboseLog)
		{
			UE_LOG(LogOculusXRScene, Display, TEXT("SpatialAnchorQueryResult_Handler Found a room layout."));
		}

		// This is a room layout.  We can now populate it with the room layout manager component
		FOculusXRRoomLayout roomLayout;
		const bool bGetRoomLayoutResult = RoomLayoutManagerComponent->GetRoomLayout(Space, roomLayout, MaxQueries);
		if (bGetRoomLayoutResult)
		{
			// If we get here, then we know that captured scene was already created by the end-user in Capture Flow
			bFoundCapturedScene = true;

			// We can now validate the room
			bRoomLayoutIsValid = true;

			bRoomLayoutIsValid &= IsValidUuid(roomLayout.CeilingUuid);
			bRoomLayoutIsValid &= IsValidUuid(roomLayout.FloorUuid);
			bRoomLayoutIsValid &= roomLayout.WallsUuid.Num() > 3;

			for (int32 i = 0; i < roomLayout.WallsUuid.Num(); ++i)
			{
				bRoomLayoutIsValid &= IsValidUuid(roomLayout.WallsUuid[i]);
			}

			if (bRoomLayoutIsValid || !bEnsureRoomIsValid)
			{
				if (bVerboseLog)
				{
					UE_LOG(LogOculusXRScene, Display, TEXT("SpatialAnchorQueryResult_Handler Room is valid = %d (# walls = %d)."), bRoomLayoutIsValid, roomLayout.WallsUuid.Num());
				}

				// We found a valid room, we can now query all ScenePlane/SceneVolume anchors
				bResult = QuerySpatialAnchors(false);
				if (bResult)
				{
					if (bVerboseLog)
					{
						UE_LOG(LogOculusXRScene, Display, TEXT("SpatialAnchorQueryResult_Handler Made a request to query spatial anchors"));
					}
				}
				else
				{
					UE_LOG(LogOculusXRScene, Error, TEXT("SpatialAnchorQueryResult_Handler Failed to query spatial anchors!"));
				}
			}
			else
			{
				UE_LOG(LogOculusXRScene, Display, TEXT("SpatialAnchorQueryResult_Handler Room is invalid."));
				ClearScene();
				LaunchCaptureFlowIfNeeded();
			}
		}
	}
	else
	{
		// Is it a ScenePlane anchor?
		bool bIsScenePlane = false;
		bResult = OculusXRAnchors::FOculusXRAnchorManager::GetSpaceComponentStatus(Space.Value, EOculusXRSpaceComponentType::ScenePlane, bIsScenePlane, bOutPending);
		if (bResult && bIsScenePlane)
		{
			if (bVerboseLog)
			{
				UE_LOG(LogOculusXRScene, Display, TEXT("SpatialAnchorQueryResult_Handler Found a ScenePlane anchor."));
			}

			FVector scenePlanePos;
			FVector scenePlaneSize;
			bResult = OculusXRAnchors::FOculusXRAnchors::GetSpaceScenePlane(Space.Value, scenePlanePos, scenePlaneSize);
			if (bResult)
			{
				if (bVerboseLog)
				{
					UE_LOG(LogOculusXRScene, Display, TEXT("SpatialAnchorQueryResult_Handler ScenePlane pos = [%.2f, %.2f, %.2f], size = [%.2f, %.2f, %.2f]."),
						scenePlanePos.X, scenePlanePos.Y, scenePlanePos.Z,
						scenePlaneSize.X, scenePlaneSize.Y, scenePlaneSize.Z);
				}
			}
			else
			{
				UE_LOG(LogOculusXRScene, Error, TEXT("SpatialAnchorQueryResult_Handler Failed to get bounds for ScenePlane space."));
			}

			TArray<FString> semanticClassifications;
			bResult = OculusXRAnchors::FOculusXRAnchors::GetSpaceSemanticClassification(Space.Value, semanticClassifications);
			if (bResult)
			{
				if (bVerboseLog)
				{
					UE_LOG(LogOculusXRScene, Display, TEXT("SpatialAnchorQueryResult_Handler Semantic Classifications:"));
					for (FString& label : semanticClassifications)
					{
						UE_LOG(LogOculusXRScene, Display, TEXT("%s"), *label);
					}
				}
			}
			else
			{
				UE_LOG(LogOculusXRScene, Error, TEXT("SpatialAnchorQueryResult_Handler Failed to get semantic classification space."));
			}

			if (!SpawnSceneAnchor(Space, scenePlaneSize, semanticClassifications, EOculusXRSpaceComponentType::ScenePlane))
			{
				UE_LOG(LogOculusXRScene, Error, TEXT("SpatialAnchorQueryResult_Handler Failed to spawn scene anchor."));
			}
		}
		else
		{
			// Is it a scenevolume anchor?
			bool bIsSceneVolume = false;
			bResult = OculusXRAnchors::FOculusXRAnchorManager::GetSpaceComponentStatus(Space.Value, EOculusXRSpaceComponentType::SceneVolume, bIsSceneVolume, bOutPending);
			if (bResult && bIsSceneVolume)
			{
				if (bVerboseLog)
				{
					UE_LOG(LogOculusXRScene, Display, TEXT("SpatialAnchorQueryResult_Handler Found a SceneVolume anchor."));
				}

				FVector sceneVolumePos;
				FVector sceneVolumeSize;
				bResult = OculusXRAnchors::FOculusXRAnchors::GetSpaceSceneVolume(Space.Value, sceneVolumePos, sceneVolumeSize);
				if (bResult)
				{
					if (bVerboseLog)
					{
						UE_LOG(LogOculusXRScene, Display, TEXT("SpatialAnchorQueryResult_Handler SceneVolume pos = [%.2f, %.2f, %.2f], size = [%.2f, %.2f, %.2f]."),
							sceneVolumePos.X, sceneVolumePos.Y, sceneVolumePos.Z,
							sceneVolumeSize.X, sceneVolumeSize.Y, sceneVolumeSize.Z);
					}
				}
				else
				{
					UE_LOG(LogOculusXRScene, Error, TEXT("SpatialAnchorQueryResult_Handler Failed to get bounds for SceneVolume space."));
				}

				TArray<FString> semanticClassifications;
				bResult = OculusXRAnchors::FOculusXRAnchors::GetSpaceSemanticClassification(Space.Value, semanticClassifications);
				if (bResult)
				{
					if (bVerboseLog)
					{
						UE_LOG(LogOculusXRScene, Display, TEXT("SpatialAnchorQueryResult_Handler Semantic Classifications:"));
						for (FString& label : semanticClassifications)
						{
							UE_LOG(LogOculusXRScene, Display, TEXT("%s"), *label);
						}
					}
				}
				else
				{
					UE_LOG(LogOculusXRScene, Error, TEXT("SpatialAnchorQueryResult_Handler Failed to get semantic classifications space."));
				}

				if (!SpawnSceneAnchor(Space, sceneVolumeSize, semanticClassifications, EOculusXRSpaceComponentType::SceneVolume))
				{
					UE_LOG(LogOculusXRScene, Error, TEXT("SpatialAnchorQueryResult_Handler Failed to spawn scene anchor."));
				}
			}
		}
	}
}

void AOculusXRSceneActor::SpatialAnchorQueryComplete_Handler(FOculusXRUInt64 RequestId, bool bResult)
{
	if (bVerboseLog)
	{
		UE_LOG(LogOculusXRScene, Display, TEXT("SpatialAnchorQueryComplete_Handler (requestId = %llu)"), RequestId.Value);
	}

	if (!bFoundCapturedScene) // only try to launch capture flow if a captured scene was not found
	{
		if (!bResult)
		{
			if (bVerboseLog)
			{
				UE_LOG(LogOculusXRScene, Display, TEXT("SpatialAnchorQueryComplete_Handler No scene found."));
			}
			LaunchCaptureFlowIfNeeded();
		}
	}
}

void AOculusXRSceneActor::SceneCaptureComplete_Handler(FOculusXRUInt64 RequestId, bool bResult)
{	
	if (bVerboseLog)
	{
		UE_LOG(LogOculusXRScene, Display, TEXT("SceneCaptureComplete_Handler (requestId = %llu)"), RequestId);
	}

	if (!bResult)
	{
		UE_LOG(LogOculusXRScene, Error, TEXT("Scene Capture Complete failed!"));
		return;
	}
	
	// Mark that we already launched Capture Flow and try to query spatial anchors again
	bCaptureFlowWasLaunched = true;

	ClearScene();
	
	const bool bQueryResult = QuerySpatialAnchors(true);
	if (bResult)
	{
		if (bVerboseLog)
		{
			UE_LOG(LogOculusXRScene, Display, TEXT("Made a request to query spatial anchors"));
		}
	}
	else
	{
		UE_LOG(LogOculusXRScene, Error, TEXT("Failed to query spatial anchors!"));
	}
}

#undef LOCTEXT_NAMESPACE
