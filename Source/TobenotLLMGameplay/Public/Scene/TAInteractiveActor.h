// Copyright (c) 2024 tobenot, See LICENSE in the project root for license information.

// TAInteractiveActor.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Chat/TAInteractionComponent.h"
#include "Chat/TAChatComponent.h"
#include "Agent/TAAgentInterface.h"
#include "Save/TAGuidInterface.h"
#include "TAInteractiveActor.generated.h"

class UTADialogueComponent;
class USphereComponent;
class UTAFunctionInvokeComponent;
/**
 * TA游戏中的交互物基类
 */
UCLASS()
class TOBENOTLLMGAMEPLAY_API ATAInteractiveActor : public AActor
	,public ITAAgentInterface
	,public ITAGuidInterface
{
	GENERATED_BODY()
	virtual void OnConstruction(const FTransform& Transform) override;
public:	
	ATAInteractiveActor();

protected:
	virtual void BeginPlay() override;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* RootSceneComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UTAChatComponent* ChatComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UTADialogueComponent* DialogueComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UTAFunctionInvokeComponent* FunctionInvokeComponent ;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UTAInteractionComponent* InteractionComponent;
	
	// 添加碰撞组件
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USphereComponent* CollisionComponent;

	// 添加静态Mesh组件
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* DisplayMeshComponent;
	
	// ITAAgentInterface中定义的获取系统prompt的函数实现
	virtual FString GetSystemPrompt() override;
	virtual FString GetPersonalityPrompt() const override;
	virtual const FString& GetAgentName() const override;

	virtual int32 GetAgentSpeakPriority() const override{return 150;}; //交互物先说

public:
	UFUNCTION(BlueprintCallable, Category = "TAInteractiveActor")
	UTAInteractionComponent* GetInteractionComponent() const;
};