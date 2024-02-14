// Copyright (c) 2024 tobenot, See LICENSE in the project root for license information.

#include "TAChatWidget.h"

UTAChatWidget::UTAChatWidget(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
}

void UTAChatWidget::NativeConstruct() 
{
	Super::NativeConstruct();
	if (ChatTextBox)
	{
		ChatTextBox->OnTextCommitted.AddDynamic(this, &UTAChatWidget::OnChatTextCommitted);
	}
}

void UTAChatWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
}

FString UTAChatWidget::GetMessage()
{
	return ChatTextBox->GetText().ToString();
}

void UTAChatWidget::OnChatTextCommitted(const FText& Text, ETextCommit::Type CommitMethod)
{
	if(CommitMethod == ETextCommit::OnEnter)
	{
		// ...
		
		ChatTextBox->SetText(FText::FromString(""));
	}
}