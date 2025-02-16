// Copyright (c) Jared Taylor. All Rights Reserved.


#include "HitReactPhysicsLib.h"

#include "PhysicsEngine/BodySetup.h"

bool FHitReactPhysicsLib::DoAnyPhysicsBodiesHaveWeight(const USkeletalMeshComponent* Mesh)
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

bool FHitReactPhysicsLib::ShouldBlendPhysicsBones(const USkeletalMeshComponent* Mesh)
{
	return (Mesh->Bodies.Num() > 0) &&
			(CollisionEnabledHasPhysics(Mesh->GetCollisionEnabled())) &&
			(Mesh->bBlendPhysics || DoAnyPhysicsBodiesHaveWeight(Mesh));
}

bool FHitReactPhysicsLib::ShouldRunEndPhysicsTick(const USkeletalMeshComponent* Mesh)
{
	return (Mesh->bEnablePhysicsOnDedicatedServer || !Mesh->IsNetMode(NM_DedicatedServer)) && // Early out if we are on a dedicated server and not running physics.
		((Mesh->IsSimulatingPhysics() && Mesh->RigidBodyIsAwake()) || ShouldBlendPhysicsBones(Mesh));
}

void FHitReactPhysicsLib::UpdateEndPhysicsTickRegisteredState(USkeletalMeshComponent* Mesh)
{
	Mesh->RegisterEndPhysicsTick(Mesh->PrimaryComponentTick.IsTickFunctionRegistered() && ShouldRunEndPhysicsTick(Mesh));
}

bool FHitReactPhysicsLib::ShouldRunClothTick(const USkeletalMeshComponent* Mesh)
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

void FHitReactPhysicsLib::UpdateClothTickRegisteredState(USkeletalMeshComponent* Mesh)
{
	Mesh->RegisterClothTick(Mesh->PrimaryComponentTick.IsTickFunctionRegistered() && ShouldRunClothTick(Mesh));
}

bool FHitReactPhysicsLib::SetAllBodiesBelowSimulatePhysics(USkeletalMeshComponent* Mesh, const FName& InBoneName,
	bool bNewSimulate, bool bIncludeSelf)
{
	static constexpr bool bSkipCustomPhysicsType = false;
	int32 NumBodiesFound = Mesh->ForEachBodyBelow(InBoneName, bIncludeSelf, bSkipCustomPhysicsType, [bNewSimulate](FBodyInstance* BI)
	{
		BI->SetInstanceSimulatePhysics(bNewSimulate);
	});

	// We remove this and call FinalizeMeshPhysics manually after the hit reacts all finish ticking
	
	// if (NumBodiesFound)
	// {
	// 	if (IsSimulatingPhysics())
	// 	{
	// 		SetRootBodyIndex(RootBodyData.BodyIndex);	//Update the root body data cache in case animation has moved root body relative to root joint
	// 	}
	//
	// 	UpdateEndPhysicsTickRegisteredState();
	// 	UpdateClothTickRegisteredState();
	// }

	return NumBodiesFound > 0;
}

bool FHitReactPhysicsLib::SetAllBodiesBelowPhysicsBlendWeight(USkeletalMeshComponent* Mesh, const FName& InBoneName,
	float PhysicsBlendWeight, bool bSkipCustomPhysicsType, bool bIncludeSelf)
{
	int32 NumBodiesFound = Mesh->ForEachBodyBelow(InBoneName, bIncludeSelf, bSkipCustomPhysicsType, [PhysicsBlendWeight](FBodyInstance* BI)
	{
		BI->PhysicsBlendWeight = PhysicsBlendWeight;
	});

	// We remove this and call FinalizeMeshPhysics manually after the hit reacts all finish ticking

	// if (NumBodiesFound)
	// {
	// 	bBlendPhysics = false;
	//
	// 	UpdateEndPhysicsTickRegisteredState();
	// 	UpdateClothTickRegisteredState();
	// }
	
	return NumBodiesFound > 0;
}

void FHitReactPhysicsLib::SetAllBodiesSimulatePhysics(USkeletalMeshComponent* Mesh, bool bNewSimulate)
{
	for(int32 i=0; i<Mesh->Bodies.Num(); i++)
	{
		Mesh->Bodies[i]->SetInstanceSimulatePhysics(bNewSimulate);
	}

	Mesh->SetRootBodyIndex(Mesh->RootBodyData.BodyIndex);	//Update the root body data cache in case animation has moved root body relative to root joint
}

bool FHitReactPhysicsLib::SetAllBodiesPhysicsBlendWeight(USkeletalMeshComponent* Mesh, float PhysicsBlendWeight,
	bool bSkipCustomPhysicsType)
{
	UPhysicsAsset* const PhysicsAsset = Mesh->GetPhysicsAsset();
	if (!PhysicsAsset)
	{
		return false;
	}

	// Fix / Unfix bones
	bool bModified = false;
	for(int32 i=0; i<Mesh->Bodies.Num(); i++)
	{
		FBodyInstance*	BodyInst	= Mesh->Bodies[i];
		if (!ensure(BodyInst))
		{
			continue;
		}
		UBodySetup*	BodyInstSetup	= BodyInst->GetBodySetup();

		// Set fixed on any bodies with bAlwaysFullAnimWeight set to true
		if(BodyInstSetup && (!bSkipCustomPhysicsType || BodyInstSetup->PhysicsType == PhysType_Default) )
		{
			BodyInst->PhysicsBlendWeight = PhysicsBlendWeight;
			bModified = true;
		}
	}

	return bModified;
}

void FHitReactPhysicsLib::FinalizeMeshPhysics(USkeletalMeshComponent* Mesh)
{
	Mesh->bBlendPhysics = false;
	
	UpdateEndPhysicsTickRegisteredState(Mesh);
	UpdateClothTickRegisteredState(Mesh);
}
