// Copyright (c) 2024 tobenot, See LICENSE in the project root for license information.

// UTAInteractionComponent.h
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TobenotLLMGameplay/Common/TAPromptDefinitions.h"
#include "TAInteractionComponent.generated.h"

/**
 * TA游戏中用于特定交互物的组件
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TOBENOTLLMGAMEPLAY_API UTAInteractionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UTAInteractionComponent();
	virtual void InitPrompt();
	
	virtual void BeginPlay() override;
	
	UPROPERTY(BlueprintReadOnly, VisibleDefaultsOnly, Category = "Prompts")
	FTAPrompt InteractiveActorPromptTemplate;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Prompts")
	FString InteractableActorProfile;
	
	FString GetFullPrompt();
};