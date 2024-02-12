// Copyright (c) 2024 tobenot, See LICENSE in the project root for license information.


#include "TALLMLibrary.h"

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