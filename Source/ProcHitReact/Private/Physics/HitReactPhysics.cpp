// Copyright (c) Jared Taylor


#include "Physics/HitReactPhysics.h"

#include "HitReactProfile.h"
#include "PhysicsEngine/PhysicsAsset.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(HitReactPhysics)


void FHitReactPhysics::HitReact(USkeletalMeshComponent* InMesh, const TObjectPtr<const UHitReactProfile>& InProfile,
	const FName& BoneName, const TArray<FName>& InDisabledBones, const TMap<FName, float>& InBoneWeightScalars)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(FHitReactPhysics::HitReact);

	// Reset physics states
	PhysicsState = {};
	
	// Assign properties
	Mesh = InMesh;
	SimulatedBoneName = BoneName;
	Profile = InProfile;
	DisabledBones = InDisabledBones;
	BoneWeightScalars = InBoneWeightScalars;

	// Activate the physics state
	PhysicsState.Params = Profile->BlendParams;
	PhysicsState.Activate();
}

void FHitReactPhysics::Tick(float DeltaTime)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(FHitReactPhysics::Tick);

	// Reset blend weight request
	RequestedBlendWeight = 0.f;
	MaxBlendWeight = 0.f;

	// Update existing physics states
	if (!PhysicsState.IsActive() || !Profile)
	{
		return;
	}

	// Interpolate the physics state
	PhysicsState.Tick(DeltaTime);
	
	// Determine physics blend weight
	const float StateAlpha = PhysicsState.GetBlendStateAlpha();
	float BlendWeight = 0.f;
	switch (PhysicsState.GetBlendState())
	{
	case EHitReactBlendState::BlendIn:
		BlendWeight = StateAlpha;
		break;
	case EHitReactBlendState::BlendHold:
		BlendWeight = 1.f;
		break;
	case EHitReactBlendState::BlendOut:
		BlendWeight = 1.f - StateAlpha;
		break;
	default: break;
	}

	// Clamp the blend weight
	MaxBlendWeight = Profile->MaxBlendWeight;
	RequestedBlendWeight = FMath::Min<float>(BlendWeight, MaxBlendWeight);
}

bool FHitReactPhysics::IsActive() const
{
	return PhysicsState.IsActive();
}

bool FHitReactPhysics::HasCompleted() const
{
	return PhysicsState.HasCompleted();
}
