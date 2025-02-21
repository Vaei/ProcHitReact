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

protected:
	static const FHitReactPhysicsBlend* GetPhysicsBlend(const UHitReact* HitReact, const USkeletalMeshComponent* Mesh, const FName& BoneName);

public:
	UFUNCTION(BlueprintCallable, Category=HitReact)
	static void FinalizeMeshPhysics(USkeletalMeshComponent* Mesh);

	static FBodyInstance* ChooseBodyInstance(USkeletalMeshComponent* Mesh, const FName& BoneName, const FHitReactGlobals& Globals);
	
	static bool AccumulateBlendWeight(const USkeletalMeshComponent* Mesh, const FHitReactPhysicsBlend& Physics, float BlendWeight, float Alpha);
	
	static bool AccumulateBlendWeightBelow(USkeletalMeshComponent* Mesh, const FName& BoneName, float BlendWeight, bool bIncludeSelf = true);

	static bool SetBlendWeightBelow(USkeletalMeshComponent* Mesh, const FName& BoneName, float BlendWeight, bool bIncludeSelf = true, bool bSkipCustomPhysicsType = false);

	UFUNCTION(BlueprintPure, Category=HitReact)
	static float GetBoneBlendWeight(const USkeletalMeshComponent* Mesh, const FName& BoneName);
	
	UFUNCTION(BlueprintPure, Category=HitReact)
	static float GetPhysicsBlendAlpha(const UHitReact* HitReact, const USkeletalMeshComponent* Mesh, const FName& BoneName);

	UFUNCTION(BlueprintPure, Category=HitReact)
	static EHitReactBlendState GetPhysicsBlendState(const UHitReact* HitReact, const USkeletalMeshComponent* Mesh, const FName& BoneName);
};