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
	
	static UOpenAIChat* SendMessageToOpenAIWithRetry(const FChatSettings& ChatSettings, TFunction<void(const FChatCompletion& Message, const FString& ErrorMessage,  bool Success)> Callback, const UObject* LogObject = nullptr);
	
	static UOpenAIChat* DownloadImageFromPollinations(const FString& ImagePrompt, const FTAImageDownloadedDelegate & OnDownloadComplete, const FTAImageDownloadedDelegate & OnDownloadFailed);
	
	UFUNCTION(BlueprintCallable, Category = "Prompt")
	static FString PromptToStr(const FTAPrompt& Prompt);
private:
	static constexpr int32 MaxRetryCount = 3;
	static constexpr float RetryDelay = 2.0f;

private:

	/** Handles image requests coming from the web */
	static void HandleImageRequest(FHttpRequestPtr HttpRequest, const FHttpResponsePtr&  HttpResponse, bool bSucceeded, const FTAImageDownloadedDelegate & OnDownloadComplete, const FTAImageDownloadedDelegate & OnDownloadFailed);
};
