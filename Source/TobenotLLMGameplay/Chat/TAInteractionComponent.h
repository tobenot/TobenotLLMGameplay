// Copyright (c) 2024 tobenot, See LICENSE in the project root for license information.

// UTAInteractionComponent.h
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TobenotLLMGameplay/Common/TAPromptDefinitions.h"
#include "TAInteractionComponent.generated.h"

USTRUCT(BlueprintType)
struct FInteractableInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString UniqueFeature;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Objective;
};

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
	virtual void InitializeComponent() override;
	virtual void BeginPlay() override;
	
	UPROPERTY(BlueprintReadOnly, VisibleDefaultsOnly, Category = "Prompts")
	FTAPrompt InteractiveActorPromptTemplate;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Prompts")
	FInteractableInfo InteractableInfo;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Prompts")
	FString BelongEventDescription;
	
	FString GetFullPrompt();
	const FString& GetInteractableName(){return InteractableInfo.Name;};
};