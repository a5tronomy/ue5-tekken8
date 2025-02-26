// Copyright Epic Games, Inc. All Rights Reserved.

#include "Engine/PrimaryAssetLabel.h"
#include "AssetRegistry/AssetData.h"
#include "AssetRegistry/IAssetRegistry.h"
#include "Engine/DataAsset.h"
#include "Misc/PackageName.h"
#include "Engine/AssetManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(PrimaryAssetLabel)

#if WITH_EDITOR
#include "ICollectionManager.h"
#include "CollectionManagerModule.h"
#endif

const FName UPrimaryAssetLabel::DirectoryBundle = FName("Directory");
const FName UPrimaryAssetLabel::CollectionBundle = FName("Collection");

// TEKKEN 8 Custom Modification
const FName UPrimaryAssetLabel::ExplicitDirectoriesBundle  = FName("ExplicitDirectories");

UPrimaryAssetLabel::UPrimaryAssetLabel()
{
	bLabelAssetsInMyDirectory = false;
	bIsRuntimeLabel = false;

	// By default have low priority and don't recurse
	Rules.bApplyRecursively = false;
	Rules.Priority = 0;
}

#if WITH_EDITORONLY_DATA
void UPrimaryAssetLabel::UpdateAssetBundleData()
{
	Super::UpdateAssetBundleData();

	if (!UAssetManager::IsValid())
	{
		return;
	}

	UAssetManager& Manager = UAssetManager::Get();
	IAssetRegistry& AssetRegistry = Manager.GetAssetRegistry();
	
	/* Old version of bLabelAssetsInMyDirectory
	if (bLabelAssetsInMyDirectory)
	{
		FName PackagePath = FName(*FPackageName::GetLongPackagePath(GetOutermost()->GetName()));

		TArray<FAssetData> DirectoryAssets;
		AssetRegistry.GetAssetsByPath(PackagePath, DirectoryAssets, true);

		TArray<FTopLevelAssetPath> NewPaths;

		for (const FAssetData& AssetData : DirectoryAssets)
		{
			FSoftObjectPath AssetRef = Manager.GetAssetPathForData(AssetData);

			if (!AssetRef.IsNull())
			{
				NewPaths.Add(AssetRef.GetAssetPath());
			}
		}

		// Fast set, destroys NewPaths
		AssetBundleData.SetBundleAssets(DirectoryBundle, MoveTemp(NewPaths));
	} */

	// TEKKEN 8 Custom Modification
	if (bLabelAssetsInMyDirectory)
	{
		FName PackagePath = FName(*FPackageName::GetLongPackagePath(GetOutermost()->GetName()));

		TArray<FAssetData> DirectoryAssets;
		AssetRegistry.GetAssetsByPath(PackagePath, DirectoryAssets, true);

		TArray<FTopLevelAssetPath> NewPaths;
		TSet<FTopLevelAssetPath> ExcludedAssets;
		
		for (const FDirectoryPath& ExcludeDir : ExcludeDirectories)
		{
			FName ExcludePath = FName(*ExcludeDir.Path);
			TArray<FAssetData> ExcludeAssets;
			AssetRegistry.GetAssetsByPath(ExcludePath, ExcludeAssets, true);

			for (const FAssetData& AssetData : ExcludeAssets)
			{
				FSoftObjectPath ExcludeAssetRef = Manager.GetAssetPathForData(AssetData);
				if (!ExcludeAssetRef.IsNull())
				{
					ExcludedAssets.Add(ExcludeAssetRef.GetAssetPath());
				}
			}
		}
		
		for (const FAssetData& AssetData : DirectoryAssets)
		{
			FSoftObjectPath AssetRef = Manager.GetAssetPathForData(AssetData);

			if (!AssetRef.IsNull() && !ExcludedAssets.Contains(AssetRef.GetAssetPath()))
			{
				NewPaths.Add(AssetRef.GetAssetPath());
			}
		}
		
		AssetBundleData.SetBundleAssets(DirectoryBundle, MoveTemp(NewPaths));
	}

	// TEKKEN 8 Custom Modification
	if (ExplicitDirectories.Num() > 0)
	{
		TArray<FTopLevelAssetPath> ExplicitDirectoryPaths;
		TSet<FTopLevelAssetPath> ExcludedAssets;
		
		for (const FDirectoryPath& ExcludeDir : ExcludeDirectories)
		{
			FName ExcludePath = FName(*ExcludeDir.Path);
			TArray<FAssetData> ExcludeAssets;
			AssetRegistry.GetAssetsByPath(ExcludePath, ExcludeAssets, true);

			for (const FAssetData& AssetData : ExcludeAssets)
			{
				FSoftObjectPath ExcludeAssetRef = Manager.GetAssetPathForData(AssetData);
				if (!ExcludeAssetRef.IsNull())
				{
					ExcludedAssets.Add(ExcludeAssetRef.GetAssetPath());
				}
			}
		}
		
		for (const FDirectoryPath& Directory : ExplicitDirectories)
		{
			FName DirectoryPackagePath = FName(*Directory.Path);
			TArray<FAssetData> DirectoryAssets;
			AssetRegistry.GetAssetsByPath(DirectoryPackagePath, DirectoryAssets, true);

			for (const FAssetData& AssetData : DirectoryAssets)
			{
				FSoftObjectPath AssetRef = Manager.GetAssetPathForData(AssetData);

				if (!AssetRef.IsNull() && !ExcludedAssets.Contains(AssetRef.GetAssetPath()))
				{
					ExplicitDirectoryPaths.Add(AssetRef.GetAssetPath());
				}
			}
		}
		
		AssetBundleData.SetBundleAssets(ExplicitDirectoriesBundle, MoveTemp(ExplicitDirectoryPaths));
	}
	
	if (AssetCollection.CollectionName != NAME_None)
	{
		TArray<FTopLevelAssetPath> NewPaths;
		TArray<FSoftObjectPath> CollectionAssets;
		ICollectionManager& CollectionManager = FCollectionManagerModule::GetModule().Get();
		CollectionManager.GetAssetsInCollection(AssetCollection.CollectionName, ECollectionShareType::CST_All, CollectionAssets);
		for (int32 Index = 0; Index < CollectionAssets.Num(); ++Index)
		{
			FAssetData FoundAsset = Manager.GetAssetRegistry().GetAssetByObjectPath(CollectionAssets[Index]);
			FSoftObjectPath AssetRef = Manager.GetAssetPathForData(FoundAsset);

			if (!AssetRef.IsNull())
			{
				NewPaths.Add(AssetRef.GetAssetPath());
			}
		}

		// Fast set, destroys NewPaths
		AssetBundleData.SetBundleAssets(CollectionBundle, MoveTemp(NewPaths));
	}
	
	// Update rules
	FPrimaryAssetId PrimaryAssetId = GetPrimaryAssetId();
	Manager.SetPrimaryAssetRules(PrimaryAssetId, Rules);
}
#endif

