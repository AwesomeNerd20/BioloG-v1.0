// Copyright 2021 Sam Carey. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AchievementHUD.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "AchievementSaveGame.h"
#include "AchievementSystemSettings.h"
#include "Tickable.h"
#include "AchievementSubSystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAchievementUnlock, FName, AchievementName);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnAchievementProgressUpdate, FName, AchievementName, float, NewProgress, float, Change);

/**
 * 
 */
UCLASS()
class ACHIEVEMENTSYSTEM_API UAchievementSubSystem : public UGameInstanceSubsystem, public FTickableGameObject
{
	GENERATED_BODY()
public:
	UAchievementSubSystem();
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	//Miscellaneous
	/**Returns true if an achievement is found in the list.*/
	UFUNCTION(BlueprintCallable, Category="Achievements")
		bool AchievementExists(FName Key);
	/**Returns the total number of achievements which have been unlocked.*/
	UFUNCTION(BlueprintCallable, Category = "Achievements")
		int32 GetAchievedCount();
	UFUNCTION(BlueprintCallable, Category = "Achievements")
		int32 GetAchievedCountWithTag(FName Tag);
	UFUNCTION(BlueprintCallable, Category = "Achievements")
		int32 GetAchievementCountWithTag(FName Tag);
	UFUNCTION(BlueprintCallable, Category = "Achievements")
		int32 GetAchievementCount();
	UFUNCTION(BlueprintCallable, Category = "Achievements")
		UAchievementSystemSettings* GetSettings();
	/**Returns the current percentage of unlocked achievements.*/
	/**Returns the current percentage of unlocked achievements.*/
	UFUNCTION(BlueprintCallable, Category = "Achievements")
		float GetAchievedPercentage();
	/**Returns the current percentage of unlocked achievements with a given tag*/
	UFUNCTION(BlueprintCallable, Category = "Achievements")
		float GetAchievedPercentageWithTag(FName Tag);
	
	/**Returns the state and data for a given achievement*/
	UFUNCTION(BlueprintCallable, Category = "Achievements")
		void GetAchievementState(FName Key, bool& FoundAchievement, FAchievementData& Data, FAchievementStates& State);
	/**Returns the data for a given achievement*/
	UFUNCTION(BlueprintCallable, Category = "Achievements")
		FAchievementData GetAchievementDataByKey(FName Key, bool& FoundAchievement);
	/**Returns a list of data for achievements*/
	UFUNCTION(BlueprintCallable, Category = "Achievements")
		TArray<FAchievementData> GetAllAchievementData();
	/**Returns a list of achievements which have a specific tag.*/
	UFUNCTION(BlueprintCallable, Category = "Achievements")
		TArray<FAchievementData> GetAllAchievementDataWithTag(FName Tag);
	/**Returns a list of all unique tags used in achievement definitions*/
	UFUNCTION(BlueprintCallable, Category = "Achievements")
		TArray<FName> GetUniqueTags();
	
	//Unlocking/Progress
	/**Fully unlocks a progress or goal achievement.*/
	UFUNCTION(BlueprintCallable, Category = "Achievements")
		void Unlock(FName Key);
	/**Set the progress of an achievement to a specific value.*/
	UFUNCTION(BlueprintCallable, Category = "Achievements")
		void SetAchievementProgress(FName Key, float Progress);
	/**Adds the input amount of progress to an achievement. Clamps achievement progress to its max value.*/
	UFUNCTION(BlueprintCallable, Category = "Achievements")
		void AddAchievementProgress(FName Key, float Progress);

	//Saving and Loading
	/**Saves progress and achieved states to player prefs. This function is automatically called if the Auto Save setting is set to true.*/
	UFUNCTION(BlueprintCallable, Category = "Achievements")
		void SaveAchievementState();
	/**Clears all saved progress and achieved states.*/
	UFUNCTION(BlueprintCallable, Category = "Achievements")
		void ResetAchievementStates();

	UFUNCTION(BlueprintCallable, Category = "Achievement Notification Provider")
		void RegisterNotificationProvider(UAchievementHUD *Provider);
	UFUNCTION(BlueprintCallable, Category = "Achievement Notification Provider")
		void UnRegisterNotificationProvider(UAchievementHUD* Provider);

	UPROPERTY()
	UAchievementSaveGame* SaveGame;

	UPROPERTY(BlueprintAssignable, Category="Achievements")
		FOnAchievementProgressUpdate OnAchievementProgressUpdate;
	UPROPERTY(BlueprintAssignable, Category = "Achievements")
		FOnAchievementUnlock OnAchievementUnlock;

	virtual void Tick(float DeltaTime) override;
	virtual bool IsTickable() const override { return !IsTemplate(); }
	virtual TStatId GetStatId() const override { RETURN_QUICK_DECLARE_CYCLE_STAT(UBenchmarkSubsystem, STATGROUP_Tickables); }

private:
	void TryAutoSaveStates();

	void DisplayProgress(const FAchievementData& Achievement, int Index);
	UAchievementHUD* GetHUD();
	
	TSubclassOf<UAchievementHUD> AchievementHUDClass;
	
	UPROPERTY()
	UAchievementSystemSettings *Settings;
	UPROPERTY()
	UAchievementHUD* Screen;
	
	UPROPERTY()
	TArray<UAchievementHUD *> NotificationProviders;
};