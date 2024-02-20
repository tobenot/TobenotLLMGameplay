// Copyright (c) 2024 tobenot, See LICENSE in the project root for license information.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "TAGuidSubsystem.generated.h"

/**
 * 
 */
UCLASS()
class TOBENOTLLMGAMEPLAY_API UTAGuidSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
public:
	// 根据 GUID 获取 Actor 的函数
	AActor* GetActorByGUID(const FGuid& Guid);

	// 当注册新 Actor 时调用，添加映射关系
	void RegisterActorGUID(const FGuid& Guid, AActor* Actor);

	// 当 Actor 销毁时调用，移除映射关系
	void UnregisterActorGUID(const FGuid& Guid);
	
private:
	// 用于存储 GUID 和 Actor 的映射
	UPROPERTY()
	TMap<FGuid, AActor*> GuidToActorMap;
};
