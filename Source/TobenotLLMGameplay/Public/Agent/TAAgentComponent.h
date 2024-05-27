#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TAAgentComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TOBENOTLLMGAMEPLAY_API UTAAgentComponent : public UActorComponent
{
	GENERATED_BODY()

public:    
	UTAAgentComponent();

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Agent")
	bool bEnableScheduleShout;

	UFUNCTION(BlueprintCallable, Category="Agent")
	void RequestSpeak();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Agent")
	float MinTimeBetweenShouts;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Agent")
	float MaxTimeBetweenShouts;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Agent")
	float MinTimeBetweenRetryShouts;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Agent")
	float MaxTimeBetweenRetryShouts;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Agent")
	float MinTimeBetweenDecisions;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Agent")
	float MaxTimeBetweenDecisions;

	UFUNCTION(BlueprintCallable, Category="Agent")
	void PerceiveAndDecide();

	UFUNCTION(BlueprintCallable, Category="Agent")
	void HandleDecisionResponse(const FString& Outcome, bool bIsDecisionValid);

	UFUNCTION(BlueprintCallable, Category="Agent")
	void RequestDecision();

private:
	float TimeToNextShout;
	void ScheduleNextShout();
	void SendDecisionToGameMaster(const FString& Decision);

	float TimeToNextDecision;
	void ScheduleNextDecision();
};