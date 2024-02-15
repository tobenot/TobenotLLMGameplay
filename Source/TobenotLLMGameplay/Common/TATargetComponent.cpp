// Copyright (c) 2024 tobenot
// This code is licensed under the MIT License. See LICENSE in the project root for license information.

#include "TATargetComponent.h"

#include "TAAgentInterface.h"

DEFINE_LOG_CATEGORY_STATIC(LogTATarget, Log, All);

// 构造函数
UTATargetComponent::UTATargetComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

// 调用BeginPlay时，进行初始化
void UTATargetComponent::BeginPlay()
{
	Super::BeginPlay();
	
	UpdateNearbyTargets();
	StartTargetCheckTimer();
}

// 轮选下一个目标
void UTATargetComponent::SelectNextTarget()
{
	CycleToNextTarget();
}

// 获取当前选中的目标
AActor* UTATargetComponent::GetCurrentTarget() const
{
	return CurrentTarget;
}

// 获取附近的可交互目标列表
const TArray<AActor*>& UTATargetComponent::GetNearbyTargets() const
{
	return NearbyTargets;
}
// 检查目标列表是否有变化的辅助函数
bool UTATargetComponent::HasTargetListChanged(const TArray<AActor*>& OldList, const TArray<AActor*>& NewList) const
{
	if (OldList.Num() != NewList.Num())
	{
		return true;
	}

	for (AActor* Actor : NewList)
	{
		if (!OldList.Contains(Actor))
		{
			return true;
		}
	}

	return false;
}

// 更新附近目标列表
void UTATargetComponent::UpdateNearbyTargets()
{
	TArray<AActor*> OldNearbyTargets = NearbyTargets;

	// 清空当前的目标列表
	NearbyTargets.Empty();

	// 获取Actor的位置
	FVector Location = GetOwner()->GetActorLocation();

	// 定义碰撞查询参数
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(GetOwner()); // 忽略自己
	QueryParams.bReturnPhysicalMaterial = false;

	// 定义碰撞形状，这里使用球形
	FCollisionShape CollisionShape;
	CollisionShape.SetSphere(SearchRadius);

	// 执行重叠检查，这里我们使用所有的Actor类型
	TArray<FOverlapResult> OverlapResults;
	GetWorld()->OverlapMultiByChannel(
		OverlapResults,
		Location,
		FQuat::Identity,
		ECollisionChannel::ECC_WorldDynamic, // 注意：这里可能需要修改为合适的碰撞通道
		CollisionShape,
		QueryParams
	);

	// 遍历重叠结果
	for (const FOverlapResult& Result : OverlapResults)
	{
		AActor* PotentialTarget = Result.GetActor();
		if (PotentialTarget
			&& !PotentialTarget->ActorHasTag(FName("CanNotBeTargeted"))
			&& !NearbyTargets.Contains(PotentialTarget)
			&& IsValid(PotentialTarget))
		{
			// 检查目标是否实现了ITAAgentInterface接口
			ITAAgentInterface* AgentInterface = Cast<ITAAgentInterface>(PotentialTarget);
			if(AgentInterface)
			{
				// 将实现了接口的潜在目标添加到列表中
				NearbyTargets.Add(PotentialTarget);
			}
		}
	}

	if (HasTargetListChanged(OldNearbyTargets, NearbyTargets))
	{
		// 如果列表发生了变化，则触发目标列表更新事件
		OnTargetListUpdated.Broadcast();
	}

	// 检查当前选中的目标是否仍在更新后的NearbyTargets数组中
	if (CurrentTarget && !NearbyTargets.Contains(CurrentTarget))
	{
		// 如果当前选中的目标不在范围内，则取消选中
		CurrentTarget = nullptr;
		OnTargetChanged.Broadcast(CurrentTarget);
	}
	else if (!CurrentTarget && NearbyTargets.Num() > 0)
	{
		// 选取第一个实现了接口的Actor作为当前目标
		CurrentTarget = NearbyTargets[0];
		OnTargetChanged.Broadcast(CurrentTarget);
	}
}

// 轮选逻辑的辅助函数
void UTATargetComponent::CycleToNextTarget()
{
	if (NearbyTargets.Num() == 0)
	{
		// 如果没有目标，确保CurrentTarget为空
		CurrentTarget = nullptr;
	}
	else
	{
		// 如果当前没有选中的目标，或者已经是列表中的最后一个目标，则选择列表中的第一个目标
		int32 CurrentIndex = NearbyTargets.IndexOfByKey(CurrentTarget);
		if (CurrentIndex == INDEX_NONE || CurrentIndex == NearbyTargets.Num() - 1)
		{
			CurrentTarget = NearbyTargets[0];
		}
		else
		{
			// 否则，选择下一个目标
			CurrentTarget = NearbyTargets[++CurrentIndex];
		}
		// 触发目标变更事件
		OnTargetChanged.Broadcast(CurrentTarget);
	}
}

// 设置检测目标的计时器
void UTATargetComponent::StartTargetCheckTimer()
{
	// 设置定时器以每0.3秒调用UpdateNearbyTargets函数
	GetWorld()->GetTimerManager().SetTimer(TargetingCheckTimerHandle, this, &UTATargetComponent::UpdateNearbyTargets, 0.3f, true);
}
