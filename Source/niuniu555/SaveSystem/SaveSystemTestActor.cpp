// Copyright Epic Games, Inc. All Rights Reserved.

#include "SaveSystemTestActor.h"
#include "SaveGameSubsystem.h"
#include "NiuniuSaveGame.h"
#include "Kismet/GameplayStatics.h"

// ========== ASaveSystemTestActor ==========

ASaveSystemTestActor::ASaveSystemTestActor()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ASaveSystemTestActor::BeginPlay()
	{
	Super::BeginPlay();

	// 绑定事件
	if (USaveGameSubsystem* SaveSubsystem = GetSaveSubsystem())
	{
		if (!bEventsBound)
		{
			SaveSubsystem->OnSaveOperationCompleted.AddDynamic(this, &ASaveSystemTestActor::OnSaveCompleted);
			SaveSubsystem->OnLoadOperationCompleted.AddDynamic(this, &ASaveSystemTestActor::OnLoadCompleted);
			bEventsBound = true;
			UE_LOG(LogTemp, Log, TEXT("SaveSystemTestActor: Events bound"));
		}
	}
}

USaveGameSubsystem* ASaveSystemTestActor::GetSaveSubsystem() const
{
	if (UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this))
	{
		return GameInstance->GetSubsystem<USaveGameSubsystem>();
	}
	return nullptr;
}

void ASaveSystemTestActor::TestCreateSave(int32 SlotIndex, const FString& SaveName)
{
	if (USaveGameSubsystem* SaveSubsystem = GetSaveSubsystem())
	{
		UE_LOG(LogTemp, Log, TEXT("Test: Creating save at slot %d with name '%s'"), SlotIndex, *SaveName);
		SaveSubsystem->CreateNewSaveGame(SlotIndex, SaveName);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Test: SaveSubsystem not found!"));
	}
}

void ASaveSystemTestActor::TestLoadSave(int32 SlotIndex)
{
	if (USaveGameSubsystem* SaveSubsystem = GetSaveSubsystem())
	{
		UE_LOG(LogTemp, Log, TEXT("Test: Loading save from slot %d"), SlotIndex);
		SaveSubsystem->LoadSaveGame(SlotIndex);
	}
}

void ASaveSystemTestActor::TestSaveCurrentGame()
{
	if (USaveGameSubsystem* SaveSubsystem = GetSaveSubsystem())
	{
		if (SaveSubsystem->HasCurrentSaveGame())
		{
			UE_LOG(LogTemp, Log, TEXT("Test: Saving current game..."));
			SaveSubsystem->SaveCurrentGame();
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Test: No current save game to save!"));
		}
	}
}

void ASaveSystemTestActor::TestDeleteSave(int32 SlotIndex)
{
	if (USaveGameSubsystem* SaveSubsystem = GetSaveSubsystem())
	{
		const bool bSuccess = SaveSubsystem->DeleteSaveGame(SlotIndex);
		UE_LOG(LogTemp, Log, TEXT("Test: Delete save at slot %d - %s"), SlotIndex, bSuccess ? TEXT("Success") : TEXT("Failed"));
	}
}

void ASaveSystemTestActor::TestAddGold(int32 Amount)
{
	if (USaveGameSubsystem* SaveSubsystem = GetSaveSubsystem())
	{
		SaveSubsystem->UpdateGold(Amount);
		UE_LOG(LogTemp, Log, TEXT("Test: Added %d gold"), Amount);
	}
}

void ASaveSystemTestActor::TestAddGodFragments(int32 Amount)
{
	if (USaveGameSubsystem* SaveSubsystem = GetSaveSubsystem())
	{
		SaveSubsystem->AddGodFragments(Amount);
		UE_LOG(LogTemp, Log, TEXT("Test: Added %d god fragments"), Amount);
	}
}

void ASaveSystemTestActor::TestUnlockSkill(const FString& SkillId)
{
	if (USaveGameSubsystem* SaveSubsystem = GetSaveSubsystem())
	{
		SaveSubsystem->UnlockSkill(FName(*SkillId));
		UE_LOG(LogTemp, Log, TEXT("Test: Unlocked skill '%s'"), *SkillId);
	}
}

void ASaveSystemTestActor::TestUpdateChapter(int32 ChapterIndex)
{
	if (USaveGameSubsystem* SaveSubsystem = GetSaveSubsystem())
	{
		SaveSubsystem->UpdateHighestChapter(ChapterIndex);
		UE_LOG(LogTemp, Log, TEXT("Test: Updated highest chapter to %d"), ChapterIndex);
	}
}

void ASaveSystemTestActor::TestPrintAllSaveInfo()
{
	if (USaveGameSubsystem* SaveSubsystem = GetSaveSubsystem())
	{
		TArray<UNiuniuSaveGame*> SaveGames = SaveSubsystem->GetAllSaveGamesInfo();
		
		UE_LOG(LogTemp, Log, TEXT("=== All Save Games Info ==="));
		UE_LOG(LogTemp, Log, TEXT("Total saves found: %d"), SaveGames.Num());
		
		for (UNiuniuSaveGame* SaveGame : SaveGames)
		{
			if (SaveGame)
			{
				UE_LOG(LogTemp, Log, TEXT("Slot %d: %s | Gold: %d | Fragments: %d | Chapter: %d"),
					SaveGame->SlotIndex,
					*SaveGame->SaveDisplayName,
					SaveGame->PlayerData.TotalGold,
					SaveGame->PlayerData.GodFragmentCount,
					SaveGame->PlayerData.HighestClearedChapter);
			}
		}
		UE_LOG(LogTemp, Log, TEXT("=========================="));
	}
}

void ASaveSystemTestActor::TestPrintCurrentSaveInfo()
{
	if (USaveGameSubsystem* SaveSubsystem = GetSaveSubsystem())
	{
		if (UNiuniuSaveGame* CurrentSave = SaveSubsystem->GetCurrentSaveGame())
		{
			UE_LOG(LogTemp, Log, TEXT("=== Current Save Info ==="));
			UE_LOG(LogTemp, Log, TEXT("Name: %s"), *CurrentSave->SaveDisplayName);
			UE_LOG(LogTemp, Log, TEXT("Slot: %d"), CurrentSave->SlotIndex);
			UE_LOG(LogTemp, Log, TEXT("Gold: %d"), CurrentSave->PlayerData.TotalGold);
			UE_LOG(LogTemp, Log, TEXT("God Fragments: %d"), CurrentSave->PlayerData.GodFragmentCount);
			UE_LOG(LogTemp, Log, TEXT("Highest Chapter: %d"), CurrentSave->PlayerData.HighestClearedChapter);
			UE_LOG(LogTemp, Log, TEXT("Play Time: %s"), *CurrentSave->GetFormattedPlayTime());
			UE_LOG(LogTemp, Log, TEXT("Unlocked Skills: %d"), CurrentSave->PlayerData.UnlockedSkills.Num());
			UE_LOG(LogTemp, Log, TEXT("======================="));
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("No current save game loaded!"));
		}
	}
}

void ASaveSystemTestActor::OnSaveCompleted(bool bSuccess, const FString& Message)
{
	UE_LOG(LogTemp, Log, TEXT("Save Operation: %s - %s"), bSuccess ? TEXT("SUCCESS") : TEXT("FAILED"), *Message);
}

void ASaveSystemTestActor::OnLoadCompleted(bool bSuccess, const FString& Message)
{
	UE_LOG(LogTemp, Log, TEXT("Load Operation: %s - %s"), bSuccess ? TEXT("SUCCESS") : TEXT("FAILED"), *Message);
}

// ========== USaveAutoSaveComponent ==========

USaveAutoSaveComponent::USaveAutoSaveComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void USaveAutoSaveComponent::BeginPlay()
{
	Super::BeginPlay();

	if (bEnableAutoSave && AutoSaveInterval > 0)
	{
		StartAutoSave(AutoSaveInterval);
	}
}

void USaveAutoSaveComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	StopAutoSave();
	Super::EndPlay(EndPlayReason);
}

void USaveAutoSaveComponent::StartAutoSave(float Interval)
{
	if (UWorld* World = GetWorld())
	{
		StopAutoSave(); // 先停止现有的定时器
		
		World->GetTimerManager().SetTimer(
			AutoSaveTimerHandle,
			this,
			&USaveAutoSaveComponent::OnAutoSaveTimer,
			Interval,
			true // 循环执行
		);
		
		UE_LOG(LogTemp, Log, TEXT("AutoSave started with interval %.1f seconds"), Interval);
	}
}

void USaveAutoSaveComponent::StopAutoSave()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(AutoSaveTimerHandle);
	}
}

void USaveAutoSaveComponent::DoAutoSaveNow()
{
	OnAutoSaveTimer();
}

void USaveAutoSaveComponent::OnAutoSaveTimer()
{
	if (UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this))
	{
		if (USaveGameSubsystem* SaveSubsystem = GameInstance->GetSubsystem<USaveGameSubsystem>())
		{
			if (SaveSubsystem->HasCurrentSaveGame())
			{
				UE_LOG(LogTemp, Log, TEXT("AutoSave: Saving game..."));
				SaveSubsystem->SaveCurrentGame();
			}
		}
	}
}
