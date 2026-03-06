// Copyright Epic Games, Inc. All Rights Reserved.

#include "SaveEncryptionUtil.h"

TArray<uint8> FSaveEncryptionUtil::EncryptData(const TArray<uint8>& Data, const FString& Key)
{
	TArray<uint8> Result = Data;
	XorEncryptDecrypt(Result, Key.IsEmpty() ? GetDefaultEncryptionKey() : Key);
	return Result;
}

TArray<uint8> FSaveEncryptionUtil::DecryptData(const TArray<uint8>& EncryptedData, const FString& Key)
{
	// XOR加密和解密使用相同算法
	return EncryptData(EncryptedData, Key);
}

FString FSaveEncryptionUtil::GetDefaultEncryptionKey()
{
	// 这里可以从配置文件或环境变量读取
	// 硬编码一个默认密钥（实际项目中应该使用更安全的方式）
	return TEXT("NiuniuGodOfWealth2026");
}

void FSaveEncryptionUtil::XorEncryptDecrypt(TArray<uint8>& Data, const FString& Key)
{
	if (Data.IsEmpty() || Key.IsEmpty())
	{
		return;
	}

	const TArray<uint8> KeyBytes = StringToKeyBytes(Key);
	const int32 KeyLength = KeyBytes.Num();
	
	if (KeyLength == 0)
	{
		return;
	}

	// XOR加密：每个字节与密钥循环异或
	for (int32 i = 0; i < Data.Num(); ++i)
	{
		Data[i] ^= KeyBytes[i % KeyLength];
	}
}

TArray<uint8> FSaveEncryptionUtil::StringToKeyBytes(const FString& Key)
{
	TArray<uint8> Result;
	const FTCHARToUTF8 Converter(*Key);
	Result.Append((uint8*)Converter.Get(), Converter.Length());
	return Result;
}
