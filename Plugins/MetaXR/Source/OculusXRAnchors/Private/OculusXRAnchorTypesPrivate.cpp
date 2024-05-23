/*
Copyright (c) Meta Platforms, Inc. and affiliates.
All rights reserved.

This source code is licensed under the license found in the
LICENSE file in the root directory of this source tree.
*/

#include "OculusXRAnchorTypesPrivate.h"

ovrpSpaceComponentType ConvertToOvrpComponentType(const EOculusXRSpaceComponentType ComponentType)
{
	ovrpSpaceComponentType ovrpType = ovrpSpaceComponentType_Max;
	switch (ComponentType)
	{
		case EOculusXRSpaceComponentType::Locatable:
			ovrpType = ovrpSpaceComponentType_Locatable;
			break;
		case EOculusXRSpaceComponentType::Sharable:
			ovrpType = ovrpSpaceComponentType_Sharable;
			break;
		case EOculusXRSpaceComponentType::Storable:
			ovrpType = ovrpSpaceComponentType_Storable;
			break;
		case EOculusXRSpaceComponentType::ScenePlane:
			ovrpType = ovrpSpaceComponentType_Bounded2D;
			break;
		case EOculusXRSpaceComponentType::SceneVolume:
			ovrpType = ovrpSpaceComponentType_Bounded3D;
			break;
		case EOculusXRSpaceComponentType::SemanticClassification:
			ovrpType = ovrpSpaceComponentType_SemanticLabels;
			break;
		case EOculusXRSpaceComponentType::RoomLayout:
			ovrpType = ovrpSpaceComponentType_RoomLayout;
			break;
		case EOculusXRSpaceComponentType::SpaceContainer:
			ovrpType = ovrpSpaceComponentType_SpaceContainer;
			break;
		case EOculusXRSpaceComponentType::TriangleMesh:
			ovrpType = ovrpSpaceComponentType_TriangleMesh;
			break;
		default:;
	}

	return ovrpType;
}

EOculusXRSpaceComponentType ConvertToUe4ComponentType(const ovrpSpaceComponentType ComponentType)
{
	EOculusXRSpaceComponentType ue4ComponentType = EOculusXRSpaceComponentType::Undefined;
	switch (ComponentType)
	{
		case ovrpSpaceComponentType_Locatable:
			ue4ComponentType = EOculusXRSpaceComponentType::Locatable;
			break;
		case ovrpSpaceComponentType_Sharable:
			ue4ComponentType = EOculusXRSpaceComponentType::Sharable;
			break;
		case ovrpSpaceComponentType_Storable:
			ue4ComponentType = EOculusXRSpaceComponentType::Storable;
			break;
		case ovrpSpaceComponentType_Bounded2D:
			ue4ComponentType = EOculusXRSpaceComponentType::ScenePlane;
			break;
		case ovrpSpaceComponentType_Bounded3D:
			ue4ComponentType = EOculusXRSpaceComponentType::SceneVolume;
			break;
		case ovrpSpaceComponentType_SemanticLabels:
			ue4ComponentType = EOculusXRSpaceComponentType::SemanticClassification;
			break;
		case ovrpSpaceComponentType_RoomLayout:
			ue4ComponentType = EOculusXRSpaceComponentType::RoomLayout;
			break;
		case ovrpSpaceComponentType_SpaceContainer:
			ue4ComponentType = EOculusXRSpaceComponentType::SpaceContainer;
			break;
		case ovrpSpaceComponentType_TriangleMesh:
			ue4ComponentType = EOculusXRSpaceComponentType::TriangleMesh;
			break;
		default:;
	}

	return ue4ComponentType;
}
