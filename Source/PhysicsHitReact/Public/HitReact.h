// Copyright (c) Jared Taylor. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AlphaInterp.h"
#include "HitReactImpulseParams.h"
#include "HitReactTypes.h"
#include "HitReact.generated.h"

class UPhysicalAnimationComponent;

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
		, bCachedIncludeSelf(false)
		, Mesh(nullptr)
		, PhysicalAnimation(nullptr)
		, CachedBoneParams(nullptr)
		, CachedProfile(nullptr)
		, LastHitReactTime(-1.f)
		, HoldTimeRemaining(0.f)
		, bCollisionEnabledChanged(false)
		, DefaultCollisionEnabled(ECollisionEnabled::NoCollision)
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
	bool bCachedIncludeSelf;

	UPROPERTY()
	USkeletalMeshComponent* Mesh;
	
	UPROPERTY()
	UPhysicalAnimationComponent* PhysicalAnimation;

	/** Cached bone params to avoid unnecessary TMap lookups */
	const FHitReactBoneApplyParams* CachedBoneParams;
	const FHitReactProfile* CachedProfile;

	/** Last time a hit reaction was applied - prevent rapid application causing poor results */
	UPROPERTY()
	float LastHitReactTime;

	UPROPERTY()
	float HoldTimeRemaining;

	UPROPERTY()
	bool bCollisionEnabledChanged;

	UPROPERTY()
	TEnumAsByte<ECollisionEnabled::Type> DefaultCollisionEnabled;

	bool NeedsCollisionEnabled() const;
	
	/** @return True if we have valid data to simulate */
	bool CanSimulate() const;

	/** Cache bone existence to avoid redundant checks */
	void CacheBoneParams(const FName& BoneName);

	/**
	 * Apply a hit reaction to the bone
	 * @param InMesh - Mesh to apply the hit reaction to
	 * @param InPhysicalAnimation - Physical animation component to set animation profile (optional)
	 * @param InBoneName - Bone to apply the hit reaction to
	 * @param bIncludeSelf - If false, exclude the BoneName and only simulate bones below it
	 * @param Profile - Profile to use when applying the hit react
	 * @param ApplyParams - Bone-specific application properties to use
	 * @param ImpulseParams - Impulse parameters to use when applying the hit react
	 * @return True if the hit reaction was applied
	 */
	
	bool HitReact(USkeletalMeshComponent* InMesh
		, UPhysicalAnimationComponent* InPhysicalAnimation
		, const FName& InBoneName
		, bool bIncludeSelf
		, const FHitReactProfile* Profile
		, const FHitReactBoneApplyParams* ApplyParams
		, const FHitReactImpulseParams& ImpulseParams
		, const FHitReactImpulseWorldParams& WorldSpaceParams);

	/** @return True if completed */
	bool Update(float GlobalScalar, float DeltaTime);

	void SetAllBodiesBelowPhysicsBlendWeight(float PhysicsBlendWeight) const;
};

