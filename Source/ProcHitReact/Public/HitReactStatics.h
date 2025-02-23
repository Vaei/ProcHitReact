// Copyright (c) Jared Taylor

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "HitReactStatics.generated.h"

struct FHitReactGlobals;
struct FHitReactBone;
struct FHitReactPhysicsBlend;
enum class EHitReactBlendState : uint8;
class UHitReact;
class USkeletalMeshComponent;
struct FBodyInstance;
/**
 * 
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
	static FName GetBoneName(const USkeletalMeshComponent* Mesh, const FBodyInstance* BI);
	
	/** Convenience wrapper for Mesh->ForEachBodyBelow */
	static int32 ForEach(USkeletalMeshComponent* Mesh, FName BoneName, bool bIncludeSelf, const TFunctionRef<void(FBodyInstance*)>& Func);

public:
	UFUNCTION(BlueprintCallable, Category=HitReact)
	static void FinalizeMeshPhysics(USkeletalMeshComponent* Mesh);
	
	static bool AccumulateBlendWeight(const USkeletalMeshComponent* Mesh, const FHitReactPhysicsBlend& Physics, float BlendWeight, float Alpha);
	static bool SetBlendWeight(const USkeletalMeshComponent* Mesh, const FHitReactPhysicsBlend& Physics, float BlendWeight, float Alpha = 1.f);

	UFUNCTION(BlueprintPure, Category=HitReact)
	static float GetBoneBlendWeight(const USkeletalMeshComponent* Mesh, const FName& BoneName);
};