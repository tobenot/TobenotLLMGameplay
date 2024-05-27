// Copyright (c) 2024 tobenot, See LICENSE in the project root for license information.

#pragma once

#include "CoreMinimal.h"
#include "Interfaces/IHttpRequest.h"
#include "TASystemLibrary.generated.h"

DECLARE_DYNAMIC_DELEGATE_OneParam(FOnTextFileDownloaded, const FString&, DownloadedText);

/**
 * 
 */
UCLASS()
class TOBENOTLLMGAMEPLAY_API UTASystemLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
	UFUNCTION(BlueprintCallable, Category = "Chat Engine")
	static void SetGlobalProxyAddress(const FString& ProxyAddress, int32 ProxyPort);

	UFUNCTION(BlueprintCallable, Category = "Chat Engine")
	static void ClearGlobalProxy();

	UFUNCTION(BlueprintCallable, Category = "HTTP")
	static void DownloadTextFile(const FString& URL, const FOnTextFileDownloaded& OnDownloadComplete, const FOnTextFileDownloaded& OnDownloadFailed);

	UFUNCTION(BlueprintCallable, Category = "Utilities")
	static void ClipboardActionCopy(const FString& Context);
	
private:
	static void OnResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful, const FOnTextFileDownloaded& OnDownloadComplete, const FOnTextFileDownloaded& OnDownloadFailed);
	
public:
	UFUNCTION(BlueprintCallable, Category = "Localization")
	static void SetGameLanguage(const FString& Language);

	UFUNCTION(BlueprintPure, Category = "Localization")
	static FString GetGameLanguage();

	UFUNCTION(BlueprintCallable, Category = "Localization")
	static void TASetCurrentCulture(FString Culture);

	static FString CurrentGameLanguage;
};
