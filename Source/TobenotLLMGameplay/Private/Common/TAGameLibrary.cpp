// Copyright (c) 2024 tobenot, See LICENSE in the project root for license information.


#include "Common/TAGameLibrary.h"

TSoftObjectPtr<UTexture2D> UTAGameLibrary::GetRandomTextureFromCategory(const UDataTable* DataTable, const FName& CategoryName)
{
	TArray<FCategoryTextureData*> MatchingTextures;
	FCategoryTextureData* TempRow = nullptr;

	if (!DataTable)
	{
		return nullptr;
	}

	for (auto& Row : DataTable->GetRowMap())
	{
		TempRow = (FCategoryTextureData*)Row.Value;

		if (!TempRow)
		{
			// 输出日志，指出Row.Value转换为FCategoryTextureData*失败
			//UE_LOG(LogTemp, Warning, TEXT("Row value is null after casting."));
			continue;
		}
    
		if (!TempRow->CategoryName.IsEqual(CategoryName))
		{
			// 输出日志，指出CategoryName不匹配
			//UE_LOG(LogTemp, Warning, TEXT("CategoryName does not match."));
			continue;
		}

		// 软指针的 is valid 是加载好了 重载的bool也是这个意思
		if (!TempRow->Texture.ToSoftObjectPath().IsValid())
		{
			// 输出日志，指出Texture无效
			//UE_LOG(LogTemp, Warning, TEXT("Texture is not valid."));
			continue;
		}

		// 添加匹配纹理
		MatchingTextures.Add(TempRow);
	}

	if (MatchingTextures.Num() > 0)
	{
		// Randomly select an index
		int32 RandomIndex = FMath::RandRange(0, MatchingTextures.Num() - 1);
		return MatchingTextures[RandomIndex]->Texture;
	}

	// Return a null pointer if there is no match or list is empty
	return nullptr;
}
