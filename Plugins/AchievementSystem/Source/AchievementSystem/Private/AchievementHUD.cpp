// Copyright 2021 Sam Carey. All Rights Reserved.

#include "AchievementHUD.h"
#include "UIAchievement.h"
#include "Components/CanvasPanelSlot.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"
#include "Kismet/KismetMathLibrary.h"

UAchievementHUD::UAchievementHUD(const FObjectInitializer &ObjectInitializer) : UUserWidget(ObjectInitializer)
{
	static ConstructorHelpers::FClassFinder<UUIAchievement> UIAchievementClassHelper(TEXT("WidgetBlueprintGeneratedClass'/AchievementSystem/BP_UIAchievement_Rounded.BP_UIAchievement_Rounded_C'"));
	UIAchievementClass = UIAchievementClassHelper.Class;
}

void UAchievementHUD::ScheduleAchievementDisplay(FAchievementData Achievement, FAchievementStates State)
{
	if(Active.Num() < NotificationHudSettings.TotalOnScreen && !MovingUp)
	{
		Spawn(Achievement, State);
	}
	else
	{
		FAchievementPackage TempPackage = FAchievementPackage();
		TempPackage.Data = Achievement;
		TempPackage.State = State;
		BackLog.Add(TempPackage);
	}
}

void UAchievementHUD::UpdateHUD(float InDeltaTime)
{
	if (Active.Num() < NotificationHudSettings.TotalOnScreen && !MovingUp)
	{
		if (BackLog.Num() > 0)
		{
			Spawn(BackLog[0].Data, BackLog[0].State);
			BackLog.RemoveAt(0);
		}
	}
	else
	{
		if (MovingUp)
		{
			CurrentTime += InDeltaTime;
			for (int32 i = 0; i < Active.Num(); i++)
			{
				UUIAchievement* current = Active[i];
				UCanvasPanelSlot* CSlot = Cast<UCanvasPanelSlot>(current->Slot);

				const float AdjustedSpacing = (i == Active.Num() - 1) ? 0 : NotificationHudSettings.Spacing;
        
				CSlot->SetPosition(FVector2D(
					CSlot->GetPosition().X,
					UKismetMathLibrary::MapRangeClamped(CurrentTime, 0, NotificationHudSettings.EntranceAnimationLength, current->StartHeight, current->StartHeight + (IsBottomOfScreen() ? -Size.Y - AdjustedSpacing : Size.Y + AdjustedSpacing))
				));
			}
			if (CurrentTime >= NotificationHudSettings.EntranceAnimationLength)
			{
				MovingUp = false;
				CurrentTime = 0;
				for (int i = 0; i < Active.Num(); i++)
				{
					Active[i]->StartHeight = Cast<UCanvasPanelSlot>(Active[i]->Slot)->GetPosition().Y;
				}
			}
		}
	}
}

void UAchievementHUD::Spawn(FAchievementData Achievement, FAchievementStates State)
{
	USoundBase * Sound = State.Achieved ? UnlockSound_Loaded : ProgressMadeSound_Loaded;
	if(IsValid(Sound))
	{
		UGameplayStatics::PlaySound2D(GetWorld(), Sound);
	}

	//Select override class if found
	TSubclassOf<UUIAchievement> SelectedUIAchievementClass = NotificationHudSettings.UiAchievementOverride ? NotificationHudSettings.UiAchievementOverride : UIAchievementClass;
	
	UUIAchievement *Temp = CreateWidget<UUIAchievement>(UGameplayStatics::GetPlayerController(GetWorld(), 0), SelectedUIAchievementClass);

	Temp->SetVisibility(Settings->MainNotificationProviderSettings.InteractBlockable? ESlateVisibility::Visible : ESlateVisibility::HitTestInvisible);
	
	Temp->SetValue(Achievement, State, Settings->bShowExactProgress, Settings);
	Active.Add(Temp);
	UCanvasPanelSlot *CSlot = MainNotificationPanel->AddChildToCanvas(Temp);
	
	Size = FVector2D(Temp->Root->WidthOverride, Temp->Root->HeightOverride);

	float XLocation = 0;
	EXScreenLocation location = GetXSpawnLocation();

	switch (location)
	{
		case EXScreenLocation::LEFT:
			XLocation = NotificationHudSettings.NotificationHorizontalMargin;
			break;
		case EXScreenLocation::RIGHT:
			XLocation = -Size.X - NotificationHudSettings.NotificationHorizontalMargin;
			break;
		case EXScreenLocation::CENTERED:
			//XLocation = (CSlot->GetSize().X * 0.5) + Size.X;
			XLocation = Size.X / -2;
			break;
	}

	SpawnLocation = FVector2D(
		XLocation,
		IsBottomOfScreen() ? -NotificationHudSettings.NotificationVerticalMargin : -Size.Y + NotificationHudSettings.NotificationVerticalMargin
	);
	
	CSlot->SetSize(Size);
	CSlot->SetAnchors(SpawnAnchors);
	CSlot->SetPosition(SpawnLocation);
	Temp->OnDeath.AddDynamic(this, &UAchievementHUD::OnAchievementDeath);
	Temp->StartDeathTimer(NotificationHudSettings.ExitAnimationLength, NotificationHudSettings.ScreenTime, NotificationHudSettings.ExitAnimation, IsRightOfScreen());
	Temp->StartHeight = SpawnLocation.Y;
	MovingUp = true;
}

void UAchievementHUD::OnAchievementDeath(UUIAchievement* Achievement)
{
	Achievement->OnDeath.RemoveAll(this);
	Active.Remove(Achievement);
	Achievement->RemoveFromParent();
}

void UAchievementHUD::NativeConstruct()
{
	Super::NativeConstruct();

	Settings = UAchievementSystemSettings::GetSettings();


	if(!OverrideSettings)
	{
		NotificationHudSettings = Settings->MainNotificationProviderSettings;
	}

	float XAnchor = 0;
	const EXScreenLocation Location = GetXSpawnLocation();

	switch (Location)
	{
		case EXScreenLocation::LEFT:
			XAnchor = 0;
			break;
		case EXScreenLocation::RIGHT:
			XAnchor = 1;
			break;
		case EXScreenLocation::CENTERED:
			XAnchor = 0.5;
			break;
	}

	SpawnAnchors = FAnchors(
		XAnchor,
		IsBottomOfScreen() ? 1 : 0
	);


	if(NotificationHudSettings.UiAchievementOverride)
	{
		UIAchievementClass = NotificationHudSettings.UiAchievementOverride;
	}

	if(NotificationHudSettings.UnlockSound.IsValid())
	{
		UnlockSound_Loaded = Cast<USoundBase>(NotificationHudSettings.UnlockSound.TryLoad());
	}
	if(NotificationHudSettings.ProgressMadeSound.IsValid())
	{
		ProgressMadeSound_Loaded = Cast<USoundBase>(NotificationHudSettings.ProgressMadeSound.TryLoad());
	}
}

bool UAchievementHUD::IsRightOfScreen()
{
	return NotificationHudSettings.ScreenLocation == EAchievementSceenLocation::TOP_RIGHT ||NotificationHudSettings.ScreenLocation == EAchievementSceenLocation::BOTTOM_RIGHT;
}

bool UAchievementHUD::IsBottomOfScreen()
{
	return NotificationHudSettings.ScreenLocation == EAchievementSceenLocation::BOTTOM_LEFT
	|| NotificationHudSettings.ScreenLocation == EAchievementSceenLocation::BOTTOM_RIGHT
	|| NotificationHudSettings.ScreenLocation == EAchievementSceenLocation::BOTTOM_CENTERED;
}

EXScreenLocation UAchievementHUD::GetXSpawnLocation()
{
	if(NotificationHudSettings.ScreenLocation == EAchievementSceenLocation::BOTTOM_LEFT || NotificationHudSettings.ScreenLocation == EAchievementSceenLocation::TOP_LEFT)
	{
		return EXScreenLocation::LEFT;
	}
	if (NotificationHudSettings.ScreenLocation == EAchievementSceenLocation::BOTTOM_RIGHT || NotificationHudSettings.ScreenLocation == EAchievementSceenLocation::TOP_RIGHT)
	{
		return EXScreenLocation::RIGHT;
	}

	return EXScreenLocation::CENTERED;
}