// Copyright Epic Games, Inc. All Rights Reserved.

#include "GridPlacerEditorMode.h"
#include "GridPlacerEditorModeToolkit.h"
#include "EdModeInteractiveToolsContext.h"
#include "InteractiveToolManager.h"
#include "GridPlacerEditorModeCommands.h"


//////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////// 
// AddYourTool Step 1 - include the header file for your Tools here
//////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////// 
#include "Tools/GridPlacerSimpleTool.h"
#include "Tools/GridPlacerInteractiveTool.h"
#include "Tools/PlacementTool.h"
#include "WorldPartition/ContentBundle/ContentBundleLog.h"

// step 2: register a ToolBuilder in FGridPlacerEditorMode::Enter() below


#define LOCTEXT_NAMESPACE "GridPlacerEditorMode"

const FEditorModeID UGridPlacerEditorMode::EM_GridPlacerEditorModeId = TEXT("EM_GridPlacerEditorMode");

FString UGridPlacerEditorMode::SimpleToolName = TEXT("GridPlacer_ActorInfoTool");
FString UGridPlacerEditorMode::InteractiveToolName = TEXT("GridPlacer_MeasureDistanceTool");
FString UGridPlacerEditorMode::PlacementToolName = TEXT("GridPlacer_PlacementTool");


UGridPlacerEditorMode::UGridPlacerEditorMode()
{
	FModuleManager::Get().LoadModule("EditorStyle");

	// appearance and icon in the editing mode ribbon can be customized here
	// FString IconPath = FPaths::ProjectPluginsDir() + TEXT("GridPlacer/Content/Icons/GridPlacer_40x.png");
	// FName IconBrushName = FName(*IconPath);
	// new FSlateImageBrush(IconBrushName, FVector2D(40, 40));
	Info = FEditorModeInfo(UGridPlacerEditorMode::EM_GridPlacerEditorModeId,
		LOCTEXT("ModeName", "GridPlacer"),
		FSlateIcon(),
		true);
}


UGridPlacerEditorMode::~UGridPlacerEditorMode()
{
}


void UGridPlacerEditorMode::ActorSelectionChangeNotify()
{
}

void UGridPlacerEditorMode::Enter()
{
	UEdMode::Enter();

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	// AddYourTool Step 2 - register the ToolBuilders for your Tools here.
	// The string name you pass to the ToolManager is used to select/activate your ToolBuilder later.
	//////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////// 
	const FGridPlacerEditorModeCommands& SampleToolCommands = FGridPlacerEditorModeCommands::Get();

	/*RegisterTool(SampleToolCommands.SimpleTool, SimpleToolName, NewObject<UGridPlacerSimpleToolBuilder>(this));
	RegisterTool(SampleToolCommands.InteractiveTool, InteractiveToolName, NewObject<UGridPlacerInteractiveToolBuilder>(this));*/
	RegisterTool(SampleToolCommands.PlacementTool, PlacementToolName, NewObject<UPlacementToolBuilder>(this));

	// active tool type is not relevant here, we just set to default
	GetToolManager()->SelectActiveToolType(EToolSide::Left, SimpleToolName);
}

void UGridPlacerEditorMode::CreateToolkit()
{
	Toolkit = MakeShareable(new FGridPlacerEditorModeToolkit);
}

TMap<FName, TArray<TSharedPtr<FUICommandInfo>>> UGridPlacerEditorMode::GetModeCommands() const
{
	return FGridPlacerEditorModeCommands::Get().GetCommands();
}

#undef LOCTEXT_NAMESPACE
