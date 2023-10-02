// Copyright Epic Games, Inc. All Rights Reserved.

#include "GridPlacerModule.h"
#include "GridPlacerEditorModeCommands.h"

#define LOCTEXT_NAMESPACE "GridPlacerModule"

void FGridPlacerModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module

	FGridPlacerEditorModeCommands::Register();

	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	PropertyModule.RegisterCustomPropertyTypeLayout(
		FName("ObjectPalette"),
		FOnGetPropertyTypeCustomizationInstance::CreateStatic(&ObjectPaletteCustomization::MakeInstance));
}

void FGridPlacerModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	PropertyModule.UnregisterCustomPropertyTypeLayout(FName("ObjectPalette"));

	FGridPlacerEditorModeCommands::Unregister();
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FGridPlacerModule, GridPlacerEditorMode)