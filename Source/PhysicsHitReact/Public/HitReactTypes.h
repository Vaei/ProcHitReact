// Copyright (c) Jared Taylor. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
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
 * Bone-specific properties for hit reactions
 */
USTRUCT(BlueprintType)
struct PHYSICSHITREACT_API FHitReactBoneParams
{
	GENERATED_BODY()
	
	FHitReactBoneParams(bool bInIncludeSelf = true, bool bInDisablePhysics = false, float InMinBlendWeight = 0.f, float InMaxBlendWeight = 1.f)
		: bIncludeSelf(bInIncludeSelf)
		, bDisablePhysics(bInDisablePhysics)
		, MinBlendWeight(InMinBlendWeight)
		, MaxBlendWeight(InMaxBlendWeight)
	{}

	/** If false, exclude the bone itself and only simulate bones below it */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Physics)
	bool bIncludeSelf;

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
struct PHYSICSHITREACT_API FHitReactBoneApplyParams
{
	GENERATED_BODY()

	FHitReactBoneApplyParams()
		: bIncludeSelf(true)
		, MinBlendWeight(0.f)
		, MaxBlendWeight(1.f)
		, bReinitializeExistingPhysics(false)
		, LinearImpulseScalar(1.f)
		, MaxLinearImpulse(0.f)
		, AngularImpulseScalar(1.f)
		, MaxAngularImpulse(0.f)
		, RadialImpulseScalar(1.f)
		, MaxRadialImpulse(0.f)
		, HoldTime(0.f)
		, Cooldown(0.1f)
		, PhysicalAnimProfile(NAME_None)
		, ConstraintProfile(NAME_None)
	{}

	/** If false, exclude the bone itself and only simulate bones below it */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Physics)
	bool bIncludeSelf;

	/** Minimum weight provided to physical animation (0 is disabled, 1 is full) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Physics, meta=(UIMin="0", ClampMin="0", UIMax="1", ClampMax="1", EditCondition="!bDisablePhysics"))
	float MinBlendWeight;

	/** Maximum weight provided to physical animation (0 is disabled, 1 is full) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Physics, meta=(UIMin="0", ClampMin="0", UIMax="1", ClampMax="1", EditCondition="!bDisablePhysics"))
	float MaxBlendWeight;

	/**
	 * If true, will reinitialize physics on this bone from 0, causing a snap
	 * This provides more reliable/predictable results, but can be visually jarring in some cases
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Physics)
	bool bReinitializeExistingPhysics;
	
	/**
	 * Scale the impulse by this amount
	 * @see MaxImpulseTaken will mitigate this value if it would otherwise exceed MaxImpulseTaken
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Physics, meta=(UIMin="0", ClampMin="0"))
	float LinearImpulseScalar;

	/** Maximum impulse that can be applied to this bone (at a single time). 0 to disable. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Physics, meta=(UIMin="0", ClampMin="0"))
	float MaxLinearImpulse;
	
	/**
	 * Scale the impulse by this amount
	 * @see MaxImpulseTaken will mitigate this value if it would otherwise exceed MaxImpulseTaken
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Physics, meta=(UIMin="0", ClampMin="0"))
	float AngularImpulseScalar;

	/** Maximum impulse that can be applied to this bone (at a single time). 0 to disable. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Physics, meta=(UIMin="0", ClampMin="0"))
	float MaxAngularImpulse;

	/**
	 * Scale the impulse by this amount
	 * @see MaxImpulseTaken will mitigate this value if it would otherwise exceed MaxImpulseTaken
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Physics, meta=(UIMin="0", ClampMin="0"))
	float RadialImpulseScalar;

	/** Maximum impulse that can be applied to this bone (at a single time). 0 to disable. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Physics, meta=(UIMin="0", ClampMin="0"))
	float MaxRadialImpulse;

	/** After fully interpolating in, wait this long before interpolating out */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Physics, meta=(UIMin="0", ClampMin="0"))
	float HoldTime;

	/** Delay before applying another impulse to prevent rapid application */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Physics, meta=(UIMin="0", ClampMin="0"))
	float Cooldown;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Physics)
	FName PhysicalAnimProfile;

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
