// Copyright (c) Jared Taylor

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "HitReactStatics.generated.h"

struct FHitReactBoneParamsOverride;
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
	UFUNCTION(BlueprintCallable, Category=HitReact)
	static void FinalizeMeshPhysicsForCurrentFrame(USkeletalMeshComponent* Mesh);
	
	static bool AccumulateBlendWeightBelow(USkeletalMeshComponent* Mesh, const FName& BoneName, float BlendWeight,
		const TMap<FName, FHitReactBoneParamsOverride>* OverrideBoneParams, bool bIncludeSelf = true);

	UFUNCTION(BlueprintPure, Category=HitReact)
	static float GetBlendWeight(const USkeletalMeshComponent* Mesh, const FName& BoneName);
};