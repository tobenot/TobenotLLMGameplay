// Copyright (c) 2024 tobenot, See LICENSE in the project root for license information.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "OpenAIDefinitions.h"
#include "TAChatCallback.generated.h"

/**
 * 
 */
UCLASS(Blueprintable)
class TOBENOTLLMGAMEPLAY_API UTAChatCallback : public UObject
{
	GENERATED_BODY()
	
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FTAChatSuccessCallbackDelegate, FChatCompletion, Message);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FTAChatSuccessWithSenderCallbackDelegate, FChatCompletion, Message, AActor*, Sender);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FTAChatFailedCallbackDelegate);

public:
	UPROPERTY(BlueprintAssignable)
	FTAChatSuccessCallbackDelegate OnSuccess;

	UPROPERTY(BlueprintAssignable)
	FTAChatSuccessWithSenderCallbackDelegate OnSuccessWithSender;

	UPROPERTY(BlueprintAssignable)
	FTAChatFailedCallbackDelegate OnFailure;
};
