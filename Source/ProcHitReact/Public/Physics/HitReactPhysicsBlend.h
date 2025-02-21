// Copyright (c) Jared Taylor

#pragma once

#include "CoreMinimal.h"
#include "HitReactPhysicsState.h"
#include "HitReactPhysicsBlend.generated.h"

/**
 * Process hit reactions on a single bone
 * This is the core system that handles impulse application, physics blend weights, and interpolation
 */
USTRUCT(BlueprintType)
struct PROCHITREACT_API FHitReactPhysicsBlend
{
	GENERATED_BODY()

	FHitReactPhysicsBlend()
		: RequestedBlendWeight(0.f)
		, MaxBlendWeight(0.f)
	{}

public:
	/** Interpolation state handling for hit reactions -- Supports blend in, hold, and blend out */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Physics)
	TArray<FHitReactPhysicsStateWeighted> PhysicsStates;

	/** Bone to simulate physics on */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category=Physics)
	FName SimulatedBoneName;

public:
	UPROPERTY()
	TObjectPtr<USkeletalMeshComponent> Mesh;

	UPROPERTY()
	float RequestedBlendWeight;

	UPROPERTY()
	float MaxBlendWeight;

public:
	bool HitReact(USkeletalMeshComponent* InMesh, const UHitReactProfile* Profile, const FName& BoneName, float Alpha);

	void Tick(float DeltaTime);

	/** The last added physics state is always the current state */
	FHitReactPhysicsStateWeighted* GetWeightedPhysicsState() { return PhysicsStates.Num() > 0 ? &PhysicsStates.Last() : nullptr; }

	/** The last added physics state is always the current state */
	FHitReactPhysicsState* GetPhysicsState() { return PhysicsStates.Num() > 0 ? &PhysicsStates.Last().State : nullptr; }
	const FHitReactPhysicsState* GetPhysicsState() const { return PhysicsStates.Num() > 0 ? &PhysicsStates.Last().State : nullptr; }

	bool IsActive() const;
	bool HasCompleted() const;
};
