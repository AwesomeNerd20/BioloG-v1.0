// Copyright 2021 Sam Carey. All Rights Reserved..

#pragma once

#include "CoreMinimal.h"

#include "AchievementData.h"
#include "AchievementStates.h"
#include "AchievementSystemSettings.h"
#include "Blueprint/UserWidget.h"
#include "Components/CanvasPanel.h"
#include "AchievementHUD.generated.h"


USTRUCT()
struct FAchievementPackage
{
	GENERATED_BODY()
	FAchievementData Data;
	FAchievementStates State;
};

/**
 * 
 */
UCLASS()
class ACHIEVEMENTSYSTEM_API UAchievementHUD : public UUserWidget
{
	GENERATED_BODY()
public:
	UAchievementHUD(const FObjectInitializer& ObjectInitializer);
	
	UFUNCTION(BlueprintCallable, Category = "Achievement HUD")
	void ScheduleAchievementDisplay (FAchievementData Achievement, FAchievementStates State);

	void UpdateHUD(float InDeltaTime);

	UPROPERTY(EditAnywhere, Category="Achievement Notification Hud")
	bool OverrideSettings = false;
	UPROPERTY(EditAnywhere, Category="Achievement Notification Hud", meta=(EditCondition="OverrideSettings", EditConditionHides))
	FAchievementNotificationHudSettings NotificationHudSettings;

	UPROPERTY(meta = (BindWidget), BlueprintReadOnly, Category="Compoents")
	UCanvasPanel* MainNotificationPanel;
protected:
	virtual void NativeConstruct() override;
	

private:
	
	UPROPERTY()
	UAchievementSystemSettings* Settings;

	float CurrentTime;
	bool MovingUp;
	
	//Default spawning location for UIAchievement
	FVector2D SpawnLocation = FVector2D(0);
	FAnchors SpawnAnchors = FAnchors(0);
	FVector2D Size = FVector2D(0);


	//List to store active achievement + backlogged data 
	UPROPERTY()
	TArray<UUIAchievement*> Active;
	UPROPERTY()
	TArray<FAchievementPackage> BackLog;


	TSubclassOf<UUIAchievement> UIAchievementClass;

	UFUNCTION()
	void OnAchievementDeath(UUIAchievement* Achievement);
	void Spawn(FAchievementData Achievement, FAchievementStates State);

	UPROPERTY()
	USoundBase* UnlockSound_Loaded;
	UPROPERTY()
	USoundBase* ProgressMadeSound_Loaded;

	bool IsRightOfScreen();
	bool IsBottomOfScreen();
	EXScreenLocation GetXSpawnLocation();
};