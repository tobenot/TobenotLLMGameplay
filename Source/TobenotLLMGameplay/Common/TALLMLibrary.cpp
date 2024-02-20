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

UOpenAIChat* UTALLMLibrary::SendMessageToOpenAIWithRetry(const FChatSettings& ChatSettings, TFunction<void(const FChatCompletion& Message, const FString& ErrorMessage,  bool Success)> Callback)
{
	// 打印ChatSettings的调试信息
	FStringBuilderBase StringBuilder;
	StringBuilder.Append(TEXT("Sending chat messages:\n"));
	for (const FChatLog& ChatEntry : ChatSettings.messages)
	{
		FString RoleName = UEnum::GetValueAsString(ChatEntry.role);
		StringBuilder.Append(FString::Printf(TEXT("Role: %s, Content: %s\n"), *RoleName, *ChatEntry.content));
	}
	UE_LOG(LogTemp, Log, TEXT("%s"), StringBuilder.ToString());

	// 调用OpenAIChat进行通信
	return UOpenAIChat::Chat(ChatSettings, [Callback](const FChatCompletion& Message, const FString& ErrorMessage, bool Success)
	{
		if (Success)
		{
			UE_LOG(LogTemp, Log, TEXT("Reponse success: %s"), *Message.message.content);
		}else
		{
			UE_LOG(LogTemp, Log, TEXT("Reponse failed: %s"), *ErrorMessage);
		}
		Callback(Message,ErrorMessage,Success);
	});
}


FString UTALLMLibrary::PromptToStr(const FTAPrompt& Prompt)
{
	return FString::Printf(TEXT("Prompt:[%s];%s%s"),
				*Prompt.PromptTemplate,
				!Prompt.PromptExample.IsEmpty() ? *FString::Printf(TEXT("Example:[%s];"), *Prompt.PromptExample) : TEXT(""),
				Prompt.bUseJsonFormat ? TEXT("reply in json format;") : TEXT(""));
}

UOpenAIChat* UTALLMLibrary::DownloadImageFromPollinations(const FString& ImagePrompt, const FTAImageDownloadedDelegate & OnDownloadComplete, const FTAImageDownloadedDelegate & OnDownloadFailed)
{
	TArray<FChatLog> TempMessagesList;
	const FString SystemPrompt =
		"You need to design a scene \"description\" for the text description I give,"
		"which is a vast scene description of the text."
		"Remember, only describe the vast scenery, DO NOT describe any characters or creatures or monster."
		"Follow the ensuing JSON template: "
		"{"
			"\"description\": \"description within 20 words or less, exp:A%20[place]%20creating%20an%20[adj]%20atmosphere.\""
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
	});
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

