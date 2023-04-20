// Copyright Epic Games, Inc. All Rights Reserved.

#include "OculusXRToolCommands.h"
#include "Framework/Docking/TabManager.h"

#define LOCTEXT_NAMESPACE "FOculusXREditorModule"

void FOculusToolCommands::RegisterCommands()
{
	UI_COMMAND(OpenPluginWindow, "Oculus Tool", "Show Oculus Tool Window", EUserInterfaceActionType::Button, FInputChord());
	UI_COMMAND(ToggleDeploySo, "Deploy compiled .so directly to device", "Faster deploy when we only have code changes by deploying compiled .so directly to device", EUserInterfaceActionType::ToggleButton, FInputChord());
}

void FOculusToolCommands::ShowOculusTool()
{
	FGlobalTabmanager::Get()->TryInvokeTab(FOculusXREditorModule::OculusPerfTabName);
}

#undef LOCTEXT_NAMESPACE
