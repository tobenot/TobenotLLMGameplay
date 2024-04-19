// Copyright (c) 2024 tobenot, See LICENSE in the project root for license information.


#include "TALLMLibrary.h"
#include "Common/TAPromptDefinitions.h"
#include "OpenAIChat.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "GenericPlatform/GenericPlatformHttp.h"
#include "Modules/ModuleManager.h"
#include "Engine/Texture2D.h"
#include "Engine/Texture2DDynamic.h"
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "HttpModule.h"
#include "TextureResource.h"
#include "RenderingThread.h"
#include "TobenotToolkit/Debug/CategoryLogSubsystem.h"

EOAChatEngineType UTALLMLibrary::GetChatEngineTypeFromQuality(const ELLMChatEngineQuality Quality)
{
	switch (Quality)
	{
	case ELLMChatEngineQuality::Fast:
		return EOAChatEngineType::GPT_3_5_TURBO;
	case ELLMChatEngineQuality::Moderate:
		return EOAChatEngineType::GPT_4_TURBO;
	case ELLMChatEngineQuality::HighQuality:
		return EOAChatEngineType::GPT_4;
	default:
		return EOAChatEngineType::GPT_3_5_TURBO; // Default to a reasonable type
	}
}

UOpenAIChat* UTALLMLibrary::SendMessageToOpenAIWithRetry(const FChatSettings& ChatSettings, TFunction<void(const FChatCompletion& Message, const FString& ErrorMessage, bool Success)> Callback, const UObject* LogObject, const int32 NewRetryCount)
{
    // 调用OpenAIChat进行通信，并定义重试逻辑
    UOpenAIChat* Chat = UOpenAIChat::Chat(ChatSettings, [Callback, NewRetryCount, LogObject, ChatSettings, &Chat](const FChatCompletion& Message, const FString& ErrorMessage, bool Success)
    {
        if (Success)
        {
            // 处理成功的响应
            UE_LOG(LogTemp, Log, TEXT("Response success: %s"), *Message.message.content);
        	if(LogObject)
        	{
				if (UCategoryLogSubsystem* CategoryLogSubsystem = LogObject->GetWorld()->GetSubsystem<UCategoryLogSubsystem>())
				{
					const FString ResponseStr = FString::Printf(TEXT("[%s] Assistant Response:\n%s\n"), *LogObject->GetName(),*Message.message.content);
					CategoryLogSubsystem->WriteLog(TEXT("Chat"), *ResponseStr);
				}
        		Callback(Message, ErrorMessage, true);
			}
			Chat = nullptr;
        }
        else
        {
        	// 是否还能重试
            if (NewRetryCount > 0)
            {
                UE_LOG(LogTemp, Warning, TEXT("Response failed: %s. Retrying..."), *ErrorMessage);

                // 设置重试延时调用
                FTimerHandle RetryTimerHandle;
                // 获取World上下文
                UWorld* World = GEngine->GetWorldFromContextObject(LogObject, EGetWorldErrorMode::LogAndReturnNull); // 或者用其他方式获取World上下文
                if (World)
                {
                	World->GetTimerManager().SetTimer(RetryTimerHandle, [NewRetryCount, LogObject, ChatSettings, Callback]()
					{
						// 重新发送请求，传递新的重试次数
						SendMessageToOpenAIWithRetry(ChatSettings, Callback, LogObject, NewRetryCount - 1);
					}, RetryDelay, false);
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
						CategoryLogSubsystem->WriteLog(TEXT("Chat"), *ResponseStr);
					}
            		Callback(Message, ErrorMessage, false);
				}
            	Chat = nullptr;
            }
        }
    });

	// 打印ChatSettings的调试信息
	FStringBuilderBase StringBuilder;
	StringBuilder.Append(TEXT("Sending chat messages:\n"));
	for (const FChatLog& ChatEntry : ChatSettings.messages)
	{
#if UE_BUILD_SHIPPING
		if(ChatEntry.role == EOAChatRole::SYSTEM)
		{
			continue;
		}
#endif
		FString RoleName = UEnum::GetValueAsString(ChatEntry.role);
		StringBuilder.Append(FString::Printf(TEXT("Role: %s, Content: %s\n"), *RoleName, *ChatEntry.content));
	}
	const FString LogContent = StringBuilder.ToString();
	UE_LOG(LogTemp, Log, TEXT("%s"), *LogContent);

	if(LogObject)
	{
		UE_LOG(LogTemp, Log, TEXT("[%s] Send Chat"), *LogObject->GetName());
		if (UCategoryLogSubsystem* CategoryLogSubsystem = LogObject->GetWorld()->GetSubsystem<UCategoryLogSubsystem>())
		{
			const FString LogStr = FString::Printf(TEXT("[%s] %s"), *LogObject->GetName(),*LogContent);
			CategoryLogSubsystem->WriteLog(TEXT("Chat"), LogStr);
		}
	}

    // 返回Chat实例
    return Chat;
}


FString UTALLMLibrary::PromptToStr(const FTAPrompt& Prompt)
{
	return FString::Printf(TEXT("Prompt:[%s];%s%s"),
				*Prompt.PromptTemplate,
				!Prompt.PromptExample.IsEmpty() ? *FString::Printf(TEXT("Example:%s;"), *Prompt.PromptExample) : TEXT(""),
				Prompt.bUseJsonFormat ? TEXT("reply in json format;") : TEXT(""));
}

UOpenAIChat* UTALLMLibrary::DownloadImageFromPollinations(const FString& ImagePrompt, const FTAImageDownloadedDelegate & OnDownloadComplete, const FTAImageDownloadedDelegate & OnDownloadFailed, const UObject* LogObject)
{
	TArray<FChatLog> TempMessagesList;
	const FString SystemPrompt =
		"You need to design a scene \"description\" for the text description I give,"
		"which is a vast scene description of the text."
		"Remember, only describe the vast scenery, DO NOT describe any characters or creatures or monster."
		"Follow the ensuing JSON template: "
		"{"
			"\"description\": \"description within 20 words or less, exp:A [place] creating an [adj] atmosphere.\""
		"}";
	TempMessagesList.Add({EOAChatRole::SYSTEM, SystemPrompt});
	TempMessagesList.Add({EOAChatRole::USER, "text description:"+ImagePrompt});
	FChatSettings ChatSettings{
		UTALLMLibrary::GetChatEngineTypeFromQuality(ELLMChatEngineQuality::Fast),
		TempMessagesList
	};
	ChatSettings.jsonFormat = true;
	
	// 异步发送消息
	return UTALLMLibrary::SendMessageToOpenAIWithRetry(ChatSettings, [OnDownloadFailed,OnDownloadComplete,ImagePrompt](const FChatCompletion& Message, const FString& ErrorMessage, bool Success)
	{
		if (Success)
		{
			TSharedPtr<FJsonObject> JsonObject;
			const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Message.message.content);

			if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
			{
				FString Description;
				if (JsonObject->TryGetStringField(TEXT("description"), Description))
				{
					// 全小写，因为网址里面大小写是不区分的
					Description.ToLowerInline();
					
					// 简易的替换屏蔽词，它们生成出来的图片不好看
					Description.ReplaceInline(TEXT("mushroom"), TEXT("*"));
					Description.ReplaceInline(TEXT("monster"), TEXT("*"));
					Description.ReplaceInline(TEXT("fungi"), TEXT("*"));
					Description.ReplaceInline(TEXT("dance"), TEXT("*"));
					Description.ReplaceInline(TEXT("battle"), TEXT("*"));
					Description.ReplaceInline(TEXT("eye"), TEXT("*"));
					
					// URL编码图片提示词
					FString EncodedPrompt = FGenericPlatformHttp::UrlEncode(Description);
		    
					// 构建Pollinations的图片请求URL
					FString RequestUrl = FString::Printf(TEXT("https://image.pollinations.ai/prompt/%s"), *EncodedPrompt);
					
					UE_LOG(LogTemp, Log, TEXT("RequestUrl: %s"), *RequestUrl);
					
					// 创建HTTP请求实例
					TSharedRef<IHttpRequest> HttpRequest = FHttpModule::Get().CreateRequest();
					HttpRequest->OnProcessRequestComplete().BindLambda([OnDownloadComplete, OnDownloadFailed](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
					{
						HandleImageRequest(Request,Response,bWasSuccessful,OnDownloadComplete,OnDownloadFailed);
					});
					HttpRequest->SetURL(RequestUrl);
					HttpRequest->SetVerb("GET");
					HttpRequest->ProcessRequest();
				}else
				{
					OnDownloadFailed.Broadcast(nullptr);
					UE_LOG(LogTemp, Error, TEXT("JSON parse 'description' failed. Content: %s"), *Message.message.content);
				}
			}else
			{
				OnDownloadFailed.Broadcast(nullptr);
				UE_LOG(LogTemp, Error, TEXT("JSON parse failed. Content: %s"), *Message.message.content);
			}
		}
		else
		{
			OnDownloadFailed.Broadcast(nullptr);
			UE_LOG(LogTemp, Error, TEXT("Description generation failed. ImagePrompt: %s"), *ImagePrompt);
		}
	}, LogObject);
}

void UTALLMLibrary::HandleImageRequest(FHttpRequestPtr HttpRequest, const FHttpResponsePtr& HttpResponse, bool bSucceeded, const FTAImageDownloadedDelegate & OnDownloadComplete, const FTAImageDownloadedDelegate & OnDownloadFailed)
{
	if ( bSucceeded && HttpResponse.IsValid() && HttpResponse->GetContentLength() > 0 && HttpResponse->GetContent().Num() > 0 )
	{
		IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName("ImageWrapper"));
		TSharedPtr<IImageWrapper> ImageWrappers[3] =
		{
			ImageWrapperModule.CreateImageWrapper(EImageFormat::PNG),
			ImageWrapperModule.CreateImageWrapper(EImageFormat::JPEG),
			ImageWrapperModule.CreateImageWrapper(EImageFormat::BMP),
		};

		for ( auto ImageWrapper : ImageWrappers )
		{
			if ( ImageWrapper.IsValid() && ImageWrapper->SetCompressed(HttpResponse->GetContent().GetData(), HttpResponse->GetContent().Num()) )
			{
				TArray64<uint8> RawData;
				const ERGBFormat InFormat = ERGBFormat::BGRA;
				if ( ImageWrapper->GetRaw(InFormat, 8, RawData) )
				{
					if ( UTexture2DDynamic* Texture = UTexture2DDynamic::Create(ImageWrapper->GetWidth(), ImageWrapper->GetHeight()) )
					{
						Texture->SRGB = true;
						Texture->UpdateResource();

						FTexture2DDynamicResource* TextureResource = static_cast<FTexture2DDynamicResource*>(Texture->GetResource());
						if (TextureResource)
						{
							ENQUEUE_RENDER_COMMAND(FWriteRawDataToTexture)(
								[TextureResource, RawData = MoveTemp(RawData)](FRHICommandListImmediate& RHICmdList)
								{
									TextureResource->WriteRawToTexture_RenderThread(RawData);
								});
						}
						OnDownloadComplete.Broadcast(Texture);
						return;
					}
				}
			}
		}
	}

	OnDownloadFailed.Broadcast(nullptr);
}

