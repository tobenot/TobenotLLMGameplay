// Copyright (c) 2024 tobenot, See LICENSE in the project root for license information.


#include "TAInteractiveActor.h"
#include "Chat/TAFunctionInvokeComponent.h"
#include "Components/SphereComponent.h"

ATAInteractiveActor::ATAInteractiveActor()
{
	// 初始化ChatComponent
	ChatComponent = CreateDefaultSubobject<UTAChatComponent>(TEXT("ChatComponent"));
	ChatComponent->bEnableFunctionInvoke = true;
	
	FunctionInvokeComponent  = CreateDefaultSubobject<UTAFunctionInvokeComponent >(TEXT("FunctionInvokeComponent "));
	
	// 初始化InteractionComponent
	InteractionComponent = CreateDefaultSubobject<UTAInteractionComponent>(TEXT("InteractionComponent"));

	// 初始化碰撞组件
	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComponent"));
	CollisionComponent->SetupAttachment(RootComponent);
	CollisionComponent->SetCollisionProfileName(TEXT("WorldDynamic")); // 设置为世界动态的碰撞预设
	CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CollisionComponent->CanCharacterStepUpOn = ECB_No;
	
	// 初始化静态Mesh组件
	DisplayMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DisplayMeshComponent"));
	DisplayMeshComponent->SetupAttachment(CollisionComponent); // 将Mesh附加到碰撞组件上
	// 如果你希望Mesh的碰撞用于游戏逻辑，可以设置相应的碰撞选项
	DisplayMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision); // 如果只用于显示不参与碰撞，可以禁用碰撞
}

void ATAInteractiveActor::BeginPlay()
{
	Super::BeginPlay();
}

FString ATAInteractiveActor::GetSystemPrompt()
{
	return InteractionComponent->GetFullPrompt();
}

// getter函数
UTAInteractionComponent* ATAInteractiveActor::GetInteractionComponent() const
{
	return InteractionComponent;
}