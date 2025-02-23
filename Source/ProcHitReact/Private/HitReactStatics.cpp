// Copyright (c) Jared Taylor


#include "HitReactStatics.h"

#include "HitReact.h"
#include "PhysicsEngine/PhysicsAsset.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/SkeletalMesh.h"

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

FName UHitReactStatics::GetBoneName(const USkeletalMeshComponent* Mesh, const FBodyInstance* BI)
{
	return Mesh->GetBoneName(BI->InstanceBoneIndex);
}

int32 UHitReactStatics::ForEach(USkeletalMeshComponent* Mesh, FName BoneName, bool bIncludeSelf, const TFunctionRef<void(FBodyInstance*)>& Func)
{
	return Mesh->ForEachBodyBelow(BoneName, bIncludeSelf, false, Func);
}

void UHitReactStatics::FinalizeMeshPhysics(USkeletalMeshComponent* Mesh)
{
	if (Mesh->IsSimulatingPhysics())
	{
		Mesh->SetRootBodyIndex(Mesh->RootBodyData.BodyIndex);	//Update the root body data cache in case animation has moved root body relative to root joint
	}
	
	Mesh->bBlendPhysics = false;

	UpdateEndPhysicsTickRegisteredState(Mesh);
	UpdateClothTickRegisteredState(Mesh);
}

bool UHitReactStatics::AccumulateBlendWeight(const USkeletalMeshComponent* Mesh, const FHitReactPhysics& Physics, float BlendWeight, float Alpha)
{
	const FName& BoneName = Physics.SimulatedBoneName;
	FBodyInstance* BI = Mesh->GetBodyInstance(BoneName);
	if (!BI)
	{
		return false;
	}

	return SetBlendWeight(Mesh, Physics, BI->PhysicsBlendWeight + BlendWeight, Alpha);
}

bool UHitReactStatics::SetBlendWeight(const USkeletalMeshComponent* Mesh, const FHitReactPhysics& Physics,
	float BlendWeight, float Alpha)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UHitReactStatics::SetBlendWeight);
	
	const FName& BoneName = Physics.SimulatedBoneName;
	FBodyInstance* BI = Mesh->GetBodyInstance(BoneName);
	if (!BI)
	{
		return false;
	}

	// Clamp the blend weight
	BI->PhysicsBlendWeight = FMath::Clamp(BlendWeight, 0.f, Physics.MaxBlendWeight);

	// Scale the blend weight by alpha
	BI->PhysicsBlendWeight *= Alpha;
	
	// Apply thresholds
	if (FMath::IsNearlyEqual(BI->PhysicsBlendWeight, Physics.MaxBlendWeight))
	{
		BI->PhysicsBlendWeight = Physics.MaxBlendWeight;
	}
	else if (FMath::IsNearlyZero(BI->PhysicsBlendWeight))
	{
		BI->PhysicsBlendWeight = 0.f;
	}

	// Set simulate physics if necessary
	const bool bWantsSim = BI->PhysicsBlendWeight > 0.f;
	if (bWantsSim != BI->bSimulatePhysics)
	{
		BI->SetInstanceSimulatePhysics(bWantsSim, false, true);
	}

	return true;
}

float UHitReactStatics::GetBoneBlendWeight(const USkeletalMeshComponent* Mesh, const FName& BoneName)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UHitReactStatics::GetBoneBlendWeight);

	UPhysicsAsset* const PhysicsAsset = Mesh->GetPhysicsAsset();
	if (!PhysicsAsset)
	{
		return 0.f;
	}

	FBodyInstance* const BI = Mesh->GetBodyInstance(BoneName);
	return BI ? BI->PhysicsBlendWeight : 0.f;
}
