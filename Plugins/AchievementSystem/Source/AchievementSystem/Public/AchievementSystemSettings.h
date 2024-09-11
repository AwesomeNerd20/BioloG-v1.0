// Copyright 2021 Sam Carey. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "Sound/SoundBase.h"
#include "AchievementData.h"
#include "AchievementSystemSettings.generated.h"


class UUIAchievement;
class UAchievementHUD;

struct AchievementStorageData
{
	FAchievementData Data;
	int Index = -1;
};

UENUM(BlueprintType)
enum class EAchievementSceenLocation : uint8
{
	TOP_LEFT 		UMETA(DisplayName = "Top Left"),
	TOP_RIGHT		UMETA(DisplayName = "Top Right"),
	TOP_CENTERED	UMETA(DisplayName = "Top Centered"),
	BOTTOM_LEFT		UMETA(DisplayName = "Bottom Left"),
	BOTTOM_RIGHT	UMETA(DisplayName = "Bottom Right"),
	BOTTOM_CENTERED UMETA(DisplayName = "Bottom Centered"),
};

UENUM(BlueprintType)
enum class EXScreenLocation : uint8
{
	LEFT,
	RIGHT,
	CENTERED
};

UENUM(BlueprintType)
enum class EAchievementAnimation : uint8
{
	FADE 			UMETA(DisplayName = "Fade"),
	DISAPPEAR		UMETA(DisplayName = "Disappear"),
	SLIDE			UMETA(DisplayName = "Slide"),
	SCALE_DOWN		UMETA(DisplayName = "Scale Down"),
	CUSTOM			UMETA(DisplayName = "Custom")
};


USTRUCT(BlueprintType)
struct FAchievementNotificationHudSettings
{
	GENERATED_BODY()
public:
	/**Define the corner of the screen that progress and unlock notifications will be displayed. */
	UPROPERTY(EditAnywhere, Category = "Achievement Unlock Notification")
	EAchievementSceenLocation ScreenLocation = EAchievementSceenLocation::BOTTOM_RIGHT;
	/**The total number of achievement notifications which can be on the screen at any one time.*/
	UPROPERTY(EditAnywhere, Category = "Achievement Unlock Notification")
	int TotalOnScreen = 3;
	/**The number of seconds an achievement will display on the screen once unlocked or progress is made.*/
	UPROPERTY(EditAnywhere, Category = "Achievement Unlock Notification")
	float ScreenTime = 3;
	/**The space between each achievement widget in the stack.*/
	UPROPERTY(EditAnywhere, Category = "Achievement Unlock Notification")
	float Spacing = 10;
	/**The space between achievement notifications*/
	UPROPERTY(EditAnywhere, Category = "Achievement Unlock Notification")
	/**The space between achievement notification and side of screen*/
	float NotificationHorizontalMargin = 5;
	/**The space between achievement notification and top/bottom of screen*/
	UPROPERTY(EditAnywhere, Category = "Achievement Unlock Notification")
	float NotificationVerticalMargin = 5;
	/**If true, notifications will be able to block UI under them*/
	UPROPERTY(EditAnywhere, Category = "Achievement Unlock Notification Animation")
	bool InteractBlockable = false;
	/**The animation that is played when notifications are removed from the screen. This can either be Fade, Disappear, Slide or Scale Down.*/
	UPROPERTY(EditAnywhere, Category = "Achievement Unlock Notification Animation")
	EAchievementAnimation ExitAnimation = EAchievementAnimation::SLIDE;
	/**The number of seconds an achievement notification takes to leave the screen.*/
	UPROPERTY(EditAnywhere, Category = "Achievement Unlock Notification Animation")
	float ExitAnimationLength = 0.5;
	/**The number of seconds an achievement notification takes to enter the screen.*/
	UPROPERTY(EditAnywhere, Category = "Achievement Unlock Notification Animation")
	float EntranceAnimationLength = 0.2;
	/**The sound that plays when an achievement is unlocked is displayed to a user. Only plays if ShowAchievementUnlocks is true.*/
	UPROPERTY(EditAnywhere, Category = "Achievement Unlock Notification Sound", meta = (AllowedClasses = "SoundBase"))
	FSoftObjectPath UnlockSound;
	/**The sound that plays when a progress update is displayed to a user. Only plays if ShowAchievementProgress is true.*/
	UPROPERTY(EditAnywhere, Category = "Achievement Unlock Notification Sound", meta = (AllowedClasses = "SoundBase"))
	FSoftObjectPath ProgressMadeSound;
	/**Use this achievement ui to display*/
	UPROPERTY(EditAnywhere, Category = "Achievement UI")
	TSubclassOf<UUIAchievement> UiAchievementOverride;
};

#define LOCTEXT_NAMESPACE "AchievementSystem"

/**
 * 
 */
UCLASS(config = Game, defaultconfig, Category = "Holds all settings for the achievement system")
class ACHIEVEMENTSYSTEM_API UAchievementSystemSettings : public UDeveloperSettings
{
	GENERATED_BODY()
public:
	UAchievementSystemSettings(const FObjectInitializer& ObjectInitializer);


	/**List of all achievement which can be unlocked by the player */
	UPROPERTY(config, BlueprintReadOnly, EditAnywhere, Category = "Achievements", meta=(TitleProperty = "Key"))
		TArray<FAchievementData> Achievements;

	/**If true, CompletionistAchievementKey will be unlocked when all other achievement have be gained*/
	UPROPERTY(config, BlueprintReadOnly, EditAnywhere, Category = "Achievements")
		bool bUseCompletionistAchievement = false;

	/**The key of the achievement to be unlocked when all others have be gained*/
	UPROPERTY(config, BlueprintReadOnly, EditAnywhere, Category = "Achievements")
		FName CompletionistAchievementKey = FName();


	/**If true, achievement unlock notifications will be displayed on the players screen. */
	UPROPERTY(config, BlueprintReadOnly, EditAnywhere, Category = "Achievement Unlock Notification - Main Provider")
		bool bShowAchievementUnlocks = true;
	/**If true, achievement progress update notifications will be displayed on the players screen. */
	UPROPERTY(config, BlueprintReadOnly, EditAnywhere, Category = "Achievement Unlock Notification - Main Provider")
		bool bShowAchievementProgress = true;

	UPROPERTY(config, BlueprintReadOnly, EditAnywhere, Category = "Achievement Unlock Notification - Main Provider")
	FAchievementNotificationHudSettings MainNotificationProviderSettings;
	
	/**If true, progress notifications will display their exact progress, if false it will show the closest bracket.*/
	UPROPERTY(config, BlueprintReadOnly,  EditAnywhere, Category = "Achievement UI")
		bool bShowExactProgress = false;
	/**The max number of decimal places which can be displayed on a UI Achievement*/
	UPROPERTY(config, BlueprintReadOnly, EditAnywhere, Category = "Achievement UI")
		int MaxDecimalPlaces = 2;
	/**Appends to the progress bar when an achievement has not been achieved*/
	UPROPERTY(config, BlueprintReadOnly, EditAnywhere, Category = "Achievement UI")
		FText AchievedWord = NSLOCTEXT("AchievementSystem", "AchievedWord", "(Achieved)");
	/**Appends to the progress bar when an achievement is achieved*/
	UPROPERTY(config, BlueprintReadOnly, EditAnywhere, Category = "Achievement UI")
		FText UnAchievedWord = NSLOCTEXT("AchievementSystem", "UnAchievedWord", "(Locked)");

	/**If true, the state of all achievements will be saved without any user input. If false, achievement states must be saved manually by calling  SaveAchievementState().*/
	UPROPERTY(config, BlueprintReadOnly, EditAnywhere, Category = "Saving")
		bool bAutoSave = true;
	/**The name of the save game slot used to keep track of achievement states.*/
	UPROPERTY(config, BlueprintReadOnly, EditAnywhere, Category = "Saving")
		FString SaveGameName = FString("SaveGame_Achievements");


	AchievementStorageData GetAchievementLocation(FName Key);

	static UAchievementSystemSettings* GetSettings();
	TArray<TSharedPtr<FName>> GetComboBoxNames();
};

#undef LOCTEXT_NAMESPACE