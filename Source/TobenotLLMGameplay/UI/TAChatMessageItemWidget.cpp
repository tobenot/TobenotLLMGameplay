// Copyright (c) 2024 tobenot, See LICENSE in the project root for license information.

#include "TAChatMessageItemWidget.h"

void UTAChatMessageItemWidget::SetupMessage(const FString& Message, AActor* Sender, const FString& DynamicParam)
{
	if(MessageTextBlock)
	{
		// 构建信息文本，并将它设置到文本块中
		FString DisplayText;
		if(Sender != nullptr)
		{
			DisplayText = FString::Printf(TEXT("%s: %s"), *Sender->GetName(), *Message);
		}else
		{
			DisplayText = *Message;
		}
		MessageTextBlock->SetText(FText::FromString(DisplayText));
	}
}