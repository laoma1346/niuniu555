// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

/**
 * 存档加密工具类 - 提供简单的XOR加密/解密功能
 * 注意：这只是基础加密，防止普通玩家直接修改存档
 * 如需更高级的安全保护，建议使用AES或服务器验证
 */
class NIUNIU555_API FSaveEncryptionUtil
{
public:
	/**
	 * 加密字节数组
	 * @param Data 原始数据
	 * @param Key 加密密钥（如果为空则使用默认密钥）
	 * @return 加密后的数据
	 */
	static TArray<uint8> EncryptData(const TArray<uint8>& Data, const FString& Key = TEXT(""));

	/**
	 * 解密字节数组
	 * @param EncryptedData 加密数据
	 * @param Key 解密密钥（如果为空则使用默认密钥）
	 * @return 解密后的数据
	 */
	static TArray<uint8> DecryptData(const TArray<uint8>& EncryptedData, const FString& Key = TEXT(""));

	/**
	 * 获取默认加密密钥
	 * 可以从配置文件读取，这里硬编码作为示例
	 */
	static FString GetDefaultEncryptionKey();

private:
	// XOR加密核心算法（加密和解密使用相同操作）
	static void XorEncryptDecrypt(TArray<uint8>& Data, const FString& Key);
	
	// 将字符串转换为字节数组作为密钥
	static TArray<uint8> StringToKeyBytes(const FString& Key);
};
