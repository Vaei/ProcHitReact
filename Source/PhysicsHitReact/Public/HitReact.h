// Copyright (c) Jared Taylor. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "HitReactImpulseParams.h"
#include "HitReactPhysicsState.h"
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
		: SimulatedBoneName(NAME_None)
		, CachedBoneIndex(INDEX_NONE)
		, bCachedBoneExists(false)
		, bHasCachedBoneExists(false)
		, bCachedIncludeSelf(false)
		, Mesh(nullptr)
		, PhysicalAnimation(nullptr)
		, CachedBoneParams(nullptr)
		, CachedProfile(nullptr)
		, bCollisionEnabledChanged(false)
		, DefaultCollisionEnabled(ECollisionEnabled::NoCollision)
	{}

	/** Interpolation state handling for hit reactions -- Supports blend in, hold, and blend out */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Physics)
	FHitReactPhysicsState PhysicsState;

	/** Bone to simulate physics on */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category=Physics)
	FName SimulatedBoneName;

	/**
	 * Used to sort hit reacts so the child bones are simulated last,
	 * i.e. they overwrite the blend weight set by their parents calling SetAllBodiesBelowPhysicsBlendWeight, etc.
	 */
	UPROPERTY()
	int32 CachedBoneIndex;

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
	 * @param SimulatedBoneName - Bone that will be simulated
	 * @param ImpulseBoneName - Bone to apply the impulse to
	 * @param bIncludeSelf - If false, exclude the BoneName and only simulate bones below it
	 * @param Profile - Profile to use when applying the hit react
	 * @param BoneParams - Bone-specific application parameters to use
	 * @param ImpulseParams - Impulse parameters to use when applying the hit react
	 * @param WorldSpaceParams - World space parameters to use when applying the hit react
	 * @param ImpulseScalar - Scalar to apply to all impulses
	 * @return True if the hit reaction was applied
	 */
	
	bool HitReact(USkeletalMeshComponent* InMesh
		, UPhysicalAnimationComponent* InPhysicalAnimation
		, const FName& SimulatedBoneName
		, const FName& ImpulseBoneName
		, bool bIncludeSelf
		, const FHitReactProfile* Profile
		, const FHitReactBoneApplyParams* BoneParams
		, const FHitReactImpulseParams& ImpulseParams
		, const FHitReactImpulseWorldParams& WorldSpaceParams
		, float ImpulseScalar);

	/** @return True if completed */
	bool Tick(float GlobalScalar, float DeltaTime);

	void SetAllBodiesBelowPhysicsBlendWeight(float PhysicsBlendWeight) const;
};

