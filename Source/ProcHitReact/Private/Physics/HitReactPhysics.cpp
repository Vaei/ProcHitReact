// Copyright (c) Jared Taylor


#include "Physics/HitReactPhysics.h"

#include "HitReactProfile.h"
#include "PhysicsEngine/PhysicsAsset.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(HitReactPhysics)


bool FHitReactPhysics::HitReact(USkeletalMeshComponent* InMesh, const UHitReactProfile* InProfile, const FName& BoneName, float InMaxBlendWeightForBone)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(FHitReactPhysics::HitReact);

	// Validate blend params
	if (!FHitReactPhysicsState::CanActivate(InProfile->BlendParams))
	{
		return false;
	}

	// Reset physics states
	PhysicsState = {};
	
	// Activate physics state if not already active
	Mesh = InMesh;
	SimulatedBoneName = BoneName;
	Profile = InProfile;
	MaxBlendWeightForBone = InMaxBlendWeightForBone;

	// Activate the physics state
	PhysicsState.Params = Profile->BlendParams;
	PhysicsState.Activate();

	return true;
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
	MaxBlendWeight = Profile->MaxBlendWeight * MaxBlendWeightForBone;
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
