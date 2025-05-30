﻿// Copyright (c) Jared Taylor

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "HitReactStatics.generated.h"

struct FHitReactGlobals;
struct FHitReactBone;
struct FHitReactPhysics;
enum class EHitReactBlendState : uint8;
class UHitReact;
class USkeletalMeshComponent;
struct FBodyInstance;
/**
 * Function library for HitReact with common utility functions
 */
UCLASS()
class PROCHITREACT_API UHitReactStatics : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

protected:
	static bool DoAnyPhysicsBodiesHaveWeight(const USkeletalMeshComponent* Mesh);
	static bool ShouldBlendPhysicsBones(const USkeletalMeshComponent* Mesh);
	static bool ShouldRunEndPhysicsTick(const USkeletalMeshComponent* Mesh);
	static bool ShouldRunClothTick(const USkeletalMeshComponent* Mesh);
	static void UpdateEndPhysicsTickRegisteredState(USkeletalMeshComponent* Mesh);
	static void UpdateClothTickRegisteredState(USkeletalMeshComponent* Mesh);

public:
	/** Retrieve the bone name based on the FBodyInstance::InstanceBoneIndex */
	static FName GetBoneName(const USkeletalMeshComponent* Mesh, const FBodyInstance* BI);
	
	/** Convenience wrapper for Mesh->ForEachBodyBelow */
	static int32 ForEach(USkeletalMeshComponent* Mesh, FName BoneName, bool bIncludeSelf, const TFunctionRef<bool(FBodyInstance*)>& Func);

public:
	/** Finalize the physics state of the mesh, must be called after modifying blend weights or simulate physics state */
	static void FinalizeMeshPhysics(USkeletalMeshComponent* Mesh);

	/** Accumulate the blend weight for the given bone */
	static bool AccumulateBlendWeight(const USkeletalMeshComponent* Mesh, const FName& BoneName, float BlendWeight, float ClampBlendWeight, float Alpha);

	/** Set the blend weight for the given bone */
	static bool SetBlendWeight(const USkeletalMeshComponent* Mesh, const FName& BoneName, float BlendWeight, float ClampBlendWeight = 1.f, float Alpha = 1.f);

	/** @return Blend Weight for the given bone ( FBodyInstance::PhysicsBlendWeight ) */
	UFUNCTION(BlueprintPure, Category=HitReact)
	static float GetBoneBlendWeight(const USkeletalMeshComponent* Mesh, const FName& BoneName);
};