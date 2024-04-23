#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TAAgentComponent.generated.h"

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class TOBENOTLLMGAMEPLAY_API UTAAgentComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UTAAgentComponent();

protected:
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Agent")
	bool bEnableScheduleShout;
	
	// 函数用于调用Owner上的ShoutComponent的RequestToSpeak
	UFUNCTION(BlueprintCallable, Category="Agent")
	void RequestSpeak();

	// 变量声明为蓝图可配置
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Agent")
	float MinTimeBetweenShouts;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Agent")
	float MaxTimeBetweenShouts;

private:
	// 内部追踪下一次应该喊话的时间
	float TimeToNextShout;
	// 内部用于生成下一次喊话的时间间隔
	void ScheduleNextShout();
};