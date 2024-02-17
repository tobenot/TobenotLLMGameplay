// Copyright (c) 2024 tobenot, See LICENSE in the project root for license information.

// TAEventPool.h
// 管理游戏中的活跃/待触发事件集合

#pragma once

#include "CoreMinimal.h"
#include "TAEventInstance.h"
#include "TAEventPool.generated.h"

UCLASS()
class TOBENOTLLMGAMEPLAY_API UTAEventPool : public UObject
{
	GENERATED_BODY()

public:
	// 活跃事件的集合
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Event")
	TArray<UTAEventInstance*> ActiveEvents;

	// 待触发的事件的集合
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Event")
	TArray<UTAEventInstance*> PendingEvents;

	UFUNCTION(BlueprintCallable, Category = "Event")
	void AddEvent(UTAEventInstance* Event);
};
