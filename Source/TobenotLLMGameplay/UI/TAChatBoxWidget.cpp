// Copyright (c) 2024 tobenot, See LICENSE in the project root for license information.


#include "TAChatBoxWidget.h"
#include "TAChatMessageItemWidget.h"

UTAChatBoxWidget::UTAChatBoxWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// 初始化代码可以放这里（如果需要的话）
}

void UTAChatBoxWidget::NativeConstruct()
{
	// 建议调用基类构造函数
	Super::NativeConstruct();

	// 构造方法的其他UI初始化代码可以放这里
}

void UTAChatBoxWidget::AddChatMessage(const FString& Message, AActor* Sender, const FString& DynamicParam)
{
	if(!ChatMessageItemWidgetClass)
	{
		UE_LOG(LogTemp, Error, TEXT("UTAChatBoxWidget ChatMessageItemWidgetClass is not set."));
		return;
	}
    
	if(ChatScrollBox)
	{
		UTAChatMessageItemWidget* NewMessageItem = CreateWidget<UTAChatMessageItemWidget>(this, ChatMessageItemWidgetClass);
		if(NewMessageItem)
		{
			NewMessageItem->SetupMessage(Message, Sender, DynamicParam);
			ChatScrollBox->AddChild(NewMessageItem);
		}
	}
}