// Copyright (c) 2024 tobenot, See LICENSE in the project root for license information.


#include "Scene/TAInteractiveActor.h"

#include "TASettings.h"
#include "Chat/TAFunctionInvokeComponent.h"
#include "Chat/Dialogue/TADialogueComponent.h"
#include "Components/SphereComponent.h"

void ATAInteractiveActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	   
	// 初始化InteractionComponent
	UClass* InteractionComponentClass = nullptr;
	const UTASettings* Settings = GetDefault<UTASettings>();
	if (Settings)
	{
		InteractionComponentClass = Settings->InteractionComponentClass.TryLoadClass<UTAInteractionComponent>();
	}

	// 如果没有指定类或者类加载失败，使用默认的InteractionComponentClass类
	if (!InteractionComponentClass)
	{
		InteractionComponentClass = UTAInteractionComponent::StaticClass();
	}
	
	InteractionComponent = NewObject<UTAInteractionComponent>(this, InteractionComponentClass, TEXT("InteractionComponent"));
	if (InteractionComponent)
	{
		InteractionComponent->RegisterComponent(); // 注册组件到Actor
		InteractionComponent->InitializeComponent(); // 调用自定义的初始化方法
	}
}

ATAInteractiveActor::ATAInteractiveActor()
{
	// 初始化一个简单的SceneComponent作为根组件
	RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
	SetRootComponent(RootSceneComponent);
    
	// 初始化碰撞组件，并将其附加到根组件
	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComponent"));
	CollisionComponent->SetupAttachment(RootComponent);
	CollisionComponent->SetCollisionProfileName(TEXT("WorldDynamic")); // 设置为世界动态的碰撞预设
	CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CollisionComponent->CanCharacterStepUpOn = ECB_No;

	// 初始化静态Mesh组件，并将其附加到碰撞组件
	DisplayMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DisplayMeshComponent"));
	DisplayMeshComponent->SetupAttachment(CollisionComponent);
	DisplayMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision); // 如果只用于显示不参与碰撞，可以禁用碰撞

	// 初始化ChatComponent
	ChatComponent = CreateDefaultSubobject<UTAChatComponent>(TEXT("ChatComponent"));
	ChatComponent->bEnableFunctionInvoke = true;
	
	DialogueComponent = CreateDefaultSubobject<UTADialogueComponent>(TEXT("DialogueComponent"));
	DialogueComponent->bEnableFunctionInvoke = true;
	//FunctionInvokeComponent = CreateDefaultSubobject<UTAFunctionInvokeComponent>(TEXT("FunctionInvokeComponent"));
}

void ATAInteractiveActor::BeginPlay()
{
	Super::BeginPlay();
}

FString ATAInteractiveActor::GetSystemPrompt()
{
	return InteractionComponent->GetFullPrompt();
}

FString ATAInteractiveActor::GetPersonalityPrompt() const
{
	return InteractionComponent->GetFullPrompt();
}

const FString& ATAInteractiveActor::GetAgentName() const
{
	return InteractionComponent->GetInteractableName();
}

// getter函数
UTAInteractionComponent* ATAInteractiveActor::GetInteractionComponent() const
{
	return InteractionComponent;
}