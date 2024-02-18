#pragma once

#include "CoreMinimal.h"
#include "TAPromptDefinitions.generated.h"

USTRUCT(BlueprintType)
struct TOBENOTLLMGAMEPLAY_API FTAPrompt
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prompt")
	FString PromptTemplate;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prompt")
	int32 VersionNumber;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prompt")
	bool bUseJsonFormat;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prompt")
	FString PromptExample;

	explicit FTAPrompt(const FString& InPromptTemplate = "", int32 InVersionNumber = 1, bool InUseJsonFormat = false, const FString& InPromptExample = "")
		: PromptTemplate(InPromptTemplate),
		  VersionNumber(InVersionNumber),
		  bUseJsonFormat(InUseJsonFormat),
		  PromptExample(InPromptExample)
	{
	}
};