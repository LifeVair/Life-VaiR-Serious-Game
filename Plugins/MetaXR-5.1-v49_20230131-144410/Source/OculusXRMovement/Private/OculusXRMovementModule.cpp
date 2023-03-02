/*
Copyright (c) Meta Platforms, Inc. and affiliates.
All rights reserved.
This source code is licensed under the license found in the
LICENSE file in the root directory of this source tree.
*/

#include "OculusXRMovementModule.h"
#include "OculusXRHMDModule.h"
#include "OculusXRMovementLog.h"

#define LOCTEXT_NAMESPACE "OculusXRMovement"

DEFINE_LOG_CATEGORY(LogOculusXRMovement);

//-------------------------------------------------------------------------------------------------
// FOculusXRMovementModule
//-------------------------------------------------------------------------------------------------

FOculusXRMovementModule::FOculusXRMovementModule()
{
}

void FOculusXRMovementModule::StartupModule()
{
}

void FOculusXRMovementModule::ShutdownModule()
{
}

IMPLEMENT_MODULE(FOculusXRMovementModule, OculusXRMovement)

#undef LOCTEXT_NAMESPACE
