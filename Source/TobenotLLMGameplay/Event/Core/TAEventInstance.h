// Copyright (c) 2024 tobenot, See LICENSE in the project root for license information.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Event/Data/TAEventInfo.h"
#include "TAEventInstance.generated.h"

class UTAAreaScene;
/**
 * 
 */
UCLASS()
class TOBENOTLLMGAMEPLAY_API UTAEventInstance : public UObject
{
	GENERATED_BODY()

public:
	// 绑定到此实例的事件信息
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event")
	FTAEventInfo EventInfo;

	// 用于打印信息的函数，以模拟事件的触发
	UFUNCTION(BlueprintCallable, Category = "Event")
	void TriggerEvent();

private:
	bool bTriggered = false;
};
