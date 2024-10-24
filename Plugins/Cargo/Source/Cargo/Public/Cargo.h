// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CargoListener.h"
#include "Modules/ModuleManager.h"
#include "Styling/SlateStyle.h"

class FCargoModule : public IModuleInterface
{
public:
	/** IModuleInterface implementation */
	virtual auto StartupModule() -> void override;
	virtual auto ShutdownModule() -> void override;

	/** This function will be bound to Command. */
	auto PluginButtonActivate() -> void;

private:
	auto RegisterMenus() -> void;
	auto CreateToolbarButton() -> void;
	auto AddCargoMenu(FToolBarBuilder& ToolbarBuilder) -> void;

private:
	TSharedPtr<class FUICommandList> PluginCommands;
	TSharedPtr<FSlateStyleSet> StyleActive;
	TSharedPtr<FSlateStyleSet> StyleInactive;
	FCargoListener* Listener = nullptr;
	FString CargoConnectionStatus;
};
