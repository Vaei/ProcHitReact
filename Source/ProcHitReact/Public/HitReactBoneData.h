// Copyright (c) Jared Taylor

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Params/HitReactParams.h"
#include "HitReactBoneData.generated.h"

/**
 * Contains per-bone data that can be reused regardless of the chosen profile
 * Joined with the profile's params
 */
UCLASS()
class PROCHITREACT_API UHitReactBoneData : public UDataAsset
{
	GENERATED_BODY()

public:
	/**
	 * Bone-specific override params
	 * Will be joined with the profile's params
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=HitReact)
	TMap<FName, FHitReactBoneOverride> BoneOverrides;
};
