// Copyright 2021 Sam Carey. All Rights Reserved.

#pragma once
#include "CoreMinimal.h"
#include "Engine/Texture2D.h"
#include "AchievementData.generated.h"

#define LOCTEXT_NAMESPACE "Achievement System"

USTRUCT(BlueprintType, meta = (ToolTip = "Stores all data related to a single achievement"))
struct FAchievementData
{
	GENERATED_BODY()
public:
	FAchievementData()
	: LockedIcon(nullptr)
	, AchievedIcon(nullptr)
	, LockOverlay(false)
	, Spoiler(false)
	, Progress(false)
	, ProgressGoal(100)
	, NotificationFrequency(10)
	, IntProgress(false)
	, HighScoreProgress(false)
	{
	}
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ToolTip = "A unique key used to identify an achievement in blueprint/C++ code."), Category="Achievement Data")
		FName Key;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ToolTip = "The name of the achievement which the player will see in-game."), Category = "Achievement Data")
		FText DisplayName;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ToolTip = "A short description which tells the player how to earn the achievement."), Category = "Achievement Data")
		FText Description;
	//Icons
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ShortTooltip = "Icon to display when locked", ToolTip = "Icon to display when locked. Overlay this if LockOverlay is true"), Category = "Achievement Data")
		UTexture2D* LockedIcon;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ToolTip = "Icon to display when achieved"), Category = "Achievement Data")
		UTexture2D* AchievedIcon;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ToolTip = "If true, LockedIcon will be rendered on top of the achieved icon"), Category = "Achievement Data")
		bool LockOverlay = false;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ToolTip = "Treat the current achievement as a spoiler for the game. Hidden from player until unlocked."), Category = "Achievement Data")
		bool Spoiler = false;
	//Progress
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ToolTip = "If true this achievement will count to a certain amount before unlocking. E.g. race a total of 500 km, collect 10 coins or reach a high score of 25."), Category = "Achievement Data")
		bool Progress = false;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ToolTip = "The goal which must be reached for the achievement to unlock.", EditCondition = "Progress", EditConditionHides), Category = "Achievement Data")
		float ProgressGoal = 100;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ToolTip = "The rate that progress updates will be displayed on the screen e.g. Progress goal = 100 and Notification Frequency = 25. In this example, the progress will be displayed at 25,50,75 and 100.", EditCondition = "Progress", EditConditionHides), Category = "Achievement Data")
		float NotificationFrequency = 10;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ToolTip = "Treat this progress achievement as an integer(full number). Truncates value before displaying.", EditCondition = "Progress", EditConditionHides), Category = "Achievement Data")
		bool IntProgress = false;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ToolTip = "A string which will be displayed with a progress achievement e.g. $, KM, Miles etc", EditCondition = "Progress", EditConditionHides), Category = "Achievement Data")
		FText ProgressSuffix;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ToolTip = "If true the progress value will only update if larger then the save number. Should only be used in conjunction with SetAchievementProgress not AddAchievementProgress", EditCondition = "Progress", EditConditionHides), Category = "Achievement Data")
		bool HighScoreProgress = false;
	//Other
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ToolTip = "OtherData"), Category = "Achievement Data")
		TArray<FName> Tags;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, AdvancedDisplay, meta = (ToolTip = "Other custom data you way want to store about an achievement"), Category = "Achievement Data")
		TMap<FString, FString> OtherAchievementData;
};

#undef LOCTEXT_NAMESPACE