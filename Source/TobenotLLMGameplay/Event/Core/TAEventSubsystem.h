// Copyright (c) 2024 tobenot, See LICENSE in the project root for license information.

#pragma once

// TAEventSubsystem.h
// 主要负责管理和协调所有事件的系统

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "TAEventSubsystem.generated.h"

struct FTAEventInfo;
class UTAEventPool;

UCLASS()
class TOBENOTLLMGAMEPLAY_API UTAEventSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	UTAEventPool* GetEventPool()
	{
		return EventPool;
	};
	
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual void Start(const int32& GenEventNum);

	UFUNCTION(BlueprintCallable, Category = "Event")
	bool HasAnyEventsInPool() const;
	
private:
	UFUNCTION()
	void HandleGeneratedEvents(TArray<FTAEventInfo>& GeneratedEvents);

	void GenerateImageForEvent(const FTAEventInfo& GeneratedEvent);

	UPROPERTY()
	UTAEventPool* EventPool;
};