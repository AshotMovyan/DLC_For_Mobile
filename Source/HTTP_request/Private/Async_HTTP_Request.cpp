// Fill out your copyright notice in the Description page of Project Settings.


#include "Async_HTTP_Request.h"
#include "ArchiverZipIncludes.h"
//#include "RuntimeArchiverBase.h"
#include "Misc/Paths.h"

//#include "FileHelper.h"


UAsync_HTTP_Request* UAsync_HTTP_Request::DownloadAndUnzipContent(FHttpRequestData HttpRequestData, FUnzipData UnzipData, FDirectory Directory)
{
    UAsync_HTTP_Request* BlueprintAction = NewObject<UAsync_HTTP_Request>();
    BlueprintAction->HttpRequestData = HttpRequestData;
    BlueprintAction->UnzipData = UnzipData;
    BlueprintAction->Directory = Directory;
    //BlueprintAction->MyArchiver = URuntimeArchiverBase::CreateRuntimeArchiver(BlueprintAction, UnzipData.ArchiverClass);


    return BlueprintAction;
}

void UAsync_HTTP_Request::Activate()
{
    //Super::Activate();

    LongUnarchiveDirectoryPath = FPaths::Combine(FPaths::ProjectContentDir(), Directory.UnarchiveDirectoryPath);
    LongDownloadArchivePath = FPaths::Combine("/storage/emulated/0/Android/data/com.YourCompany.HTTP_request/", Directory.DownloadArchivePath);

    //FString ZipFilePath = FPaths::ConvertRelativePathToFull(FPlatformFileManager::Get().GetPlatformFile().GetGameUserDeveloperDir(), TEXT("Content/Temp/cookies/Test.zip"));

    UE_LOG(LogTemp, Error, TEXT("%s"), *LongUnarchiveDirectoryPath);
    UE_LOG(LogTemp, Error, TEXT("%s"), *LongDownloadArchivePath);


    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();

    //Directory.ArchivePath = SelectIsParentDirectory(Directory.ArchivePath, Directory.bAddParentDirectory);
    //Directory.DirectoryPath = SelectIsParentDirectory(Directory.DirectoryPath, Directory.bAddParentDirectory);

    // Set the request verb based on the provided enum
    switch (HttpRequestData.Verb)
    {
    case EHTTPRequestVerb::GET:
        Request->SetVerb(TEXT("GET"));
        break;
    case EHTTPRequestVerb::POST:
        Request->SetVerb(TEXT("POST"));
        break;
    case EHTTPRequestVerb::PUT:
        Request->SetVerb(TEXT("PUT"));
        break;
    case EHTTPRequestVerb::DELETE:
        Request->SetVerb(TEXT("DELETE"));
        break;
    default:
        break;
    }

    Request->SetURL(HttpRequestData.URL);
    Request->SetContentAsString(HttpRequestData.Content);
    Request->OnProcessRequestComplete().BindUObject(this, &UAsync_HTTP_Request::HandleHTTPRequestCompleted);
    Request->OnRequestProgress().BindUObject(this, &UAsync_HTTP_Request::HandleRequestProgress);

    Request->ProcessRequest();
}

void UAsync_HTTP_Request::HandleHTTPRequestCompleted(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
    if (bWasSuccessful && Response.IsValid())
    {
        //BlueprintAction
        //URuntimeArchiverBase* RuntimeArchiverRef = URuntimeArchiverBase::CreateRuntimeArchiver(BlueprintAction, ArchiverClass);


        //FString SavePath = Directory.bAddParentDirectory ? FPaths::ProjectContentDir() + Directory.DirectoryPath : Directory.DirectoryPath + Directory.EntryName;
        //FString SavePath = FPaths::ProjectContentDir() + Directory.DownloadArchivePath;
        //FFileHelper::SaveArrayToFile(Response->GetContent(), *LongDownloadArchivePath);
        //LongDownloadArchivePath = "/storage/emulated/0/Android/data/com.YourCompany.HTTP_request/files/Test.zip";
        FFileHelper::SaveArrayToFile(Response->GetContent(), *LongDownloadArchivePath);


        //FString SavePath = FPaths::ProjectContentDir() + "AssetsArchive.zip";
        if (FPaths::FileExists(*LongDownloadArchivePath))
        {
            UE_LOG(LogTemp, Error, TEXT("Saved Array is Exist"));
            ExtractAssets(*LongDownloadArchivePath, *LongUnarchiveDirectoryPath);
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Saved Array is not exist"));
            OnRequestCompleted.Broadcast(false);
        }

        //MyUnarchivingToStorage();
        // Call the completion delegate with the success status
        //OnRequestCompleted.Broadcast(true);
    }
    else
    {
        GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, TEXT("Falid! HTTP Request is falid"));
        // Call the completion delegate with the failure status
        OnRequestCompleted.Broadcast(false);
    }

    // Mark the action as completed
    //SetReadyToDestroy();
    //MarkAsFinished();
}

void UAsync_HTTP_Request::HandleRequestProgress(FHttpRequestPtr Request, int32 BytesSent, int32 BytesReceived)
{
    // Call the progress delegate with the bytes sent and received
    OnRequestProgress.Broadcast(BytesSent, BytesReceived);
}


void UAsync_HTTP_Request::ExtractAssets(const FString& ArchivePath, const FString& DestinationPath)
{
    //FString FullArchivePath = FPaths::ConvertRelativePathToFull(FPaths::ProjectDir() + "coock/Test.zip");
    FString FullArchivePath = FPaths::ConvertRelativePathToFull(ArchivePath);
    //FString FullArchivePath = "/storage/emulated/0/Android/data/com.YourCompany.HTTP_request/files/Test.zip";
    FString FullDestinationPath = FPaths::ConvertRelativePathToFull(DestinationPath);

    UE_LOG(LogTemp, Warning, TEXT("\n FullArchivePath - %s\n FullDestinationPath - %s"), *FullArchivePath, *FullDestinationPath);
    mz_zip_archive zipArchive;
    memset(&zipArchive, 0, sizeof(zipArchive));

    if (mz_zip_reader_init_file(&zipArchive, TCHAR_TO_UTF8(*FullArchivePath), 0))
    {
        int32 numFiles = mz_zip_reader_get_num_files(&zipArchive);
        for (int32 i = 0; i < numFiles; ++i)
        {
            mz_zip_archive_file_stat fileStat;
            if (!mz_zip_reader_file_stat(&zipArchive, i, &fileStat))
            {
                OnRequestCompleted.Broadcast(false);
                UE_LOG(LogTemp, Warning, TEXT("Failed to retrieve file info from ZIP archive"));
                return;
                break;
            }

            FString EntryName = UTF8_TO_TCHAR(fileStat.m_filename);
            FString EntryPath = FullDestinationPath / EntryName;

            if (mz_zip_reader_is_file_a_directory(&zipArchive, i))
            {
                FPaths::NormalizeDirectoryName(EntryPath);
                FPlatformFileManager::Get().GetPlatformFile().CreateDirectoryTree(*EntryPath);
            }
            else
            {
                TArray<uint8> FileContents;
                FileContents.SetNum(fileStat.m_uncomp_size);

                if (mz_zip_reader_extract_to_mem(&zipArchive, i, FileContents.GetData(), FileContents.Num(), 0))
                {
                    FFileHelper::SaveArrayToFile(FileContents, *EntryPath);
                }
                else
                {
                    OnRequestCompleted.Broadcast(false);
                    UE_LOG(LogTemp, Warning, TEXT("Failed to extract file from ZIP archive: %s"), *EntryName);
                    return;
                }
            }
        }

        mz_zip_reader_end(&zipArchive);
    }
    else
    {
        OnRequestCompleted.Broadcast(false);
        UE_LOG(LogTemp, Warning, TEXT("Failed to initialize ZIP archive: %s"), *FullArchivePath);
        return;
    }
    OnRequestCompleted.Broadcast(true);
    //UStaticMesh* Mymesh = LoadObject<UStaticMesh>(nullptr, *FullDestinationPath);
}

//void UAsync_HTTP_Request::MyUnarchivingToStorage()
//{
//    if (!IsValid(MyArchiver))
//    {
//        UE_LOG(LogTemp, Error, TEXT("Failed Myarchiver Is not valied")); 
//        GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, TEXT("Falid! MyArchive Is Not Valid"));
//        OnRequestCompleted.Broadcast(false);
//        return;
//    }
//    if (!MyArchiver->OpenArchiveFromStorage(LongDownloadArchivePath))
//    {
//        GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(TEXT("Falid on open archive in %s Path"), *LongDownloadArchivePath));
//        OnRequestCompleted.Broadcast(false);
//        return;
//    }
//
//    UnarchiveOperationResult.BindDynamic(this, &UAsync_HTTP_Request::UnarchiveOnResult_Callback);
//
//    MyArchiver->ExtractEntriesToStorage_Directory(UnarchiveOperationResult, MoveTemp(Directory.EntryName), MoveTemp(LongUnarchiveDirectoryPath), Directory.bAddParentDirectory, UnzipData.bForceOverwrite);
//}
void UAsync_HTTP_Request::UnarchiveOnResult_Callback(bool bSuccess)
{

}
//void UAsync_HTTP_Request::UnarchiveOnResult_Callback(bool bSuccess)
//{
//    UnarchiveOperationResult.Clear();
//
//    bSuccess &= MyArchiver->CloseArchive();
//
//    if (!bSuccess)
//    {
//        GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, TEXT("Falid! bSuccess is false"));
//
//        OnRequestCompleted.Broadcast(false);
//        return;
//    }
//
//    OnRequestCompleted.Broadcast(true);
//}

