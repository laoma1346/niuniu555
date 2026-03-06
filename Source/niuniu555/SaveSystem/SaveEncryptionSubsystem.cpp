// Copyright Epic Games, Inc. All Rights Reserved.

#include "SaveEncryptionSubsystem.h"
#include "SaveEncryptionUtil.h"
#include "NiuniuSaveGame.h"
#include "HAL/PlatformFileManager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

const FString USaveEncryptionSubsystem::EncryptedSaveExtension = TEXT(".niuniusave");

void USaveEncryptionSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	UE_LOG(LogTemp, Log, TEXT("SaveEncryptionSubsystem: Initialized"));
}

void USaveEncryptionSubsystem::Deinitialize()
{
	Super::Deinitialize();
	UE_LOG(LogTemp, Log, TEXT("SaveEncryptionSubsystem: Deinitialized"));
}

FString USaveEncryptionSubsystem::GetEncryptedSavePath(int32 SlotIndex) const
{
	return FPaths::ProjectSavedDir() + TEXT("SaveGames/") + 
		   FString::Printf(TEXT("EncryptedSave_%d%s"), SlotIndex, *EncryptedSaveExtension);
}

bool USaveEncryptionSubsystem::SaveGameEncrypted(int32 SlotIndex, UNiuniuSaveGame* SaveGameObject, const FString& CustomKey)
{
	if (!SaveGameObject)
	{
		UE_LOG(LogTemp, Warning, TEXT("SaveGameEncrypted: SaveGameObject is null"));
		return false;
	}

	// 将SaveGame序列化为内存
	TArray<uint8> SaveData;
	FMemoryWriter MemoryWriter(SaveData, true);
	SaveGameObject->Serialize(MemoryWriter);

	// 加密数据
	TArray<uint8> EncryptedData;
	if (bEnableEncryption)
	{
		const FString KeyToUse = CustomKey.IsEmpty() ? CustomEncryptionKey : CustomKey;
		EncryptedData = FSaveEncryptionUtil::EncryptData(SaveData, KeyToUse);
	}
	else
	{
		EncryptedData = SaveData;
	}

	// 写入文件
	const FString SavePath = GetEncryptedSavePath(SlotIndex);
	const FString DirectoryPath = FPaths::GetPath(SavePath);
	
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	if (!PlatformFile.DirectoryExists(*DirectoryPath))
	{
		PlatformFile.CreateDirectoryTree(*DirectoryPath);
	}

	return FFileHelper::SaveArrayToFile(EncryptedData, *SavePath);
}

UNiuniuSaveGame* USaveEncryptionSubsystem::LoadGameEncrypted(int32 SlotIndex, const FString& CustomKey)
{
	const FString SavePath = GetEncryptedSavePath(SlotIndex);
	
	// 检查文件是否存在
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	if (!PlatformFile.FileExists(*SavePath))
	{
		UE_LOG(LogTemp, Warning, TEXT("LoadGameEncrypted: Save file not found at %s"), *SavePath);
		return nullptr;
	}

	// 读取加密文件
	TArray<uint8> EncryptedData;
	if (!FFileHelper::LoadFileToArray(EncryptedData, *SavePath))
	{
		UE_LOG(LogTemp, Warning, TEXT("LoadGameEncrypted: Failed to load file"));
		return nullptr;
	}

	// 解密数据
	TArray<uint8> DecryptedData;
	if (bEnableEncryption)
	{
		const FString KeyToUse = CustomKey.IsEmpty() ? CustomEncryptionKey : CustomKey;
		DecryptedData = FSaveEncryptionUtil::DecryptData(EncryptedData, KeyToUse);
	}
	else
	{
		DecryptedData = EncryptedData;
	}

	// 反序列化SaveGame
	UNiuniuSaveGame* LoadedSave = NewObject<UNiuniuSaveGame>();
	FMemoryReader MemoryReader(DecryptedData, true);
	LoadedSave->Serialize(MemoryReader);

	return LoadedSave;
}

void USaveEncryptionSubsystem::AsyncSaveGameEncrypted(int32 SlotIndex, UNiuniuSaveGame* SaveGameObject, const FString& CustomKey)
{
	// 在后台线程执行保存
	AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [this, SlotIndex, SaveGameObject, CustomKey]()
	{
		const bool bSuccess = SaveGameEncrypted(SlotIndex, SaveGameObject, CustomKey);
		
		// 回调到游戏线程
		AsyncTask(ENamedThreads::GameThread, [this, bSuccess]()
		{
			OnEncryptedSaveCompleted.Broadcast(bSuccess, bSuccess ? TEXT("加密保存成功") : TEXT("加密保存失败"));
		});
	});
}

void USaveEncryptionSubsystem::AsyncLoadGameEncrypted(int32 SlotIndex, const FString& CustomKey)
{
	// 在后台线程执行加载
	AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [this, SlotIndex, CustomKey]()
	{
		UNiuniuSaveGame* LoadedGame = LoadGameEncrypted(SlotIndex, CustomKey);
		bool bSuccess = (LoadedGame != nullptr);
		
		// 回调到游戏线程
		AsyncTask(ENamedThreads::GameThread, [this, bSuccess]()
		{
			OnEncryptedLoadCompleted.Broadcast(bSuccess, bSuccess ? TEXT("加密加载成功") : TEXT("加密加载失败"));
		});
	});
}
