// Copyright (c) Jared Taylor


#include "HitReactStatics.h"

#include "Params/HitReactParams.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(HitReactStatics)

bool UHitReactStatics::DoAnyPhysicsBodiesHaveWeight(const USkeletalMeshComponent* Mesh)
{
	for (const FBodyInstance* Body : Mesh->Bodies)
	{
		if (Body && Body->PhysicsBlendWeight > 0.f)
		{
			return true;
		}
	}
	return false;
}

bool UHitReactStatics::ShouldBlendPhysicsBones(const USkeletalMeshComponent* Mesh)
{
	return (Mesh->Bodies.Num() > 0) &&
		(CollisionEnabledHasPhysics(Mesh->GetCollisionEnabled())) &&
		(Mesh->bBlendPhysics || DoAnyPhysicsBodiesHaveWeight(Mesh));
}

bool UHitReactStatics::ShouldRunEndPhysicsTick(const USkeletalMeshComponent* Mesh)
{
	return (Mesh->bEnablePhysicsOnDedicatedServer || !Mesh->IsNetMode(NM_DedicatedServer)) && // Early out if we are on a dedicated server and not running physics.
		((Mesh->IsSimulatingPhysics() && Mesh->RigidBodyIsAwake()) || ShouldBlendPhysicsBones(Mesh));
}

bool UHitReactStatics::ShouldRunClothTick(const USkeletalMeshComponent* Mesh)
{
	if (Mesh->bDisableClothSimulation)
	{
		return false;
	}

	if (Mesh->CanSimulateClothing())
	{
		return true;
	}

	return false;
}

void UHitReactStatics::UpdateEndPhysicsTickRegisteredState(USkeletalMeshComponent* Mesh)
{
	Mesh->RegisterEndPhysicsTick(Mesh->PrimaryComponentTick.IsTickFunctionRegistered() && ShouldRunEndPhysicsTick(Mesh));
}

void UHitReactStatics::UpdateClothTickRegisteredState(USkeletalMeshComponent* Mesh)
{
	Mesh->RegisterClothTick(Mesh->PrimaryComponentTick.IsTickFunctionRegistered() && ShouldRunClothTick(Mesh));
}

void UHitReactStatics::FinalizeMeshPhysicsForCurrentFrame(USkeletalMeshComponent* Mesh)
{
	if (Mesh->IsSimulatingPhysics())
	{
		Mesh->SetRootBodyIndex(Mesh->RootBodyData.BodyIndex);	//Update the root body data cache in case animation has moved root body relative to root joint
	}
	
	Mesh->bBlendPhysics = false;

	UpdateEndPhysicsTickRegisteredState(Mesh);
	UpdateClothTickRegisteredState(Mesh);
}

bool UHitReactStatics::AccumulateBlendWeightBelow(USkeletalMeshComponent* Mesh, const FName& BoneName, float BlendWeight,
	const TMap<FName, FHitReactBoneParamsOverride>* OverrideBoneParams, bool bIncludeSelf)
{
	UPhysicsAsset* const PhysicsAsset = Mesh->GetPhysicsAsset();
	if (!PhysicsAsset)
	{
		return false;
	}

	if (const FHitReactBoneParamsOverride* Override = OverrideBoneParams ? OverrideBoneParams->Find(BoneName) : nullptr)
	{
		bIncludeSelf = Override->bIncludeSelf;
	}

	bool bWantsSim = false;
	const int32 NumBodiesFound = Mesh->ForEachBodyBelow(BoneName, bIncludeSelf, false, [BlendWeight, &bWantsSim](FBodyInstance* BI)
	{
		BI->PhysicsBlendWeight = FMath::Clamp(BI->PhysicsBlendWeight + BlendWeight, 0.f, 1.f);
		if (FMath::IsNearlyEqual(BI->PhysicsBlendWeight, 1.f))
		{
			BI->PhysicsBlendWeight = 1.f;
		}
		else if (FMath::IsNearlyZero(BI->PhysicsBlendWeight))
		{
			BI->PhysicsBlendWeight = 0.f;
		}

		bWantsSim |= BI->PhysicsBlendWeight > 0.f;
		if (bWantsSim != BI->bSimulatePhysics)
		{
			BI->SetInstanceSimulatePhysics(bWantsSim, false, true);
		}
	});

	return NumBodiesFound > 0;
}

float UHitReactStatics::GetBlendWeight(const USkeletalMeshComponent* Mesh, const FName& BoneName)
{
	UPhysicsAsset* const PhysicsAsset = Mesh->GetPhysicsAsset();
	if (!PhysicsAsset)
	{
		return 0.f;
	}

	FBodyInstance* const BI = Mesh->GetBodyInstance(BoneName);
	return BI ? BI->PhysicsBlendWeight : 0.f;
}
