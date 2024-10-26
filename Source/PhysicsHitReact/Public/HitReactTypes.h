// Copyright (c) Jared Taylor. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AlphaInterp.h"
#include "HitReactTypes.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogHitReact, Log, All);

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
 * Bone-specific parameters for hit reactions
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
 * Bone-specific parameters for hit reactions
 * Used to override default values on a per-bone basis
 */
USTRUCT(BlueprintType)
struct PHYSICSHITREACT_API FHitReactBoneParamsOverride : public FHitReactBoneParams
{
	GENERATED_BODY()
	
	FHitReactBoneParamsOverride(bool bInIncludeSelf = true, bool bInDisablePhysics = false, float InMinBlendWeight = 0.f, float InMaxBlendWeight = 1.f)
		: FHitReactBoneParams(bInDisablePhysics, InMinBlendWeight, InMaxBlendWeight)
		, bIncludeSelf(bInIncludeSelf)
	{}

	/** If false, exclude the bone itself and only simulate bones below it */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Physics)
	bool bIncludeSelf;
};

USTRUCT(BlueprintType)
struct PHYSICSHITREACT_API FHitReactImpulseScalar
{
	GENERATED_BODY()

	FHitReactImpulseScalar(float InScalar = 1.f, float InMax = 0.f)
		: Scalar(InScalar)
		, Max(InMax)
	{}
		
	/**
	 * Scale the impulse by this amount
	 * @see MaxImpulseTaken will mitigate this value if it would otherwise exceed MaxImpulseTaken
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Physics, meta=(UIMin="0", ClampMin="0"))
	float Scalar;

	/** Maximum impulse that can be applied to this bone (at a single time). 0 to disable. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Physics, meta=(UIMin="0", ClampMin="0"))
	float Max;
};

/**
 * Bone-specific parameters for applying hit reactions
 */
USTRUCT(BlueprintType)
struct PHYSICSHITREACT_API FHitReactBoneApplyParams
{
	GENERATED_BODY()

	FHitReactBoneApplyParams()
		: bReinitializeExistingPhysics(false)
		, PhysicsBlendParams(70.f, 40.f, EInterpFunc::FInterpTo)
		, MinBlendWeight(0.f)
		, MaxBlendWeight(1.f)
		, HoldTime(0.f)
		, Cooldown(0.1f)
		, PhysicalAnimProfile(NAME_None)
		, ConstraintProfile(NAME_None)
	{}

	/**
	 * If true, will reinitialize physics on this bone from 0, causing a snap
	 * This provides more reliable/predictable results, but can be visually jarring in some cases
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Physics)
	bool bReinitializeExistingPhysics;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Physics, meta=(UIMin="0", ClampMin="0", UIMax="1", ClampMax="1", EditCondition="!bDisablePhysics"))
	FInterpParams PhysicsBlendParams;
	
	/** Minimum weight provided to physical animation (0 is disabled, 1 is full) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Physics, meta=(UIMin="0", ClampMin="0", UIMax="1", ClampMax="1", EditCondition="!bDisablePhysics"))
	float MinBlendWeight;

	/** Maximum weight provided to physical animation (0 is disabled, 1 is full) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Physics, meta=(UIMin="0", ClampMin="0", UIMax="1", ClampMax="1", EditCondition="!bDisablePhysics"))
	float MaxBlendWeight;
	
	/** After fully interpolating in, wait this long before interpolating out */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Physics, meta=(UIMin="0", ClampMin="0"))
	float HoldTime;

	/** Delay before applying another impulse to prevent rapid application */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Physics, meta=(UIMin="0", ClampMin="0"))
	float Cooldown;

	/**
	 * Scale the impulse by this amount
	 * @see MaxImpulseTaken will mitigate this value if it would otherwise exceed MaxImpulseTaken
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Physics, meta=(UIMin="0", ClampMin="0"))
	FHitReactImpulseScalar LinearImpulseScalar;

	/**
	 * Scale the impulse by this amount
	 * @see MaxImpulseTaken will mitigate this value if it would otherwise exceed MaxImpulseTaken
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Physics, meta=(UIMin="0", ClampMin="0"))
	FHitReactImpulseScalar AngularImpulseScalar;

	/**
	 * Scale the impulse by this amount
	 * @see MaxImpulseTaken will mitigate this value if it would otherwise exceed MaxImpulseTaken
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Physics, meta=(UIMin="0", ClampMin="0"))
	FHitReactImpulseScalar RadialImpulseScalar;

	/**
	 * Physical animation profile to apply to this bone
	 * Requires a Physical Animation Component to exist on the owning actor
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Physics)
	FName PhysicalAnimProfile;

	/**
	 * Constraint profile to apply to this bone
	 * This is applied to the physics asset on the mesh
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Physics)
	FName ConstraintProfile;
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

	/** Override bone parameters for specific bones when applying HitReact */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=HitReact)
	TMap<FName, FHitReactBoneApplyParams> OverrideBoneApplyParams;

	/**
	 * Override bone parameters for specific bones
	 * Allows clamping of child bone blend weights
	 * Applies to all child bones of the specified bone
	 * 
	 * @warning Order is vitally important here - We apply to all children of the bone, so the parent must be defined first
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=HitReact)
	TMap<FName, FHitReactBoneParamsOverride> OverrideBoneParams;
};
