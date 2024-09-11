// Copyright 2021 Sam Carey. All Rights Reserved.

#include "AchievementProgressList.h"
#include "UObject/ConstructorHelpers.h"
#include "UIAchievement.h"
#include "Components/ScrollBoxSlot.h"
#include "Kismet/KismetStringLibrary.h"
#include "Kismet/GameplayStatics.h"

UAchievementProgressList::UAchievementProgressList(const FObjectInitializer& ObjectInitializer) : UUserWidget(ObjectInitializer)
{
	static ConstructorHelpers::FClassFinder<UUIAchievement> UIAchievementClassHelper(TEXT("WidgetBlueprintGeneratedClass'/AchievementSystem/BP_UIAchievement_Rounded.BP_UIAchievement_Rounded_C'"));
	UIAchievementClass = UIAchievementClassHelper.Class;
}

void UAchievementProgressList::UpdateList()
{
	TArray<FAchievementStates> States = SubSystem->SaveGame->States;
	TArray<FAchievementData> Data = Settings->Achievements;
	int32 InList = 0;
	
	ListView->ClearChildren();
	for(int i = 0; i < Data.Num(); i++)
	{
		if((Filter == EAchievementProgressFilter::ACHIEVED && !States[i].Achieved) || Filter == EAchievementProgressFilter::UNACHIEVED && States[i].Achieved)
			continue;

		if(FilterByTag)
		{
			if(!Data[i].Tags.Contains(FilterTag))
				continue;
		}
		
		if(!Data[i].Spoiler || States[i].Achieved)
		{
			TSubclassOf<UUIAchievement> SelectedUIAchievementClass = UiAchievementOverride ? UiAchievementOverride : UIAchievementClass;
			UUIAchievement* Temp = CreateWidget<UUIAchievement>(UGameplayStatics::GetPlayerController(GetWorld(), 0), SelectedUIAchievementClass);
			Temp->SetValue(Settings->Achievements[i], SubSystem->SaveGame->States[i], true, Settings);
			UScrollBoxSlot* ScrollBoxSlot = Cast<UScrollBoxSlot>(ListView->AddChild(Temp));
			ScrollBoxSlot->SetPadding(FMargin(0,0, 0, Spacing));
		}
		else
		{
			InList++;
		}
	}

	bool showText = bShowHiddenAchievementCount && InList >= 1;

	HiddenTextText->SetVisibility(showText? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	HiddenTextNumber->SetVisibility(showText ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	
	if(showText)
	{
		HiddenTextText->SetText(InList > 1 ? HiddenAchievementsWordMultiple : HiddenAchievementsWordSingular);
		HiddenTextNumber->SetText(FText::AsNumber(InList));
	}
}

void UAchievementProgressList::NativePreConstruct()
{
	HiddenTextNumber->SetVisibility(bShowHiddenAchievementCount ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	HiddenTextText->SetVisibility(bShowHiddenAchievementCount ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
}

void UAchievementProgressList::NativeConstruct()
{
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(GetWorld());
	SubSystem = GameInstance->GetSubsystem<UAchievementSubSystem>();
	Settings = UAchievementSystemSettings::GetSettings();
	UpdateList();
}