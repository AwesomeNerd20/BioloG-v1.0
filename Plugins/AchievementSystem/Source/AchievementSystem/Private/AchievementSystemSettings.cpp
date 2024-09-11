// Copyright 2021 Sam Carey. All Rights Reserved.


#include "AchievementSystemSettings.h"

UAchievementSystemSettings::UAchievementSystemSettings(const FObjectInitializer& ObjectInitializer)
{
	if(!MainNotificationProviderSettings.UnlockSound.IsValid())
	{
		MainNotificationProviderSettings.UnlockSound = FString("/AchievementSystem/Example/Sound/A_Default_Unlock_Cue.A_Default_Unlock_Cue");
	}
	if(!MainNotificationProviderSettings.ProgressMadeSound.IsValid())
	{
		MainNotificationProviderSettings.ProgressMadeSound = FString("/AchievementSystem/Example/Sound/A_Default_Progress_Cue.A_Default_Progress_Cue");
	}
}

AchievementStorageData UAchievementSystemSettings::GetAchievementLocation(FName Key)
{
	AchievementStorageData location = AchievementStorageData();
	for(int i = 0; i < Achievements.Num(); i++)
	{
		if(Achievements[i].Key == Key)
		{
			location.Data = Achievements[i];
			location.Index = i;
		}
	}
	return location;
}

UAchievementSystemSettings* UAchievementSystemSettings::GetSettings()
{
	return GetMutableDefault<UAchievementSystemSettings>();
}

TArray<TSharedPtr<FName>> UAchievementSystemSettings::GetComboBoxNames()
{
	TArray<TSharedPtr<FName>> NameList;
	for(FAchievementData i : Achievements)
	{
		if(CompletionistAchievementKey != i.Key)
		{
			TSharedPtr<FName> NameKey = MakeShareable(new FName(i.Key));
			NameList.Add(NameKey);
		}
	}
	return NameList;
}