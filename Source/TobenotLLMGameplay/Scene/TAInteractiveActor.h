// Copyright (c) 2024 tobenot, See LICENSE in the project root for license information.

// TAInteractiveActor.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TobenotLLMGameplay/Chat/TAInteractionComponent.h"
#include "TobenotLLMGameplay/Chat/TAChatComponent.h"
#include "TobenotLLMGameplay/Common/TAAgentInterface.h"
#include "TobenotLLMGameplay/Save/TAGuidInterface.h"
#include "TAInteractiveActor.generated.h"

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

	virtual const FString& GetAgentName() override;

public:
	UFUNCTION(BlueprintCallable, Category = "TAInteractiveActor")
	UTAInteractionComponent* GetInteractionComponent() const;
};