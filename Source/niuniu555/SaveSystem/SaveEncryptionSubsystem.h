// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SaveEncryptionSubsystem.generated.h"

class UNiuniuSaveGame;

// 加密存档操作结果委托
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEncryptedSaveOperationCompleted, bool, bSuccess, const FString&, Message);

/**
 * 加密存档子系统 - 支持加密/解密的存档管理
 * 继承自GameInstanceSubsystem，提供安全的存档操作
 */
UCLASS()
class NIUNIU555_API USaveEncryptionSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// ========== 加密存档操作 ==========

	// 使用加密保存存档（同步）
	UFUNCTION(BlueprintCallable, Category = "SaveSystem|Encryption")
	bool SaveGameEncrypted(int32 SlotIndex, UNiuniuSaveGame* SaveGameObject, const FString& CustomKey = TEXT(""));

	// 使用加密加载存档（同步）
	UFUNCTION(BlueprintCallable, Category = "SaveSystem|Encryption")
	UNiuniuSaveGame* LoadGameEncrypted(int32 SlotIndex, const FString& CustomKey = TEXT(""));

	// 使用加密保存存档（异步）
	UFUNCTION(BlueprintCallable, Category = "SaveSystem|Encryption")
	void AsyncSaveGameEncrypted(int32 SlotIndex, UNiuniuSaveGame* SaveGameObject, const FString& CustomKey = TEXT(""));

	// 使用加密加载存档（异步）
	UFUNCTION(BlueprintCallable, Category = "SaveSystem|Encryption")
	void AsyncLoadGameEncrypted(int32 SlotIndex, const FString& CustomKey = TEXT(""));

	// ========== 配置 ==========

	// 是否启用加密（可在配置中设置）
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "SaveSystem|Encryption")
	bool bEnableEncryption = true;

	// 自定义加密密钥（为空则使用默认密钥）
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "SaveSystem|Encryption")
	FString CustomEncryptionKey;

	// ========== 事件委托 ==========

	UPROPERTY(BlueprintAssignable, Category = "SaveSystem|Encryption|Events")
	FOnEncryptedSaveOperationCompleted OnEncryptedSaveCompleted;

	UPROPERTY(BlueprintAssignable, Category = "SaveSystem|Encryption|Events")
	FOnEncryptedSaveOperationCompleted OnEncryptedLoadCompleted;

protected:
	// 存档文件后缀
	static const FString EncryptedSaveExtension;

	// 获取带加密的存档路径
	FString GetEncryptedSavePath(int32 SlotIndex) const;

	// 异步操作完成回调
	void OnAsyncSaveCompleted(bool bSuccess, const FString& Message);
	void OnAsyncLoadCompleted(bool bSuccess, const FString& Message);
};
