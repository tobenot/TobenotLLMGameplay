// Copyright (c) 2024 tobenot, See LICENSE in the project root for license information.

#pragma once

#include "CoreMinimal.h"
#include "OpenAIDefinitions.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "TAEmbeddingSystem.generated.h"

const int32 MaxRetryCount = 3;
class UOpenAIEmbedding;

UENUM(BlueprintType)
enum class ETagEmbeddingStatus : uint8
{
	NotEmbedded,    // 未词嵌
	Embedding,      // 正在词嵌
	Embedded        // 已经被词嵌
};

/**
 * 词嵌数据结构，存储标签及其对应的词嵌结果
 */
USTRUCT(BlueprintType)
struct FTagEmbeddingData
{
	GENERATED_BODY()

	// 标签，使用FName而不是FString以提高效率
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName Tag;

	// 词嵌向量
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FHighDimensionalVector EmbeddingVector = FHighDimensionalVector();

	// 标签的词嵌状态
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	ETagEmbeddingStatus Status = ETagEmbeddingStatus::NotEmbedded;
};

/**
 * 词嵌系统基类，继承自游戏实例子系统
 */
UCLASS()
class UTAEmbeddingSystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// 请求一个Tag的词嵌
	// 调用它时，如果Tag还未嵌入完成，会返回false并开始嵌入过程
	bool GetTagEmbedding(const FName& Tag, FHighDimensionalVector& OutEmbeddingVec);
	
	// 请求词嵌的接口
	UOpenAIEmbedding* SendEmbeddingToOpenAIWithRetry(const FEmbeddingSettings& EmbeddingSettings, TFunction<void(const FEmbeddingResult& Message, const FString& ErrorMessage,  bool Success)> Callback, const UObject* LogObject, const int32 NewRetryCount = MaxRetryCount);
	
	// 检索一个标签的词嵌状态
	UFUNCTION(BlueprintCallable, Category = "Embedding")
	ETagEmbeddingStatus GetTagEmbeddingStatus(const FName& Tag) const;
	
	// 检索一个标签的词嵌数据
	UFUNCTION(BlueprintCallable, Category = "Embedding")
	FTagEmbeddingData GetTagEmbeddingData(const FName& Tag) const;

	// 计算两个词嵌向量之间的余弦相似度
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Embedding")
	static float CalculateCosineSimilarity(const FHighDimensionalVector& VectorA, const FHighDimensionalVector& VectorB);

protected:

	// 存储所有标签及其词嵌数据，使用FName作为键
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TMap<FName, FTagEmbeddingData> EmbeddingsCache;

private:
	bool bHasTagEmbedding = false;
};
