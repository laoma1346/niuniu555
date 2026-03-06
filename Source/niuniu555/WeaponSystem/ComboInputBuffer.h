// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ComboTypes.h"
#include "ComboInputBuffer.generated.h"

// 武器输入缓冲项
USTRUCT()
struct FWeaponBufferedInput
{
    GENERATED_BODY()

    UPROPERTY()
    EWeaponComboInputType WeaponInputType;

    UPROPERTY()
    float InputTime;

    UPROPERTY()
    float ExpireTime;

    FWeaponBufferedInput()
        : WeaponInputType(EWeaponComboInputType::None)
        , InputTime(0.0f)
        , ExpireTime(0.0f)
    {}
};

// 缓冲输入处理委托
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnBufferedInputProcessed, EWeaponComboInputType, InputType);

/**
 * 连招输入缓冲系统
 * 管理输入缓冲，实现预输入功能
 */
UCLASS()
class NIUNIU555_API UComboInputBuffer : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

public:
    // ========== 缓冲配置 ==========

    // 设置缓冲窗口时间（秒）
    UFUNCTION(BlueprintCallable, Category = "输入缓冲")
    void SetBufferWindowTime(float WindowTime);

    // 获取缓冲窗口时间
    UFUNCTION(BlueprintPure, Category = "输入缓冲")
    float GetBufferWindowTime() const { return BufferWindowTime; }

    // 设置最大缓冲数量
    UFUNCTION(BlueprintCallable, Category = "输入缓冲")
    void SetMaxBufferSize(int32 MaxSize);

    // ========== 输入处理 ==========

    // 添加输入到缓冲
    UFUNCTION(BlueprintCallable, Category = "输入缓冲")
    void BufferInput(EWeaponComboInputType InputType);

    // 尝试处理缓冲的输入
    UFUNCTION(BlueprintCallable, Category = "输入缓冲")
    bool TryProcessBufferedInput(FOnBufferedInputProcessed ProcessCallback);

    // 清空缓冲
    UFUNCTION(BlueprintCallable, Category = "输入缓冲")
    void ClearBuffer();

    // ========== 状态查询 ==========

    // 是否有缓冲的输入
    UFUNCTION(BlueprintPure, Category = "输入缓冲")
    bool HasBufferedInput() const;

    // 获取下一个缓冲的输入类型
    UFUNCTION(BlueprintPure, Category = "输入缓冲")
    EWeaponComboInputType PeekNextInput() const;

    // 获取缓冲数量
    UFUNCTION(BlueprintPure, Category = "输入缓冲")
    int32 GetBufferCount() const;

    // ========== Tick更新 ==========

    // 更新缓冲（检查过期）
    UFUNCTION(BlueprintCallable, Category = "输入缓冲")
    void Update(float DeltaTime);

protected:
    // 缓冲窗口时间（秒）
    UPROPERTY()
    float BufferWindowTime;

    // 最大缓冲数量
    UPROPERTY()
    int32 MaxBufferSize;

    // 输入缓冲队列
    UPROPERTY()
    TArray<FWeaponBufferedInput> WeaponInputBuffer;

    // 当前世界时间
    UPROPERTY()
    float CurrentWorldTime;

protected:
    // 移除过期输入
    UFUNCTION()
    void RemoveExpiredInputs();

    // 添加输入到队列
    UFUNCTION()
    void AddInputToBuffer(EWeaponComboInputType InputType);
};
