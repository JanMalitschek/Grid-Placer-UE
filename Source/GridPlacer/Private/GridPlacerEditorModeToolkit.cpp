// Copyright Epic Games, Inc. All Rights Reserved.

#include "GridPlacerEditorModeToolkit.h"
#include "GridPlacerEditorMode.h"
#include "Engine/Selection.h"

#include "Modules/ModuleManager.h"
#include "PropertyEditorModule.h"
#include "IDetailsView.h"
#include "EditorModeManager.h"

#define LOCTEXT_NAMESPACE "GridPlacerEditorModeToolkit"

FGridPlacerEditorModeToolkit::FGridPlacerEditorModeToolkit()
{
}

void FGridPlacerEditorModeToolkit::Init(const TSharedPtr<IToolkitHost>& InitToolkitHost, TWeakObjectPtr<UEdMode> InOwningMode)
{
	FModeToolkit::Init(InitToolkitHost, InOwningMode);
}

void FGridPlacerEditorModeToolkit::GetToolPaletteNames(TArray<FName>& PaletteNames) const
{
	PaletteNames.Add(NAME_Default);
}


FName FGridPlacerEditorModeToolkit::GetToolkitFName() const
{
	return FName("GridPlacerEditorMode");
}

FText FGridPlacerEditorModeToolkit::GetBaseToolkitName() const
{
	return LOCTEXT("DisplayName", "GridPlacerEditorMode Toolkit");
}

#undef LOCTEXT_NAMESPACE
