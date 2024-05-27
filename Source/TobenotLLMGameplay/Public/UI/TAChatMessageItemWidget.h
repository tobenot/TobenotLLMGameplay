// Copyright (c) 2024 tobenot, See LICENSE in the project root for license information.

// TAChatMessageItemWidget.h - 聊天信息Item类声明
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "TAChatMessageItemWidget.generated.h"

UCLASS()
class TOBENOTLLMGAMEPLAY_API UTAChatMessageItemWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category="TAChatMessageItemWidget")
	virtual void SetupMessage(const FString& Message, AActor* Sender, const FString& DynamicParam);

protected:
	UPROPERTY(Meta = (BindWidget))
	class UTextBlock* MessageTextBlock;
};