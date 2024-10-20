// Copyright (c) Jared Taylor. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AlphaInterp.h"
#include "HitReactTypes.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogHitReact, Log, All);

UENUM(BlueprintType)
enum class EHitReactUnits : uint8
{
	Degrees,
	Radians
};

UENUM(BlueprintType)
enum class EHitReactToggleState : uint8
{
	Disabled,
	Disabling,
	Enabling,
	Enabled
};

UENUM(BlueprintType)
enum class EInterpDirection : uint8
{
	Forward			UMETA(ToolTip="Physics system is interpolating in"),
	Hold			UMETA(ToolTip="Physics system is waiting before interpolating out, if delay is enabled"),
	Reverse			UMETA(ToolTip="Physics system is interpolating out"),
};

/**
 * Bone-specific properties for hit reactions
 */
USTRUCT(BlueprintType)
struct PHYSICSHITREACT_API FHitReactBoneParams
{
	GENERATED_BODY()
	
	FHitReactBoneParams(bool bInDisablePhysics = false, float InMinBlendWeight = 0.f, float InMaxBlendWeight = 1.f)
		: bDisablePhysics(bInDisablePhysics)
		, MinBlendWeight(InMinBlendWeight)
		, MaxBlendWeight(InMaxBlendWeight)
	{}

	/**
	 * If true, disable physics on this bone
	 * This will prevent inheriting physics from parent bones, it is not the same as setting MaxBlendWeight to 0
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Physics)
	bool bDisablePhysics;
	
	/** Minimum weight provided to physical animation (0 is disabled, 1 is full) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Physics, meta=(UIMin="0", ClampMin="0", UIMax="1", ClampMax="1", EditCondition="!bDisablePhysics"))
	float MinBlendWeight;

	/** Maximum weight provided to physical animation (0 is disabled, 1 is full) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Physics, meta=(UIMin="0", ClampMin="0", UIMax="1", ClampMax="1", EditCondition="!bDisablePhysics"))
	float MaxBlendWeight;

};

/**
 * Bone-specific properties for applying hit reactions
 */
USTRUCT(BlueprintType)
struct PHYSICSHITREACT_API FHitReactBoneApplyParams : public FHitReactBoneParams
{
	GENERATED_BODY()

	FHitReactBoneApplyParams(float InImpulseScalar = 1.f, float InMaxImpulse = 0.f, float InMinWaitDelay = 0.15f)
		: ImpulseScalar(InImpulseScalar)
		, MaxImpulse(InMaxImpulse)
		, HoldTime(0.f)
		, Cooldown(InMinWaitDelay)
	{}
	
	/**
	 * Scale the impulse by this amount
	 * @see MaxImpulseTaken will mitigate this value if it would otherwise exceed MaxImpulseTaken
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Physics, meta=(UIMin="0", ClampMin="0"))
	float ImpulseScalar;

	/** Maximum impulse that can be applied to this bone (at a single time). 0 to disable. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Physics, meta=(UIMin="0", ClampMin="0"))
	float MaxImpulse;

	/** After fully interpolating in, wait this long before interpolating out */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Physics, meta=(UIMin="0", ClampMin="0"))
	float HoldTime;

	/** Delay before applying another impulse to prevent rapid application */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Physics, meta=(UIMin="0", ClampMin="0"))
	float Cooldown;
};

/**
 * A collection of bone parameters for hit reactions
 */
USTRUCT(BlueprintType)
struct PHYSICSHITREACT_API FHitReactProfile
{
	GENERATED_BODY()

	/** Default bone params to use when applying HitReact if none are overridden */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=HitReact)
	FHitReactBoneApplyParams DefaultBoneApplyParams;

	/** Override bone properties for specific bones when applying HitReact */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=HitReact)
	TMap<FName, FHitReactBoneApplyParams> OverrideBoneApplyParams;

	/**
	 * Override bone properties for specific bones
	 * Allows clamping of child bone blend weights
	 * Applies to all child bones of the specified bone
	 * 
	 * @warning Order is vitally important here - We apply to all children of the bone, so the parent must be defined first
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=HitReact)
	TMap<FName, FHitReactBoneParams> OverrideBoneParams;
};

/**
 * Process hit reactions on a single bone
 * This is the core system that handles impulse application, physics blend weights, and interpolation
 */
USTRUCT(BlueprintType)
struct PHYSICSHITREACT_API FHitReact
{
	GENERATED_BODY()

	FHitReact()
		: BoneName(NAME_None)
		, InterpDirection(EInterpDirection::Forward)
		, bCachedBoneExists(false)
		, bHasCachedBoneExists(false)
		, Mesh(nullptr)
		, CachedBoneParams(nullptr)
		, CachedProfile(nullptr)
		, LastHitReactTime(-1.f)
		, HoldTimeRemaining(0.f)
	{}

	/** Alpha interpolation handler and properties */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Physics)
	FAlphaInterp PhysicsState;

	/** Bone to simulate physics on */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category=Physics)
	FName BoneName;

	/**
	 * Current interpolation direction. First we interpolate in, then back out
	 */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category=Physics)
	EInterpDirection InterpDirection;

	/** Cache to avoid redundant bone existence checks */
	UPROPERTY()
	bool bCachedBoneExists;

	UPROPERTY()
	bool bHasCachedBoneExists;

	UPROPERTY()
	USkeletalMeshComponent* Mesh;

	/** Cached bone params to avoid unnecessary TMap lookups */
	const FHitReactBoneApplyParams* CachedBoneParams;
	const FHitReactProfile* CachedProfile;

	/** Last time a hit reaction was applied - prevent rapid application causing poor results */
	UPROPERTY()
	float LastHitReactTime;

	UPROPERTY()
	float HoldTimeRemaining;

	UPROPERTY()
	TEnumAsByte<ECollisionEnabled::Type> DefaultCollisionEnabled;

	/** @return True if we have valid data to simulate */
	bool CanSimulate() const;

	/** Cache bone existence to avoid redundant checks */
	void CacheBoneParams(const FName& BoneName);

	/**
	 * Apply a hit reaction to the bone
	 * @param InMesh - Mesh to apply the hit reaction to
	 * @param InBoneName - Bone to apply the hit reaction to
	 * @param bOnlyBonesBelow - Exclude the BoneName and only simulate bones below it
	 * @param Profile - Profile to use when applying the hit react
	 * @param ApplyParams - Bone-specific application properties to use
	 * @param Direction - Direction of the hit
	 * @param Magnitude - Magnitude of the hit
	 * @param Units - Units to use for the magnitude
	 * @param bFactorMass - Whether to apply mass to the hit
	 * @return True if the hit reaction was applied
	 */
	bool HitReact(USkeletalMeshComponent* InMesh
		, const FName& InBoneName
		, bool bOnlyBonesBelow
		, const FHitReactProfile* Profile
		, const FHitReactBoneApplyParams* ApplyParams
		, const FVector& Direction
		, const float Magnitude
		, EHitReactUnits Units
		, bool bFactorMass = false);

	/** @return True if completed */
	bool Update(float GlobalScalar, float DeltaTime);

	void SetAllBodiesBelowPhysicsBlendWeight(float PhysicsBlendWeight, bool bSkipCustomPhysicsType = false, bool bIncludeSelf = true) const;
};
