/*
Copyright (c) Meta Platforms, Inc. and affiliates.
All rights reserved.

This source code is licensed under the license found in the
LICENSE file in the root directory of this source tree.
*/

#pragma once
#include "OculusXRAnchorTypes.h"
#include "OVR_Plugin_Types.h"

ovrpSpaceComponentType ConvertToOvrpComponentType(const EOculusXRSpaceComponentType ComponentType);

EOculusXRSpaceComponentType ConvertToUe4ComponentType(const ovrpSpaceComponentType ComponentType);
