// Copyright (c) 2024 tobenot, See LICENSE in the project root for license information.

// TAChatBoxWidget.h - 聊天框类声明
#pragma once

#include "CoreMinimal.h"
#include "TAChatMessageItemWidget.h"
#include "Blueprint/UserWidget.h"
#include "Components/ScrollBox.h"
#include "TAChatBoxWidget.generated.h"

UCLASS()
class TOBENOTLLMGAMEPLAY_API UTAChatBoxWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UTAChatBoxWidget(const FObjectInitializer& ObjectInitializer);

	virtual void NativeConstruct() override;
	
	UFUNCTION(BlueprintCallable, Category = "TAChatBoxWidget")
	UScrollBox* GetChatScrollBox() const
	{
		return ChatScrollBox;
	}

	UPROPERTY(EditDefaultsOnly, Category = "TAChatBoxWidget")
	TSubclassOf<UTAChatMessageItemWidget> ChatMessageItemWidgetClass;
	
	UFUNCTION(BlueprintCallable, Category="TAChatBoxWidget")
	void AddChatMessage(const FString& Message, AActor* Sender, const FString& DynamicParam);
    
protected:
    
	UPROPERTY(Meta = (BindWidget))
	class UScrollBox* ChatScrollBox;  
};