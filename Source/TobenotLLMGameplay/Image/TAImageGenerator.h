// Copyright (c) 2024 tobenot, See LICENSE in the project root for license information.

#pragma once

// UTAImageGenerator.h
// 图片生成器类，用于处理图片的异步生成请求

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Event/Data/TAEventInfo.h"
#include "Common/TALLMLibrary.h"
#include "TAImageGenerator.generated.h"

class UTAChatCallback;

UCLASS()
class TOBENOTLLMGAMEPLAY_API UTAImageGenerator : public UObject
{
	GENERATED_BODY()
	
	UTAImageGenerator();
public:
	// 绑定的事件信息，用于图片生成回调时的映射，到时候要找回这个事件去设置。
	int32 BoundEventID;

	// 请求生成图片
	UFUNCTION(BlueprintCallable, Category = "Image|Generation")
	void RequestGenerateImage(const FTAEventInfo& EventInfo);

	UFUNCTION()
	void OnDownloadComplete(UTexture2DDynamic* Texture);
	UFUNCTION()
	void OnDownloadFailed(UTexture2DDynamic* Texture);
	
private:
	FTAImageDownloadedDelegate OnDownloadCompleteDelegate;
	FTAImageDownloadedDelegate OnDownloadFailedDelegate;

	UPROPERTY()
	UOpenAIChat* CacheOpenAIChat;
};