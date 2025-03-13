// Copyright (c) Jared Taylor

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "HitReactTypes.h"
#include "Physics/HitReactPhysicsState.h"
#include "HitReactParams.generated.h"

class UHitReactBoneData;
class UHitReactProfile;

/**
 * Manages global toggle parameters for enabling/disabling the hit react system, including gameplay tag-based toggling.
 */
USTRUCT(BlueprintType)
struct PROCHITREACT_API FHitReactGlobalToggle
{
	GENERATED_BODY()

	FHitReactGlobalToggle()
		: bToggleStateUsingTags(false)
	{}
	/** Global interp toggle parameters for enabling/disabling the hit react system */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=HitReact, meta=(ShowOnlyInnerProperties))
	FHitReactPhysicsStateParamsSimple Params;

	/**
	 * Requires GameplayAbilities plugin to be loaded!
	 * 
	 * Whether to toggle the system using gameplay tags
	 * Disabling this can be a performance optimization if you know the system will not be toggled at runtime via tags
	 *	because we won't have to look for AbilitySystemComponent
	 * @see DisableTags, EnableTagsOverride
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=HitReact)
	bool bToggleStateUsingTags;
	
	/** If component owner has any gameplay tags assigned via their AbilitySystemComponent, this will be toggled to a disabled state using GlobalToggleParams */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=HitReact, meta=(EditCondition="bToggleStateUsingTags", EditConditionHides))
	FGameplayTagContainer DisableTags;

	/**
	 * If component owner has any gameplay tags assigned via their AbilitySystemComponent, this will be toggled to an enabled state using GlobalToggleParams
	 * @warning This overrides DisableTags!
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=HitReact, meta=(EditCondition="bToggleStateUsingTags", EditConditionHides))
	FGameplayTagContainer EnableTags;

	/** Global physics interpolation for toggling the system on and off */
	UPROPERTY(Transient, BlueprintReadOnly, Category=HitReact)
	FHitReactPhysicsStateSimple State;
};

/**
 * Subsequent impulse scalar to apply to a bone after the first impulse when hit multiple times
 */
USTRUCT(BlueprintType)
struct PROCHITREACT_API FHitReactSubsequentImpulse
{
	GENERATED_BODY()

	FHitReactSubsequentImpulse()
		: ElapsedTime(0.f)
		, ImpulseScalar(1.f)
	{}

	FHitReactSubsequentImpulse(float InElapsedTime, float InImpulseScalar)
		: ElapsedTime(InElapsedTime)
		, ImpulseScalar(InImpulseScalar)
	{}

	/** Subsequent impulse scalar will be applied if LastHitReactTime hasn't exceeded this time */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=HitReact, meta=(UIMin="0", ClampMin="0", Delta="0.05", ForceUnits="s"))
	float ElapsedTime;

	/** Scalar to apply to the impulse if ElapsedTime has not been exceeded */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=HitReact, meta=(UIMin="0", ClampMin="0", Delta="0.05", ForceUnits="x"))
	float ImpulseScalar;
};

/**
 * Bone-specific override params defined in a Profile
 */
USTRUCT(BlueprintType)
struct PROCHITREACT_API FHitReactBoneOverride
{
	GENERATED_BODY()

	FHitReactBoneOverride()
		: bIncludeSelf(true)
		, bDisablePhysics(false)
		, BlendWeightScalar(1.f)
	{}

	/** If false, exclude the bone itself and apply these overrides only to bones below */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Bones)
	bool bIncludeSelf;
	
	/**
	 * If true, disable physics on this bone
	 * This will prevent inheriting physics from parent bones
	 * If any active profile has this set to true, physics will be disabled on this bone
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Bones)
	bool bDisablePhysics;

	/** Scale the weight provided to this bone */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Bones, meta=(UIMin="0", ClampMin="0", UIMax="1", ClampMax="1", EditCondition="!bDisablePhysics", EditConditionHides, ForceUnits="x"))
	float BlendWeightScalar;
};

/**
 * Input params for applying a hit reaction
 */
USTRUCT(BlueprintType)
struct PROCHITREACT_API FHitReactInputParams
{
	GENERATED_BODY()

	FHitReactInputParams()
		: Profile(nullptr)
		, SimulatedBoneName(NAME_None)
	    , bIncludeSelf(true)
	{}

	FHitReactInputParams(const TSoftObjectPtr<UHitReactProfile>& InProfileToUse, const FName& InBoneName, bool bInIncludeSelf)
		: Profile(InProfileToUse)
		, SimulatedBoneName(InBoneName)
		, bIncludeSelf(bInIncludeSelf)
	{}

	/** Profile to use when applying the hit react */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=HitReact)
	TSoftObjectPtr<UHitReactProfile> Profile;

	/** Optional additional BoneData to provide for the profile to append */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=HitReact)
	TSoftObjectPtr<UHitReactBoneData> BoneData;

	/**
	 * Bone to apply the hit reaction to -- this bone gets simulated
	 * Note that the simulated bone must have a physics body assigned in the physics asset
	 *
	 * This bone will also receive the impulse if ImpulseBoneName is None
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=HitReact)
	FName SimulatedBoneName;

	/**
	 * Optional bone to apply the impulse to
	 * This differs from the bone that is HitReacted, as the impulse bone is the bone that will receive the impulse
	 * And the HitReact bone is the bone that will be simulated
	 *
	 * If None, the impulse will be applied to the simulated bone instead
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=HitReact)
	FName ImpulseBoneName;

	/** If false, exclude the simulated bone itself and only simulate bones below it */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=HitReact)
	bool bIncludeSelf;

	bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
	{
		Ar << Profile;
		Ar << BoneData;
		Ar << SimulatedBoneName;
		Ar << ImpulseBoneName;
		Ar << bIncludeSelf;
		return !Ar.IsError();
	}

	operator bool() const { return IsValidToApply(); }
	bool IsValidToApply() const { return !Profile.IsNull() && !SimulatedBoneName.IsNone(); }
	
	const FName& GetImpulseBoneName() const
	{
		return ImpulseBoneName.IsNone() ? SimulatedBoneName : ImpulseBoneName;
	}
};

template<>
struct TStructOpsTypeTraits<FHitReactInputParams> : public TStructOpsTypeTraitsBase2<FHitReactInputParams>
{
	enum
	{
		WithNetSerializer = true
	};
};