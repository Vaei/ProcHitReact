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

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category=Physics)
	float MaxBlendWeightForBone;

public:
	UPROPERTY()
	TObjectPtr<USkeletalMeshComponent> Mesh;

	UPROPERTY()
	float RequestedBlendWeight;

	UPROPERTY()
	float MaxBlendWeight;

public:
	bool HitReact(USkeletalMeshComponent* InMesh, const UHitReactProfile* Profile, const FName& BoneName, float MaxBlendWeightForBone);

	void Tick(float DeltaTime);

	bool IsActive() const;
	
	bool HasCompleted() const;
};
