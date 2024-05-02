// Copyright (c) 2024 tobenot
// This code is licensed under the MIT License. See LICENSE in the project root for license information.


#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TATargetComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TOBENOTLLMGAMEPLAY_API UTATargetComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UTATargetComponent();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TA|Targeting")
	float SearchRadius = 500.f;

	// 在一个循环结束之后轮空一次吗？
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TA|Targeting")
	bool bEmptyBeforeCycle = false;

	// 轮选下一个目标的函数
	UFUNCTION(BlueprintCallable, Category="TA|Targeting")
	void SelectNextTarget();

	// 获取当前选中的目标
	UFUNCTION(BlueprintCallable, Category="TA|Targeting")
	AActor* GetCurrentTarget() const;

	// 获取附近的可交互目标列表
	UFUNCTION(BlueprintCallable, Category="TA|Targeting")
	const TArray<AActor*>& GetNearbyTargets() const;

	// 目标变更事件的委托
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FTargetChangedSignature, AActor*, NewTarget);

	// 目标变更事件
	UPROPERTY(BlueprintAssignable, Category="TA|Targeting")
	FTargetChangedSignature OnTargetChanged;
	
	// 目标列表更新事件的委托
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FTargetListUpdatedSignature);

	// 目标列表更新事件
	UPROPERTY(BlueprintAssignable, Category="TA|Targeting")
	FTargetListUpdatedSignature OnTargetListUpdated;
	
protected:
	virtual void BeginPlay() override;

public:
	// 附近的可交互目标列表
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "TA|Targeting")
	TArray<AActor*> NearbyTargets;

	// 当前选中的目标
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "TA|Targeting")
	AActor* CurrentTarget = nullptr;

	UFUNCTION(BlueprintCallable, Category="TA|Targeting")
	void SetCurrentTarget(AActor* NewTarget){CurrentTarget = NewTarget;}
	
private:
	// 更新附近目标列表的函数
	void UpdateNearbyTargets();

	// 轮选逻辑的辅助函数
	void CycleToNextTarget();
	
	bool HasTargetListChanged(const TArray<AActor*>& OldList, const TArray<AActor*>& NewList) const;
public:
	void StartTargetCheckTimer();
	
private:
	FTimerHandle TargetingCheckTimerHandle;
};
