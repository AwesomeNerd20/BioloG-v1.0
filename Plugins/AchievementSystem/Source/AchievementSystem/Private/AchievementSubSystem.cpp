// Copyright 2021 Sam Carey. All Rights Reserved.

#include "AchievementSubSystem.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"

UAchievementSubSystem::UAchievementSubSystem()
{
	//Load Blueprint versions of UI elements
	static ConstructorHelpers::FClassFinder<UAchievementHUD> HUDClassHelper(TEXT("WidgetBlueprintGeneratedClass'/AchievementSystem/BP_AchievementHUD.BP_AchievementHUD_C'"));
	AchievementHUDClass = HUDClassHelper.Class;
}

void UAchievementSubSystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Settings = UAchievementSystemSettings::GetSettings();

	if(UGameplayStatics::DoesSaveGameExist(Settings->SaveGameName, 0))
	{
		SaveGame = Cast<UAchievementSaveGame>(UGameplayStatics::LoadGameFromSlot(Settings->SaveGameName, 0));
	}
	else
	{
		SaveGame = Cast<UAchievementSaveGame>(UGameplayStatics::CreateSaveGameObject(UAchievementSaveGame::StaticClass()));
	}
	SaveGame->States.SetNum(Settings->Achievements.Num(), true);
	UGameplayStatics::SaveGameToSlot(SaveGame, Settings->SaveGameName, 0);
}

bool UAchievementSubSystem::AchievementExists(FName Key)
{
	const AchievementStorageData Temp = Settings->GetAchievementLocation(Key);
	return Temp.Index != -1;
}

int32 UAchievementSubSystem::GetAchievedCount()
{
	return SaveGame->TotalUnlocked;
}

int32 UAchievementSubSystem::GetAchievedCountWithTag(FName Tag)
{
	int32 Count = 0;
	for(FAchievementData i : GetAllAchievementDataWithTag(Tag))
	{
		//TODO - Optimise this to not allocate new structs when not needed.
		bool Found;
		FAchievementStates State;
		FAchievementData Data;
		GetAchievementState(i.Key, Found, Data, State);

		if(Found)
		{
			if(State.Achieved)
			{
				Count++;
			}
		}
	}
	return Count;
}


int32 UAchievementSubSystem::GetAchievementCountWithTag(FName Tag)
{
	int32 Count = 0;
	for (FAchievementData &i : Settings->Achievements)
	{
		if(i.Tags.Contains(Tag))
		{
			Count++;
		}
	}
	return Count;
}

int32 UAchievementSubSystem::GetAchievementCount()
{
	return Settings->Achievements.Num();
}

UAchievementSystemSettings* UAchievementSubSystem::GetSettings()
{
	return UAchievementSystemSettings::GetSettings();
}

float UAchievementSubSystem::GetAchievedPercentage()
{
	const int32 AchievedCount = GetAchievedCount();
	const int32 Count = GetAchievementCount();
	if (Count == 0)
		return 100;
	
	return (float)AchievedCount / Count * 100;
}

float UAchievementSubSystem::GetAchievedPercentageWithTag(FName Tag)
{
	const int32 AchievedCount = GetAchievedCountWithTag(Tag);
	const int32 Count = GetAchievementCountWithTag(Tag);
	if (Count == 0)
		return 100;
	
	return (float)AchievedCount / Count * 100;
}

void UAchievementSubSystem::GetAchievementState(FName Key, bool &FoundAchievement, FAchievementData &Data, FAchievementStates& State)
{
	FoundAchievement = false;
	AchievementStorageData i = Settings->GetAchievementLocation(Key);
	if (i.Index == -1)
		return;

	const FAchievementData Current = i.Data;
	
	State = SaveGame->States[i.Index];
	Data = Current;
	FoundAchievement = true;
}

FAchievementData UAchievementSubSystem::GetAchievementDataByKey(FName Key, bool& FoundAchievement)
{
	FoundAchievement = false;
	AchievementStorageData i = Settings->GetAchievementLocation(Key);
	if (i.Index == -1)
		return FAchievementData();
		
	FoundAchievement = true;
	return i.Data;
}

TArray<FAchievementData> UAchievementSubSystem::GetAllAchievementData()
{
	return Settings->Achievements;
}

TArray<FAchievementData> UAchievementSubSystem::GetAllAchievementDataWithTag(FName Tag)
{
	TArray<FAchievementData> NewList;

	for (FAchievementData i : Settings->Achievements)
	{
		if(i.Tags.Contains(Tag))
		{
			NewList.Add(i);
		}
	}
	return NewList;
}

TArray<FName> UAchievementSubSystem::GetUniqueTags()
{
	TArray<FName> TagList;

	for (FAchievementData i : Settings->Achievements)
	{
		for(FName Tag : i.Tags)
		{
			if(TagList.Contains(Tag))
				continue;
			TagList.Add(Tag);
		}
	}
	return TagList;
}

void UAchievementSubSystem::Unlock(FName Key)
{
	AchievementStorageData i = Settings->GetAchievementLocation(Key);
	if(i.Index == -1)
		return;
	
	const FAchievementData Current = i.Data;
	int32 index = i.Index;
	
	if (!SaveGame->States[index].Achieved)
	{
		SaveGame->TotalUnlocked += 1;
		SaveGame->States[index].Progress = Current.ProgressGoal;
		SaveGame->States[index].Achieved = true;
		OnAchievementUnlock.Broadcast(Key);
		DisplayProgress(Current, index);
		TryAutoSaveStates();

		if(Settings->bUseCompletionistAchievement && SaveGame->TotalUnlocked == Settings->Achievements.Num() - 1)
		{
			Unlock(Settings->CompletionistAchievementKey);
		}
	}
}

void UAchievementSubSystem::SetAchievementProgress(FName Key, float Progress)
{
	AchievementStorageData i = Settings->GetAchievementLocation(Key);
	if (i.Index == -1)
		return;
	
	const FAchievementData Current = i.Data;
	int32 index = i.Index;
	
	if (Current.Progress)
	{
		if (Progress >= Current.ProgressGoal)
		{
			Unlock(Key);
		}
		else
		{
			if(Current.HighScoreProgress)
			{
				if(Progress < SaveGame->States[index].Progress)
					return;
			}
			
			SaveGame->States[index].Progress = Progress;
			OnAchievementProgressUpdate.Broadcast(Key, SaveGame->States[index].Progress, Progress);
			DisplayProgress(Current, index);
		}
	}
	TryAutoSaveStates();
}

void UAchievementSubSystem::AddAchievementProgress(FName Key, float Progress)
{
	AchievementStorageData i = Settings->GetAchievementLocation(Key);
	if (i.Index == -1)
		return;
	
	const FAchievementData Current = i.Data;
	int32 index = i.Index;

	if (Current.Progress)
	{
		if (SaveGame->States[index].Progress + Progress >= Current.ProgressGoal)
		{
			Unlock(Key);
		}
		else
		{
			SaveGame->States[index].Progress += Progress;
			OnAchievementProgressUpdate.Broadcast(Key, SaveGame->States[index].Progress, Progress);
			DisplayProgress(Current, index);
			TryAutoSaveStates();
		}
	}
}

void UAchievementSubSystem::SaveAchievementState()
{
	UGameplayStatics::SaveGameToSlot(SaveGame, Settings->SaveGameName, 0);
}

void UAchievementSubSystem::ResetAchievementStates()
{
	UGameplayStatics::DeleteGameInSlot(Settings->SaveGameName, 0);
	SaveGame = Cast<UAchievementSaveGame>(UGameplayStatics::CreateSaveGameObject(UAchievementSaveGame::StaticClass()));
	SaveGame->States.SetNum(Settings->Achievements.Num(), false);
	UGameplayStatics::SaveGameToSlot(SaveGame, Settings->SaveGameName, 0);
}

void UAchievementSubSystem::RegisterNotificationProvider(UAchievementHUD* Provider)
{
	if (NotificationProviders.Contains(Provider))
	{
		UE_LOG(LogTemp, Warning, TEXT("Notification Provider \"%s\" is already registered"), *Provider->GetName());
		return;
	}

	NotificationProviders.Add(Provider);
	UE_LOG(LogTemp, Warning, TEXT("Notification Provider \"%s\" has been registered with the achievement system successfully"), *Provider->GetName());
}

void UAchievementSubSystem::UnRegisterNotificationProvider(UAchievementHUD* Provider)
{
	if (NotificationProviders.Contains(Provider))
	{
		NotificationProviders.Remove(Provider);
		UE_LOG(LogTemp, Display, TEXT("Notification Provider \"%s\" has been unregistered from the achievement system successfully"), *Provider->GetName());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Notification Provider \"%s\" has been not unregistered as it was not registered"), *Provider->GetName());
	}
}

void UAchievementSubSystem::Tick(float DeltaTime)
{
	if (IsValid(Screen))
	{
		//Add back into viewport on level load
		if (!Screen->IsInViewport())
		{
			Screen->AddToViewport();
		}
		Screen->UpdateHUD(DeltaTime);

		for (UAchievementHUD* hud : NotificationProviders)
		{
			hud->UpdateHUD(DeltaTime);
		}
	}
}

void UAchievementSubSystem::TryAutoSaveStates()
{
	if(Settings->bAutoSave)
	{
		SaveAchievementState();
	}
}

void UAchievementSubSystem::DisplayProgress(const FAchievementData& Achievement, int Index)
{
	if((Settings->bShowAchievementUnlocks && SaveGame->States[Index].Achieved) || (Settings->bShowAchievementProgress && !SaveGame->States[Index].Achieved))
	{
		if (!Achievement.Spoiler || SaveGame->States[Index].Achieved)
		{
			UAchievementHUD* HUD = GetHUD();
			if (Achievement.Progress && SaveGame->States[Index].Progress < Achievement.ProgressGoal)
			{
				if(Achievement.NotificationFrequency > 0)
				{
					const int32 Steps = (int32)Achievement.ProgressGoal / (int32)Achievement.NotificationFrequency;

					//Loop from biggest notification bracket to the smallest
					for (int32 i = Steps; i > SaveGame->States[Index].LastProgressUpdate; i--)
					{
						if (SaveGame->States[Index].Progress >= Achievement.NotificationFrequency * i)
						{
							SaveGame->States[Index].LastProgressUpdate = i;
							HUD->ScheduleAchievementDisplay(Achievement, SaveGame->States[Index]);

							for (UAchievementHUD* Provider : NotificationProviders)
							{
								Provider->ScheduleAchievementDisplay(Achievement, SaveGame->States[Index]);
							}
							return;
						}
					}
				}
			}
			else
			{
				HUD->ScheduleAchievementDisplay(Achievement, SaveGame->States[Index]);
				for(UAchievementHUD * Provider : NotificationProviders)
				{
					Provider->ScheduleAchievementDisplay(Achievement, SaveGame->States[Index]);
				}
			}
		}
	}
}


UAchievementHUD* UAchievementSubSystem::GetHUD()
{
	if(IsValid(Screen))
	{
		//Add back into viewport on level load
		if(!Screen->IsInViewport())
		{
			Screen->AddToViewport();
		}
		return Screen;
	}
	Screen = CreateWidget<UAchievementHUD>(UGameplayStatics::GetPlayerController(GetWorld(), 0), AchievementHUDClass);
	Screen->AddToViewport();
	
	return Screen;
}