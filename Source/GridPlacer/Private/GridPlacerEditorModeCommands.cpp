// Copyright Epic Games, Inc. All Rights Reserved.

#include "GridPlacerEditorModeCommands.h"
#include "GridPlacerEditorMode.h"
#include "EditorStyleSet.h"

#define LOCTEXT_NAMESPACE "GridPlacerEditorModeCommands"

FGridPlacerEditorModeCommands::FGridPlacerEditorModeCommands()
	: TCommands<FGridPlacerEditorModeCommands>("GridPlacerEditorMode",
		NSLOCTEXT("GridPlacerEditorMode", "GridPlacerEditorModeCommands", "GridPlacer Editor Mode"),
		NAME_None,
		FEditorStyle::GetStyleSetName())
{
}

void FGridPlacerEditorModeCommands::RegisterCommands()
{
	TArray <TSharedPtr<FUICommandInfo>>& ToolCommands = Commands.FindOrAdd(NAME_Default);

	/*UI_COMMAND(SimpleTool, "Show Actor Info", "Opens message box with info about a clicked actor", EUserInterfaceActionType::Button, FInputChord());
	ToolCommands.Add(SimpleTool);

	UI_COMMAND(InteractiveTool, "Measure Distance", "Measures distance between 2 points (click to set origin, shift-click to set end point)", EUserInterfaceActionType::ToggleButton, FInputChord());
	ToolCommands.Add(InteractiveTool);*/

	UI_COMMAND(PlacementTool, "Placement", "Allows placement of Actors in the World", EUserInterfaceActionType::Button, FInputChord());
	ToolCommands.Add(PlacementTool);
}

TMap<FName, TArray<TSharedPtr<FUICommandInfo>>> FGridPlacerEditorModeCommands::GetCommands()
{
	return FGridPlacerEditorModeCommands::Get().Commands;
}

#undef LOCTEXT_NAMESPACE
