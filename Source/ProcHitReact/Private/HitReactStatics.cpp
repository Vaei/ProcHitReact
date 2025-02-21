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

const FHitReactPhysicsBlend* UHitReactStatics::GetPhysicsBlend(const UHitReact* HitReact, const USkeletalMeshComponent* Mesh,
	const FName& BoneName)
{
	if (HitReact && Mesh)
	{
		const TMap<FName, FHitReactPhysicsBlend>& PhysicsBlends = HitReact->GetPhysicsBlends();
		return PhysicsBlends.Find(BoneName);
	}
	return nullptr;
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

FBodyInstance* UHitReactStatics::ChooseBodyInstance(USkeletalMeshComponent* Mesh, const FName& BoneName,
	const FHitReactGlobals& Globals)
{
	// @TODO obsolete, nuke it
	const int32 BoneIndex = Mesh->GetSkeletalMeshAsset()->GetRefSkeleton().FindBoneIndex(BoneName);
	
	TArray<int32> BodyIndices;
	Mesh->GetPhysicsAsset()->GetBodyIndicesBelow(BodyIndices, BoneName, Mesh->GetSkeletalMeshAsset());

	if (BodyIndices.Num() > 0)
	{
		// If we blacklist spine_02, it will ignore spine_01 because there is no body, and then chooses pelvis
		// But pelvis is blacklisted, so it chooses thigh_l, which is unacceptable, so we need to look for children instead

		// BodyIndices has lowest index first, so we want to choose the closest index, then go one higher if its the same
		const int32* NearestIndex = BodyIndices.FindByPredicate([BoneIndex](int32 Index)
		{
			return Index > BoneIndex;
		});

		if (NearestIndex)
		{
			const FName NearestBoneName = Mesh->GetBoneName(BodyIndices[*NearestIndex]);
			if (Globals.BlacklistedBones.Contains(NearestBoneName))
			{
				return ChooseBodyInstance(Mesh, NearestBoneName, Globals);
			}
			return Mesh->Bodies[BodyIndices[*NearestIndex]];
		}
	}
	return nullptr;
}

bool UHitReactStatics::AccumulateBlendWeight(const USkeletalMeshComponent* Mesh, const FHitReactPhysicsBlend& Physics, float BlendWeight, float Alpha)
{
	const FName& BoneName = Physics.SimulatedBoneName;
	FBodyInstance* BI = Mesh->GetBodyInstance(BoneName);
	if (!BI)
	{
		return false;
	}

	// Clamp the blend weight
	BI->PhysicsBlendWeight = FMath::Clamp(BI->PhysicsBlendWeight + BlendWeight, 0.f, Physics.MaxBlendWeight);

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

bool UHitReactStatics::AccumulateBlendWeightBelow(USkeletalMeshComponent* Mesh, const FName& BoneName, float BlendWeight,
	bool bIncludeSelf)
{
	UPhysicsAsset* const PhysicsAsset = Mesh->GetPhysicsAsset();
	if (!PhysicsAsset)
	{
		return false;
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

bool UHitReactStatics::SetBlendWeightBelow(USkeletalMeshComponent* Mesh, const FName& BoneName, float BlendWeight,
	bool bIncludeSelf, bool bSkipCustomPhysicsType)
{
	// @TODO use or removed
	UPhysicsAsset* const PhysicsAsset = Mesh->GetPhysicsAsset();
	if (!PhysicsAsset)
	{
		return false;
	}

	const bool bWantsSim = BlendWeight > 0.f;
	bool bModified = false;
	Mesh->ForEachBodyBelow(BoneName, bIncludeSelf, bSkipCustomPhysicsType, [BlendWeight, bWantsSim, &bModified](FBodyInstance* BI)
	{
		if (!FMath::IsNearlyEqual(BI->PhysicsBlendWeight, BlendWeight))
		{
			BI->PhysicsBlendWeight = BlendWeight;
			bModified = true;
		}
		
		if (bWantsSim != BI->bSimulatePhysics)
		{
			BI->SetInstanceSimulatePhysics(bWantsSim, false, true);
			bModified = true;
		}
	});

	return bModified;
}

float UHitReactStatics::GetBoneBlendWeight(const USkeletalMeshComponent* Mesh, const FName& BoneName)
{
	UPhysicsAsset* const PhysicsAsset = Mesh->GetPhysicsAsset();
	if (!PhysicsAsset)
	{
		return 0.f;
	}

	FBodyInstance* const BI = Mesh->GetBodyInstance(BoneName);
	return BI ? BI->PhysicsBlendWeight : 0.f;
}

float UHitReactStatics::GetPhysicsBlendAlpha(const UHitReact* HitReact, const USkeletalMeshComponent* Mesh,
	const FName& BoneName)
{
	if (const FHitReactPhysicsBlend* Blend = GetPhysicsBlend(HitReact, Mesh, BoneName))
	{
		return Blend->GetPhysicsState()->GetBlendStateAlpha();
	}
	return -1.f;
}

EHitReactBlendState UHitReactStatics::GetPhysicsBlendState(const UHitReact* HitReact,
	const USkeletalMeshComponent* Mesh, const FName& BoneName)
{
	if (const FHitReactPhysicsBlend* Blend = GetPhysicsBlend(HitReact, Mesh, BoneName))
	{
		return Blend->GetPhysicsState()->GetBlendState();
	}
	return EHitReactBlendState::Unknown;
}
