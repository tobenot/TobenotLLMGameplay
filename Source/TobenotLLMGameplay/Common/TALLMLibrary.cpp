// Copyright (c) 2024 tobenot, See LICENSE in the project root for license information.


#include "TALLMLibrary.h"

#include "OpenAIChat.h"

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
	// 调用OpenAIChat进行通信
	return UOpenAIChat::Chat(ChatSettings, Callback);
}
