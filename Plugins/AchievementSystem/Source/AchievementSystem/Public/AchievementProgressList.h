// Copyright 2021 Sam Carey. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/ScrollBox.h"
#include "Components/TextBlock.h"
#include "AchievementSubSystem.h"
#include "AchievementProgressList.generated.h"


UENUM(BlueprintType)
enum class EAchievementProgressFilter : uint8
{
	ANY 		UMETA(DisplayName = "Show All"),
	ACHIEVED	UMETA(DisplayName = "Achieved Only"),
	UNACHIEVED	UMETA(DisplayName = "Unachieved Only"),
};

/**
 * 
 */
UCLASS()
class ACHIEVEMENTSYSTEM_API UAchievementProgressList : public UUserWidget
{
	GENERATED_BODY()
public:
	UAchievementProgressList(const FObjectInitializer& ObjectInitializer);
	
	/**Change the type of achievements to show. Must all UpdateList() after changing is done at runtime*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Details", meta=(EditCondition="FilterByProgress", EditConditionHides))
		EAchievementProgressFilter Filter;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Details")
	bool FilterByTag = false;
	/**Change achievements to shown based on tag. Must all UpdateList() after changing is done at runtime*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Details", meta=(EditCondition="FilterByTag", EditConditionHides))
		FName FilterTag;
	UPROPERTY(EditAnywhere, Category = "Details")
		TSubclassOf<UUIAchievement> UiAchievementOverride;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Details")
		bool bShowHiddenAchievementCount = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Details", meta=(EditCondition="bShowHiddenAchievementCount", EditConditionHides))
		FText HiddenAchievementsWordSingular = NSLOCTEXT("AchievementSystem", "HiddenAchievementsWordSingular", "Hidden Achievement");
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Details", meta=(EditCondition="bShowHiddenAchievementCount", EditConditionHides))
		FText HiddenAchievementsWordMultiple = NSLOCTEXT("AchievementSystem", "HiddenAchievementsWordMultiple", "Hidden Achievements");;
	UPROPERTY(EditAnywhere, Category = "Details")
		float Spacing = 10;
	
	UFUNCTION(BlueprintCallable, Category = "Achievement Progress List")
		void UpdateList();
	
protected:
	virtual void NativePreConstruct() override;
	virtual void NativeConstruct() override;
private:
	UPROPERTY(meta = (BindWidget))
		UScrollBox* ListView;
	
	UPROPERTY(meta = (BindWidget))
		UTextBlock* HiddenTextText;
	UPROPERTY(meta = (BindWidget))
		UTextBlock* HiddenTextNumber;

	UPROPERTY()
	UAchievementSubSystem* SubSystem;
	UPROPERTY()
	UAchievementSystemSettings* Settings;

	TSubclassOf<UUIAchievement> UIAchievementClass;
};