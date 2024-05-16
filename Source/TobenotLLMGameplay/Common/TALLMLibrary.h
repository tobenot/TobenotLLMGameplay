// Copyright (c) 2024 tobenot, See LICENSE in the project root for license information.

#pragma once

#include "CoreMinimal.h"
#include "OpenAIDefinitions.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Interfaces/IHttpRequest.h"
#include "TALLMLibrary.generated.h"

class FTAImageDownloadedDelegate;
struct FTAPrompt;
class UOpenAIChat;

UENUM(BlueprintType)
enum class ELLMChatEngineQuality : uint8
{
	Fast UMETA(DisplayName = "Fast"),
	Moderate UMETA(DisplayName = "Moderate"),
	HighQuality UMETA(DisplayName = "High Quality")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FTAImageDownloadedDelegate, UTexture2DDynamic*, Texture);
/**
 * 
 */
UCLASS()
class TOBENOTLLMGAMEPLAY_API UTALLMLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, Category = "Chat Engine")
	static EOAChatEngineType GetChatEngineTypeFromQuality(const ELLMChatEngineQuality Quality);
	
	static UOpenAIChat* SendMessageToOpenAIWithRetry(const FChatSettings& ChatSettings, TFunction<void(const FChatCompletion& Message, const FString& ErrorMessage,  bool Success)> Callback, const UObject* LogObject, const int32 NewRetryCount = MaxRetryCount);
	
	static UOpenAIChat* DownloadImageFromPollinations(const FString& ImagePrompt, const FTAImageDownloadedDelegate & OnDownloadComplete, const FTAImageDownloadedDelegate & OnDownloadFailed, const UObject* LogObject);

	static TSharedRef<IHttpRequest> DownloadImageFromPollinationsPure(const FString& PureDescription, const FTAImageDownloadedDelegate & OnDownloadComplete, const FTAImageDownloadedDelegate & OnDownloadFailed, const UObject* LogObject);
	
	UFUNCTION(BlueprintCallable, Category = "Prompt")
	static FString PromptToStr(const FTAPrompt& Prompt);

	UFUNCTION(BlueprintCallable, Category = "Token Accounting")
	static void GetAccumulatedTokenCost(int32 &TotalTokenCount, float &AccumulatedCost);
	
	UFUNCTION(BlueprintCallable, Category = "Token Accounting")
	static void GetLogObjectTokenCosts(TArray<FString>& LogObjectNames, TArray<int32>& TotalTokenCounts, TArray<float>& AccumulatedCosts);

private:
	// 最大重试次数和重试间隔定义
	static constexpr int32 MaxRetryCount = 3;
	static constexpr float RetryDelay = 3.0f;
	inline static int32 TotalTokens = 0;
	inline static float TotalCost = 0.0f;
	
	// 定义结构体用于存储每个LogObject的token统计和花费
	struct FTokenCostStats
	{
		int32 TotalTokens;
		float TotalCost;

		FTokenCostStats() : TotalTokens(0), TotalCost(0.0f) {}
	};
	
	static TMap<FString, FTokenCostStats> LogObjectTokenCosts;

private:
	/** Handles image requests coming from the web */
	static void HandleImageRequest(FHttpRequestPtr HttpRequest, const FHttpResponsePtr&  HttpResponse, bool bSucceeded, const FTAImageDownloadedDelegate & OnDownloadComplete, const FTAImageDownloadedDelegate & OnDownloadFailed);
};
