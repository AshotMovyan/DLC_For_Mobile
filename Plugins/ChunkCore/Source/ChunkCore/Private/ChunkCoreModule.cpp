// Copyright (C) 2017-2021 | eelDev (Dry Eel Development)

#include "ChunkCoreModule.h"
#include "ChunkCoreLogging.h"
#include <Interfaces/IPluginManager.h>

#define LOCTEXT_NAMESPACE "FChunkCoreModule"
DEFINE_LOG_CATEGORY(LogChunkCore);

void FChunkCoreModule::StartupModule()
{
	TSharedPtr<IPlugin> PluginPtr = IPluginManager::Get().FindPlugin("ChunkCore");

	FString PluginVersion;

	if (PluginPtr)
	{
		PluginVersion = PluginPtr->GetDescriptor().VersionName;
	}

	UE_LOG(LogTemp, Log, TEXT("--------------------------------------------------------------------------------"));
	UE_LOG(LogTemp, Log, TEXT("Using ChunkCore Version: %s"), *PluginVersion);
	UE_LOG(LogTemp, Log, TEXT("--------------------------------------------------------------------------------"));
}

void FChunkCoreModule::ShutdownModule()
{
	
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FChunkCoreModule, ChunkCore)