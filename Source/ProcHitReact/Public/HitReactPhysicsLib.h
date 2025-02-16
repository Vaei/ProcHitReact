// Copyright (c) Jared Taylor. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

/**
 * These functions are reproduced from USkeletalMeshComponent usually due to them being private or protected
 * This lets us call them only when we finish our physics simulation at the end of the tick
 * Its not just a performance optimization, it is also necessary to prevent physics from being blended incorrectly
 */
struct PROCHITREACT_API FHitReactPhysicsLib
{
	static bool DoAnyPhysicsBodiesHaveWeight(const USkeletalMeshComponent* Mesh);

	static bool ShouldBlendPhysicsBones(const USkeletalMeshComponent* Mesh);

	static bool ShouldRunEndPhysicsTick(const USkeletalMeshComponent* Mesh);

	static void UpdateEndPhysicsTickRegisteredState(USkeletalMeshComponent* Mesh);

	static bool ShouldRunClothTick(const USkeletalMeshComponent* Mesh);

	static void UpdateClothTickRegisteredState(USkeletalMeshComponent* Mesh);

	static bool SetAllBodiesBelowSimulatePhysics(USkeletalMeshComponent* Mesh, const FName& InBoneName, bool bNewSimulate, bool bIncludeSelf = true);

	static bool SetAllBodiesBelowPhysicsBlendWeight(USkeletalMeshComponent* Mesh, const FName& InBoneName,
		float PhysicsBlendWeight, bool bSkipCustomPhysicsType = false, bool bIncludeSelf = true);

	static void SetAllBodiesSimulatePhysics(USkeletalMeshComponent* Mesh, bool bNewSimulate);

	static bool SetAllBodiesPhysicsBlendWeight(USkeletalMeshComponent* Mesh, float PhysicsBlendWeight,
		bool bSkipCustomPhysicsType = false);
	
	static void FinalizeMeshPhysics(USkeletalMeshComponent* Mesh);
};
