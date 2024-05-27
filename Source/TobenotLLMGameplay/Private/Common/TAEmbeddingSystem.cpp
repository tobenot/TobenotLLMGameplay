// Copyright (c) 2024 tobenot, See LICENSE in the project root for license information.

#include "Common/TAEmbeddingSystem.h"

#include "OpenAIEmbedding.h"
#include "OpenAIUtils.h"
#include "Serialization/ArrayReader.h"
#include "Serialization/BufferArchive.h"
#include "TobenotToolkit/Debug/CategoryLogSubsystem.h"


const float RetryDelaySeconds = 5.0f;

bool UTAEmbeddingSystem::GetTagEmbedding(const FName& Tag, FHighDimensionalVector& OutEmbeddingVec)
{
	// 检查嵌入缓存是否已经有我们的Tag
	if (EmbeddingsCache.Contains(Tag))
	{
		if (EmbeddingsCache[Tag].Status == ETagEmbeddingStatus::Embedded)
		{
			// 如果Tag已经被嵌入，我们直接从缓存中取出嵌入的向量返回
			OutEmbeddingVec = EmbeddingsCache[Tag].EmbeddingVector;
			return true;
		}
		// 如果Tag正在嵌入过程中，那么我们立刻返回false
		else if (EmbeddingsCache[Tag].Status == ETagEmbeddingStatus::Embedding)
		{
			return false;
		}
	}
	else 
	{
		// 如果Tag还未开始嵌入
		if(LoadEmbeddingFromArchive(Tag,TEXT("TEXT_EMBEDDING_3_LARGE"), OutEmbeddingVec))
		{
			// 如果本地存档中有结果，直接返回true，表示成功获取到了词嵌结果
			return true;
		}
		
		// 看看有没有Tag在嵌入，因为Http并发限制，我们一次就嵌一个Tag就好
		if(bHasTagEmbedding)
		{
			return false;
		}
		bHasTagEmbedding = true;
		
		// 如果没有Tag正在嵌入我们需要将新的Tag加入到嵌入缓存中，启动嵌入请求，并立刻返回false
		FTagEmbeddingData NewEmbeddingData;
		NewEmbeddingData.Tag = Tag;
		NewEmbeddingData.Status = ETagEmbeddingStatus::Embedding;

		EmbeddingsCache.Add(Tag, NewEmbeddingData);
		FEmbeddingSettings EmbeddingSettings;
		EmbeddingSettings.model = EEmbeddingEngineType::TEXT_EMBEDDING_3_LARGE;
		EmbeddingSettings.input = Tag.ToString(); // 假设FEmbeddingSettings有一个tag字段用来标识请求
		
		SendEmbeddingToOpenAIWithRetry(EmbeddingSettings, [this, Tag](const FEmbeddingResult& Result, const FString& ErrorMessage, bool Success)
		{
			if (Success)
			{
				UE_LOG(LogTemp, Log, TEXT("[%s] SendEmbeddingToOpenAIWithRetry Success"), *Tag.ToString());

				// 更新状态和EmbeddingVector
				if (EmbeddingsCache.Contains(Tag))
				{
					FTagEmbeddingData& EmbeddingData = EmbeddingsCache[Tag];
					EmbeddingData.Status = ETagEmbeddingStatus::Embedded;
					EmbeddingData.EmbeddingVector = Result.embeddingVector;
					SaveEmbeddingToArchive(Tag, Result.embeddingVector, TEXT("TEXT_EMBEDDING_3_LARGE"));
				}
			}
			else
			{
				UE_LOG(LogTemp, Log, TEXT("[%s] SendEmbeddingToOpenAIWithRetry Fail: %s"), *Tag.ToString(), *ErrorMessage);

				// 如果失败，将状态更新为NotEmbedded以便之后重试
				if (EmbeddingsCache.Contains(Tag))
				{
					EmbeddingsCache[Tag].Status = ETagEmbeddingStatus::NotEmbedded;
				}
			}
			bHasTagEmbedding = false;
		}, this);

		return false;
	}
	return false;
}

UOpenAIEmbedding* UTAEmbeddingSystem::SendEmbeddingToOpenAIWithRetry(const FEmbeddingSettings& EmbeddingSettings,
	TFunction<void(const FEmbeddingResult& Message, const FString& ErrorMessage, bool Success)> Callback,
	const UObject* LogObject, const int32 NewRetryCount)
{
// 调用OpenAIEmbedding进行通信，并定义重试逻辑
    UOpenAIEmbedding* Embedding = UOpenAIEmbedding::Embedding(EmbeddingSettings, [this, Callback, NewRetryCount, LogObject, EmbeddingSettings /*, Embedding 这个赋值是在绑定之后，传进来就是个空指针*/]
    	(const FEmbeddingResult& Message, const FString& ErrorMessage, bool Success)
    {
        if (Success)
        {
            // 处理成功的响应
        	if(LogObject && LogObject->IsValidLowLevel())
        	{
        		UE_LOG(LogTemp, Log, TEXT("[%s] Embedding success [%s]"), *LogObject->GetName(),*EmbeddingSettings.input);
				if (UCategoryLogSubsystem* CategoryLogSubsystem = LogObject->GetWorld()->GetSubsystem<UCategoryLogSubsystem>())
				{
					const FString ResponseStr = FString::Printf(TEXT("[%s] Embedding success:\n%s\n"), *LogObject->GetName(), *EmbeddingSettings.input);
					CategoryLogSubsystem->WriteLog(TEXT("Embedding"), *ResponseStr);
				}
        		Callback(Message, ErrorMessage, true);
			}else
			{
				UE_LOG(LogTemp, Log, TEXT("[NULL] Embedding success [%s]"),*EmbeddingSettings.input);
			}
			//Embedding = nullptr;
        }
        else if(ErrorMessage == "Request cancelled")
        {
        	UE_LOG(LogTemp, Log, TEXT("[%s] Response cancelled"), *LogObject->GetName());
        }
    	else
        {
        	// 是否还能重试
            if (NewRetryCount > 0)
            {
                UE_LOG(LogTemp, Warning, TEXT("Response failed: %s. Retrying..."), *ErrorMessage);

                // 设置重试延时调用
                FTimerHandle RetryTimerHandle;
                if(LogObject)
                {
                	UWorld* World = GEngine->GetWorldFromContextObject(LogObject, EGetWorldErrorMode::LogAndReturnNull); // 或者用其他方式获取World上下文
					if (World)
					{
						World->GetTimerManager().SetTimer(RetryTimerHandle, [this, NewRetryCount, LogObject, EmbeddingSettings, Callback]()
						{
							// 重新发送请求，传递新的重试次数
							SendEmbeddingToOpenAIWithRetry(EmbeddingSettings, Callback, LogObject, NewRetryCount - 1);
						}, RetryDelaySeconds, false);
					}
                }
            }
            else
            {
                // 如果重试次数已用尽，执行最初提供的失败回调函数
                UE_LOG(LogTemp, Error, TEXT("Exhausted all retries! Response failed after retries: %s"), *ErrorMessage);
            	if(LogObject)
            	{
					if (UCategoryLogSubsystem* CategoryLogSubsystem = LogObject->GetWorld()->GetSubsystem<UCategoryLogSubsystem>())
					{
						const FString ResponseStr = FString::Printf(TEXT("[%s] Assistant Response exhausted all retries!\n%s\n"), *LogObject->GetName(),*ErrorMessage);
						CategoryLogSubsystem->WriteLog(TEXT("Embedding"), *ResponseStr);
					}
            		Callback(Message, ErrorMessage, false);
				}
            	//Embedding = nullptr;
            }
        }
    	//Embedding->RemoveFromRoot();
    });
	//Embedding->AddToRoot();

	
	// 打印EmbeddingSettings的调试信息
	if(LogObject)
	{
		UE_LOG(LogTemp, Log, TEXT("[%s] Send Embedding [%s]"), *LogObject->GetName(), *EmbeddingSettings.input);
	}

    // 返回Embedding实例
    return Embedding;
}

ETagEmbeddingStatus UTAEmbeddingSystem::GetTagEmbeddingStatus(const FName& Tag) const
{
	const FTagEmbeddingData* FoundTagEmbeddingData = EmbeddingsCache.Find(Tag);
	if (FoundTagEmbeddingData == nullptr)
	{
		return ETagEmbeddingStatus::NotEmbedded;
	}
	return FoundTagEmbeddingData->Status;
}

FTagEmbeddingData UTAEmbeddingSystem::GetTagEmbeddingData(const FName& Tag) const
{
	const FTagEmbeddingData* FoundTagEmbeddingData = EmbeddingsCache.Find(Tag);
	//检查找到的对象是否为nullptr，如果是则返回默认的FTagEmbeddingData
	if (FoundTagEmbeddingData == nullptr)
	{
		FTagEmbeddingData DefaultData;
		DefaultData.Tag = Tag;
		DefaultData.Status = ETagEmbeddingStatus::NotEmbedded;
		return DefaultData;
	}
	return *FoundTagEmbeddingData;
}

float UTAEmbeddingSystem::CalculateCosineSimilarity(const FHighDimensionalVector& VectorA,const FHighDimensionalVector& VectorB)
{
	return UOpenAIUtils::HDVectorCosineSimilaritySIMD(VectorA, VectorB);
}

FString UTAEmbeddingSystem::GetSaveFilePath(const FName& Tag, const FString& ModelName) const
{
	// 获取项目的Saved目录路径
	FString SavedDir = FPaths::ProjectSavedDir();

	// 使用标签和模型名称生成文件名，确保名称对文件系统是有效的
	FString FileName = FString::Printf(TEXT("Embeddings/%s_%s.embedding"), *Tag.ToString(), *ModelName);

	// 清洁文件名，移除可能的非法字符
	FPaths::MakeValidFileName(FileName);

	// 最后返回完整的文件路径
	return SavedDir / FileName;
}

bool UTAEmbeddingSystem::SaveEmbeddingToArchive(const FName& Tag, const FHighDimensionalVector& EmbeddingVector, const FString& ModelName)
{
	FEmbeddingArchive ArchiveData;
	ArchiveData.Tag = Tag;
	ArchiveData.EmbeddingVector = EmbeddingVector;
	ArchiveData.ModelName = ModelName;

	FString FilePath = GetSaveFilePath(Tag, ModelName); // 生成文件路径
	FBufferArchive ToBinary;
	ToBinary << ArchiveData; // 序列化数据到二进制

	// 将二进制数据写入文件
	if (FFileHelper::SaveArrayToFile(ToBinary, *FilePath))
	{
		ToBinary.FlushCache();
		ToBinary.Empty();
		return true;
	}
	return false;
}

bool UTAEmbeddingSystem::LoadEmbeddingFromArchive(const FName& Tag, const FString& ModelName, FHighDimensionalVector& OutEmbeddingVector)
{
	FString FilePath = GetSaveFilePath(Tag, ModelName); // 生成文件路径
	FArrayReader FromBinary;

	// 从文件读取二进制数据
	if (FFileHelper::LoadFileToArray(FromBinary, *FilePath))
	{
		FEmbeddingArchive ArchiveData;
		FromBinary << ArchiveData; // 从二进制反序列化数据
		
		FTagEmbeddingData NewEmbeddingData;
		NewEmbeddingData.Tag = Tag;
		NewEmbeddingData.Status = ETagEmbeddingStatus::Embedded;
		NewEmbeddingData.EmbeddingVector = ArchiveData.EmbeddingVector;
		EmbeddingsCache.Add(Tag, NewEmbeddingData);
		
		OutEmbeddingVector = ArchiveData.EmbeddingVector;
		FromBinary.FlushCache();
		FromBinary.Empty();
		return true;
	}
	return false;
}
