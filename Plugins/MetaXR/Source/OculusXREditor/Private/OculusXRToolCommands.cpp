// Copyright Epic Games, Inc. All Rights Reserved.

#include "OculusXRToolCommands.h"
#include "Framework/Docking/TabManager.h"

#define LOCTEXT_NAMESPACE "FOculusXREditorModule"

void FOculusToolCommands::RegisterCommands()
{
	UI_COMMAND(OpenPluginWindow, "Meta XR Peformance Window", "Show Meta XR Peformance Window", EUserInterfaceActionType::Button, FInputChord());
	UI_COMMAND(ToggleDeploySo, "Deploy compiled .so directly to device", "Faster deploy when we only have code changes by deploying compiled .so directly to device", EUserInterfaceActionType::ToggleButton, FInputChord());
	UI_COMMAND(OpenPlatWindow, "Meta XR Platform Window", "Show Meta XR Platform Window", EUserInterfaceActionType::Button, FInputChord());

	UI_COMMAND(ToggleMetaXRSim, "Meta XR Simulator (Experimental)", "Activate/Deactivate Meta XR Simulator (Experimental)", EUserInterfaceActionType::ToggleButton, FInputChord());

	UI_COMMAND(LaunchOffice, "Launch Office", "Launch Office", EUserInterfaceActionType::Button, FInputChord());
	UI_COMMAND(LaunchLivingRoom, "Launch Living Room", "Launch Living Room", EUserInterfaceActionType::Button, FInputChord());
	UI_COMMAND(LaunchBedroom, "Launch Bedroom", "Launch Bedroom", EUserInterfaceActionType::Button, FInputChord());
	UI_COMMAND(StopServer, "Stop Server", "Stop Server", EUserInterfaceActionType::Button, FInputChord());
}

void FOculusToolCommands::ShowOculusTool()
{
	FGlobalTabmanager::Get()->TryInvokeTab(FOculusXREditorModule::OculusPerfTabName);
}

#undef LOCTEXT_NAMESPACE
