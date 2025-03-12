// Copyright (c) Jared Taylor

#pragma once

#include "CoreMinimal.h"
#include "HitReactPhysicsState.h"
#include "HitReactPhysics.generated.h"

/**
 * Process hit reactions on a single bone
 * This is the core system that handles impulse application, physics blend weights, and interpolation
 */
USTRUCT(BlueprintType)
struct PROCHITREACT_API FHitReactPhysics
{
	GENERATED_BODY()

	FHitReactPhysics()
		: Profile(nullptr)
		, MaxBlendWeightForBone(1.f)
		, Mesh(nullptr)
		, RequestedBlendWeight(0.f)
		, MaxBlendWeight(0.f)
		, UniqueId(0)
	{}

public:
	/** Interpolation state handling for hit reactions -- Supports blend in, hold, and blend out */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category=HitReact)
	FHitReactPhysicsState PhysicsState;
	
	/** Bone to simulate physics on */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category=Physics)
	FName SimulatedBoneName;

	/** Profile that this blend is using */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category=Physics)
	const UHitReactProfile* Profile;

	/** Maximum blend weight for this bone */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category=Physics)
	float MaxBlendWeightForBone;

public:
	/** Mesh to apply the hit reaction to */
	UPROPERTY()
	TObjectPtr<USkeletalMeshComponent> Mesh;

	/** Requested blend weight for this bone to apply on UHitReact::TickComponent() */
	UPROPERTY()
	float RequestedBlendWeight;

	/** Maximum blend weight for this bone */
	UPROPERTY()
	float MaxBlendWeight;

	/** Used for comparison */
	UPROPERTY()
	uint64 UniqueId;

public:
	/** Apply a hit reaction to the bone */
	bool HitReact(USkeletalMeshComponent* InMesh, const UHitReactProfile* Profile, const FName& BoneName, float MaxBlendWeightForBone);

	/** Tick the hit reaction */
	void Tick(float DeltaTime);

	/** @return True if the hit reaction is active */
	bool IsActive() const;

	/** @return True if the hit reaction has completed */
	bool HasCompleted() const;

	bool operator==(const FHitReactPhysics& Other) const
	{
		return UniqueId == Other.UniqueId;
	}

	bool operator!=(const FHitReactPhysics& Other) const
	{
		return !(*this == Other);
	}
};
