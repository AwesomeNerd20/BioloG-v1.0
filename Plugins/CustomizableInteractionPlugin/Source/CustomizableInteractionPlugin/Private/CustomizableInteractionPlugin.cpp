// Copyright Epic Games, Inc. All Rights Reserved.

#include "CustomizableInteractionPlugin.h"

#define LOCTEXT_NAMESPACE "FCustomizableInteractionPluginModule"

void FCustomizableInteractionPluginModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
}

void FCustomizableInteractionPluginModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FCustomizableInteractionPluginModule, CustomizableInteractionPlugin)