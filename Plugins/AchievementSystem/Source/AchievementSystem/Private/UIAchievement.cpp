// Copyright 2021 Sam Carey. All Rights Reserved.


#include "UIAchievement.h"

#include "Animation/WidgetAnimation.h"
#include "Kismet/KismetStringLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetTextLibrary.h"


void UUIAchievement::SetValue_Implementation(FAchievementData Achievement, FAchievementStates State, bool ShowExactProgress, UAchievementSystemSettings* Settings)
{
	Title->SetText(Achievement.DisplayName);
	Description->SetText(Achievement.Description);

	if (Achievement.LockOverlay && !State.Achieved)
	{
		OverlayIcon->SetBrushFromTexture(Achievement.LockedIcon);
		Icon->SetBrushFromTexture(Achievement.AchievedIcon);
	}
	else
	{
		OverlayIcon->SetVisibility(ESlateVisibility::Hidden);
		Icon->SetBrushFromTexture(State.Achieved ? Achievement.AchievedIcon : Achievement.LockedIcon);
	}

	if (Achievement.Progress)
	{
		const float CurrentProgress = ShowExactProgress ? State.Progress : State.LastProgressUpdate * Achievement.NotificationFrequency;
		const float DisplayProgress = State.Achieved ? Achievement.ProgressGoal : CurrentProgress;

		FString Progress;
		FString Goal;
		if(!Achievement.IntProgress)
		{
			Progress = UKismetTextLibrary::Conv_FloatToText(DisplayProgress, ERoundingMode::HalfFromZero, false, false, 1, 324, 0, Settings->MaxDecimalPlaces).ToString();
			Goal = FString::SanitizeFloat(Achievement.ProgressGoal);
		}
		else
		{
			Progress = UKismetStringLibrary::Conv_IntToString((int32)(DisplayProgress));
			Goal = UKismetStringLibrary::Conv_IntToString((int32)Achievement.ProgressGoal);
		}
		
		FText ProgressText = FText::Format(
			FText::FromString("{0}{1} / {2}{3}{4}{5}"),
			FText::FromString(Progress),
			Achievement.ProgressSuffix,
			FText::FromString(Goal),
			Achievement.ProgressSuffix,
			State.Achieved ? FText::FromString(" ") : FText::FromString(""),
			State.Achieved ? Settings->AchievedWord : FText::FromString(""));

		PercentText->SetText(ProgressText);
		ProgressBar->SetPercent(DisplayProgress / Achievement.ProgressGoal);
	}
	else
	{
		ProgressBar->SetPercent(State.Achieved ? 1 : 0);
		PercentText->SetText(State.Achieved ? Settings->AchievedWord : Settings->UnAchievedWord);
	}
}



void UUIAchievement::StartDeathTimer(float ExitAnimationLength, float ScreenTime, EAchievementAnimation ExitAnimation, bool SlideAnimationRight)
{
	FTimerDelegate TimerDel;
	UWidgetAnimation* TempAnimation = nullptr;
	
	switch(ExitAnimation)
	{
		case EAchievementAnimation::SLIDE:
			TempAnimation = (SlideAnimationRight ? SlideRight : SlideLeft);
			break;
		case EAchievementAnimation::FADE:
			TempAnimation = Fade;
			break;
		case EAchievementAnimation::SCALE_DOWN:
			TempAnimation = ScaleDown;
			break;
		case EAchievementAnimation::CUSTOM:
			TempAnimation = Custom;
			break;
		default: ;
	}
	
	TimerDel.BindUFunction(this, FName("OnRemoveStartEvent"), TempAnimation, ExitAnimationLength);
	GetWorld()->GetTimerManager().SetTimer(RemoveTimer, TimerDel, ScreenTime, false);
}

void UUIAchievement::OnRemoveStartEvent(UWidgetAnimation *Animation, float AnimationLength)
{
	if(IsValid(Animation))
	{
		PlayAnimation(Animation, 0, 1, EUMGSequencePlayMode::Forward, 1.0 / AnimationLength, false);
		GetWorld()->GetTimerManager().SetTimer(RemoveTimerAnimationComplete, this, &UUIAchievement::OnDeathAnimationComplete, AnimationLength, false);
	}
	else
	{
		OnDeathAnimationComplete();
	}
}

void UUIAchievement::OnDeathAnimationComplete()
{
	OnDeath.Broadcast(this);
}