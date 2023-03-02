/*
Copyright (c) Meta Platforms, Inc. and affiliates.
All rights reserved.

This source code is licensed under the license found in the
LICENSE file in the root directory of this source tree.
*/
#pragma once

#include "CoreMinimal.h"
#include "OculusXRAnchorComponent.h"
#include "OculusXRHMDPrivate.h"

namespace OculusXRAnchors
{
	struct OCULUSXRANCHORS_API FOculusXRAnchorManager
	{
		static bool CreateAnchor(const FTransform& InTransform, uint64& OutRequestId, const FTransform& CameraTransform);
		static bool DestroySpace(uint64 Space);
		static bool SetSpaceComponentStatus(uint64 Space, EOculusXRSpaceComponentType SpaceComponentType, bool Enable,float Timeout, uint64& OutRequestId);
		static bool GetSpaceComponentStatus(uint64 Space, EOculusXRSpaceComponentType SpaceComponentType, bool &OutEnabled, bool &OutChangePending);
		static bool SaveAnchor(uint64 Space, EOculusXRSpaceStorageLocation StorageLocation, EOculusXRSpaceStoragePersistenceMode StoragePersistenceMode, uint64& OutRequestId);
		static bool EraseAnchor(uint64 AnchorHandle, EOculusXRSpaceStorageLocation StorageLocation, uint64& OutRequestId);
		static bool QuerySpaces(const FOculusXRSpaceQueryInfo& QueryInfo, uint64& OutRequestId);
		static bool GetSpaceScenePlane(uint64 Space, FVector& OutPos, FVector& OutSize);
		static bool GetSpaceSceneVolume(uint64 Space, FVector& OutPos, FVector& OutSize);
		static bool GetSpaceSemanticClassification(uint64 Space, TArray<FString>& OutSemanticClassification);

		static void OnPollEvent(ovrpEventDataBuffer* EventDataBuffer, bool& EventPollResult);
	};
}


