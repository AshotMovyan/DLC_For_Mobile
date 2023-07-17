// Fill out your copyright notice in the Description page of Project Settings.


#include "MyGameInstance.h"

#include "ChunkDownloader.h"
#include "Misc/CoreDelegates.h"
#include "AssetRegistryModule.h"

void UMyGameInstance::Init()
{
    Super::Init();

    const FString DeploymentName = "PatchingDemoLive";
    const FString ContentBuildId = "HttpRequestLive";

    // initialize the chunk downloader
    TSharedRef<FChunkDownloader> Downloader = FChunkDownloader::GetOrCreate();
    Downloader->Initialize("Windows", 8);

    // load the cached build ID
    if (Downloader->LoadCachedBuild(DeploymentName))
    {
        UE_LOG(LogTemp, Display, TEXT("Cached Build Succeeded"));
    }
    else
    {
        UE_LOG(LogTemp, Display, TEXT("Cached Build Failed"));

    }
    UE_LOG(LogTemp, Display, TEXT("Updating Build Manifest"));

    // update the build manifest file
    TFunction<void(bool bSuccess)> UpdateCompleteCallback = [&](bool bSuccess) {bIsDownloadManifestUpToDate = bSuccess; };
    Downloader->UpdateBuild(DeploymentName, ContentBuildId, UpdateCompleteCallback);
    
    UE_LOG(LogTemp, Display, TEXT("Content build ID %s"), *Downloader->GetContentBuildId());
    UE_LOG(LogTemp, Display, TEXT("Content build ID %s"), *Downloader->GetDeploymentName());    
}

void UMyGameInstance::Shutdown()
{
    Super::Shutdown();

    // Shut down ChunkDownloader
    FChunkDownloader::Shutdown();
}

void UMyGameInstance::GetLoadingProgress(int32& BytesDownloaded, int32& TotalBytesToDownload, float& DownloadPercent, int32& ChunksMounted, int32& TotalChunksToMount, float& MountPercent) const
{
    //Get a reference to ChunkDownloader
    TSharedRef<FChunkDownloader> Downloader = FChunkDownloader::GetChecked();

    //Get the loading stats struct
    FChunkDownloader::FStats LoadingStats = Downloader->GetLoadingStats();

    //Get the bytes downloaded and bytes to download
    BytesDownloaded = LoadingStats.BytesDownloaded;
    TotalBytesToDownload = LoadingStats.TotalBytesToDownload;

    //Get the number of chunks mounted and chunks to download
    ChunksMounted = LoadingStats.ChunksMounted;
    TotalChunksToMount = LoadingStats.TotalChunksToMount;

    //Calculate the download and mount percent using the above stats
    DownloadPercent = (float)BytesDownloaded / (float)TotalBytesToDownload;
    MountPercent = (float)ChunksMounted / (float)TotalChunksToMount;

}

bool UMyGameInstance::PatchGame()
{
    // make sure the download manifest is up to date
    if (true)
    {
        // get the chunk downloader
        TSharedRef<FChunkDownloader> Downloader = FChunkDownloader::GetChecked();

        // report current chunk status
        for (int32 ChunkID : ChunkDownloadList)
        {
            int32 ChunkStatus = static_cast<int32>(Downloader->GetChunkStatus(ChunkID));
            UE_LOG(LogTemp, Display, TEXT("Chunk %i status: %i"), ChunkID, ChunkStatus);
        }

        TFunction<void(bool bSuccess)> DownloadCompleteCallback = [&](bool bSuccess) {OnDownloadComplete(bSuccess); };
        Downloader->DownloadChunks(ChunkDownloadList, DownloadCompleteCallback, 1);

        // start loading mode
        TFunction<void(bool bSuccess)> LoadingModeCompleteCallback = [&](bool bSuccess) {OnLoadingModeComplete(bSuccess); };
        Downloader->BeginLoadingMode(LoadingModeCompleteCallback);
        return true;
    }

    // we couldn't contact the server to validate our manifest, so we can't patch
    UE_LOG(LogTemp, Display, TEXT("Manifest Update Failed. Can't patch the game"));

    return false;
}

void UMyGameInstance::OnManifestUpdateComplete(bool bSuccess)
{
	bIsDownloadManifestUpToDate = bSuccess;

}

void UMyGameInstance::OnDownloadComplete(bool bSuccess)
{
    if (bSuccess)
    {
        UE_LOG(LogTemp, Display, TEXT("Download complete"));

        // get the chunk downloader
        TSharedRef<FChunkDownloader> Downloader = FChunkDownloader::GetChecked();

        FJsonSerializableArrayInt DownloadedChunks;

        for (int32 ChunkID : ChunkDownloadList)
        {
            DownloadedChunks.Add(ChunkID);
        }

        //Mount the chunks
        TFunction<void(bool bSuccess)> MountCompleteCallback = [&](bool bSuccess) {OnMountComplete(bSuccess); };
        Downloader->MountChunks(DownloadedChunks, MountCompleteCallback);

        OnPatchComplete.Broadcast(true);

    }
    else
    {

        UE_LOG(LogTemp, Display, TEXT("Load process failed"));

        // call the delegate
        OnPatchComplete.Broadcast(false);
    }
}

void UMyGameInstance::OnLoadingModeComplete(bool bSuccess)
{
    OnDownloadComplete(bSuccess);

}

void UMyGameInstance::OnMountComplete(bool bSuccess)
{
    OnPatchComplete.Broadcast(bSuccess);

}
