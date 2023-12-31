// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "MyGameInstance.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPatchCompleteDelegate, bool, Succeeded);

UCLASS()
class UMyGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
    /** Overrides */
    virtual void Init() override;
    virtual void Shutdown() override;

    UFUNCTION(BlueprintPure, Category = "Patching|Stats")
    void GetLoadingProgress(int32& FilesDownloaded, int32& TotalFilesToDownload, float& DownloadPercent, int32& ChunksMounted, int32& TotalChunksToMount, float& MountPercent) const;

    /** Fired when the patching process succeeds or fails */
    UPROPERTY(BlueprintAssignable, Category = "Patching");
    FPatchCompleteDelegate OnPatchComplete;

    /** List of Chunk IDs to try and download */
    UPROPERTY(EditDefaultsOnly, Category = "Patching")
    TArray<int32> ChunkDownloadList;

    /** Starts the game patching process. Returns false if the patching manifest is not up to date. */
    UFUNCTION(BlueprintCallable, Category = "Patching")
    bool PatchGame();

protected:
    //Tracks Whether or not our local manifest file is up to date with the one hosted on our website
    bool bIsDownloadManifestUpToDate;

    void OnManifestUpdateComplete(bool bSuccess);

    /** Called when the chunk download process finishes */
    void OnDownloadComplete(bool bSuccess);

    /** Called whenever ChunkDownloader's loading mode is finished*/
    void OnLoadingModeComplete(bool bSuccess);

    /** Called when ChunkDownloader finishes mounting chunks */
    void OnMountComplete(bool bSuccess);

};
