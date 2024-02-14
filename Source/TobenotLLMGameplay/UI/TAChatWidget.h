// Copyright (c) 2024 tobenot, See LICENSE in the project root for license information.

#pragma once

#include "CoreMinimal.h"
#include "TAChatBoxWidget.h"
#include "Blueprint/UserWidget.h"
#include "Components/EditableTextBox.h"
#include "TAChatWidget.generated.h"

UCLASS()
class TOBENOTLLMGAMEPLAY_API UTAChatWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UTAChatWidget(const FObjectInitializer& ObjectInitializer);

	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	
	UFUNCTION(BlueprintCallable, Category="TAChatWidget")
	FString GetMessage();
	
	UFUNCTION(BlueprintCallable, Category = "TAChatWidget")
	UEditableTextBox* GetChatTextBox()
	{
		return ChatTextBox;
	}
	
	UFUNCTION(BlueprintCallable, Category = "COWChatWidget")
	UTAChatBoxWidget* GetChatBoxWidget() const
	{
		return ChatBoxWidget;
	}

	UFUNCTION()
	virtual void OnChatTextCommitted(const FText& Text, ETextCommit::Type CommitMethod);
	
protected:
	UPROPERTY(Meta = (BindWidget))
	class UEditableTextBox* ChatTextBox;
	UPROPERTY(Meta = (BindWidget))
	UTAChatBoxWidget* ChatBoxWidget;
};