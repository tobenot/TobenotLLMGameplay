// Copyright (c) 2024 tobenot, See LICENSE in the project root for license information.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "TAShoutManager.generated.h"

struct FChatCompletion;
class UTAShoutComponent;

/**
 * 
 */
UCLASS()
class TOBENOTLLMGAMEPLAY_API UTAShoutManager : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	// Initializes the instance of the subsystem.
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	// Cleans up the instance before it is destroyed.
	virtual void Deinitialize() override;

	// Functions to manage shout components.
	void RegisterShoutComponent(UTAShoutComponent* Component);
	void UnregisterShoutComponent(UTAShoutComponent* Component);

	// Function to broadcast a shout to listening components.
	void BroadcastShout(const FChatCompletion& Message, AActor* Shouter, float Volume);

private:
	// Stores references to all registered shout components.
	UPROPERTY()
	TArray<UTAShoutComponent*> RegisteredShoutComponents;

	// Helper function to get all shout components in range of the shouter.
	TArray<UTAShoutComponent*> GetShoutComponentsInRange(AActor* Shouter, float Range);
};
