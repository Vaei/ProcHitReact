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
		: SimulatedBoneName(NAME_None)
		, Profile(nullptr)
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
	TObjectPtr<const UHitReactProfile> Profile;

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
	/** Bones that descend from and may include SimulatedBoneName that do not simulate physics */
	UPROPERTY()
	TArray<FName> DisabledBones;

	/** Bones that descent from and may include SimulatedBoneWeight that have a specified MaxBoneWeight */
	UPROPERTY()
	TMap<FName, float> BoneWeightScalars = {};

public:
	/** Apply a hit reaction to the bone */
	void HitReact(USkeletalMeshComponent* InMesh, const TObjectPtr<const UHitReactProfile>& InProfile, const FName& BoneName,
		const TArray<FName>& InDisabledBones, const TMap<FName, float>& InBoneWeightScalars);

	/** Tick the hit reaction */
	void Tick(float DeltaTime);

	/** @return True if the hit reaction is active */
	bool IsActive() const;

	/** @return True if the hit reaction has completed */
	bool HasCompleted() const;
};
