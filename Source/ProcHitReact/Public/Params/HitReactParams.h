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
 * Global settings for the HitReact system
 */
USTRUCT(BlueprintType)
struct PROCHITREACT_API FHitReactGlobals
{
	GENERATED_BODY()

	FHitReactGlobals()
		: Cooldown(0.f)
		, BlacklistedBones({ "root", "pelvis" })
	{}

	/**
	 * Hit reacts will not trigger until Cooldown has lapsed
	 * This affects every HitReact regardless of profile
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=HitReact, meta=(UIMin="0", ClampMin="0", UIMax="1", Delta="0.01", ForceUnits="s"))
	float Cooldown;

	/**
	 * These bones cannot be simulated
	 * Attempting to simulate these bones will not necessarily fail,
	 * because the system will attempt to simulate the parent bone
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=HitReact)
	TArray<FName> BlacklistedBones;
};

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
 * Limits for the number of bones that can be simulated for hit reacts to improve performance and visuals
 */
USTRUCT(BlueprintType)
struct PROCHITREACT_API FHitReactBoneLimits
{
	GENERATED_BODY()

	FHitReactBoneLimits()
		: bLimitSimulatedBones(true)
		, MaxSimulatedBones(16)
		, MaxHitReactHandling(EHitReactMaxHandling::RemoveOldest)
	{}	
	/** Whether to limit the amount of active hit reacts for this component */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=HitReact)
	bool bLimitSimulatedBones;
	
	/**
	 * Limit the amount of active hit reacts for this component, which can improve both visuals and performance
	 * Hit Reacts are applied per bone
	 * @warning A single hit react can apply a count identical to the bone count
	 * @note Setting this to a low number e.g. 5, can be a stylistic choice when using 'RemoveOldest', it simplifies the resulting hit reacts considerably
	 * @note PreventNewest is not recommended, it doesn't look good and requires a much higher limit
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=HitReact, meta=(UIMin="1", ClampMin="1", UIMax="64", Delta="1", EditCondition="bLimitSimulatedBones", EditConditionHides))
	int32 MaxSimulatedBones;

	/** How to handle hit reacts when the limit is reached */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=HitReact, meta=(EditCondition="bLimitMaxHitReacts", EditConditionHides))
	EHitReactMaxHandling MaxHitReactHandling;
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
		, MaxBlendWeight(1.f)
	{}

	/** If false, exclude the bone itself and apply these overrides only to bones below */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Physics)
	bool bIncludeSelf;
	
	/**
	 * If true, disable physics on this bone
	 * This will prevent inheriting physics from parent bones, it is not the same as setting MaxBlendWeight to 0
	 * If any active profile has this set to true, physics will be disabled on this bone
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Physics)
	bool bDisablePhysics;

	/** Maximum weight provided to physical animation (0 is disabled, 1 is full) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Physics, meta=(UIMin="0", ClampMin="0", UIMax="1", ClampMax="1", EditCondition="!bDisablePhysics", EditConditionHides, ForceUnits="%"))
	float MaxBlendWeight;
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