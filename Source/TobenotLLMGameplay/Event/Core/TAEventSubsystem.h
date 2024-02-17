// Copyright (c) 2024 tobenot, See LICENSE in the project root for license information.

#pragma once

// TAEventSubsystem.h
// 主要负责管理和协调所有事件的系统

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "TAEventSubsystem.generated.h"

class UTAEventPool;

UCLASS()
class TOBENOTLLMGAMEPLAY_API UTAEventSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	// 初始化事件子系统
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	// 清理/卸载事件子系统
	virtual void Deinitialize() override;

	UPROPERTY(BlueprintReadOnly, Category = "Event")
	UTAEventPool* EventPool;
};