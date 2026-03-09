#include "EquipmentTypes.h"

void FEquipmentID::GenerateUniqueID()
{
	// 使用时间戳和随机数生成唯一ID
	FString Timestamp = FString::FromInt(FDateTime::Now().ToUnixTimestamp());
	FString RandomStr = FString::FromInt(FMath::Rand());
	UniqueID = BaseID + "_" + Timestamp + "_" + RandomStr;
}
