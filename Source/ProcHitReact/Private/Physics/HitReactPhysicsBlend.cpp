// Copyright (c) Jared Taylor


#include "Physics/HitReactPhysicsBlend.h"

#include "HitReactProfile.h"
#include "PhysicsEngine/PhysicsAsset.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(HitReactPhysicsBlend)


bool FHitReactPhysicsBlend::HitReact(USkeletalMeshComponent* InMesh, const UHitReactProfile* Profile, const FName& BoneName, float Alpha)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(FPhysicsBlend::HitReact);

	// Validate blend params
	if (!FHitReactPhysicsState::CanActivate(Profile->BlendParams))
	{
		return false;
	}

	// If our current state is active, apply a new physics state at 0 weight
	if (GetPhysicsState() && GetPhysicsState()->IsActive())
	{
		// Deactivate the current state
		GetWeightedPhysicsState()->bActive = false;

		// Apply a decay to the current state to 'knock' it back in time
		GetPhysicsState()->Decay();

		// Cache the current position (alpha)
		const float CurrentAlpha = GetPhysicsState()->GetElapsedAlpha();

		// Add a new state that becomes the active state, this state has 0 weight and will blend in
		PhysicsStates.Add(FHitReactPhysicsStateWeighted(Profile, 0.f));

		// Apply the same alpha to the new state -- we don't call Activate() at all
		GetPhysicsState()->SetElapsedAlpha(CurrentAlpha);  // Our newly added state becomes the active state

		// Apply the same decay to the new state
		GetPhysicsState()->Decay();
	}
	else
	{
		// Remove any existing but inactive physics states
		PhysicsStates.Reset();
		
		// Activate physics state if not already active
		Mesh = InMesh;
		SimulatedBoneName = BoneName;

		// Add a new state that becomes the active state
		PhysicsStates.Add(FHitReactPhysicsStateWeighted(Profile, 1.f));
		GetPhysicsState()->Activate();
	}

	return true;
}

void FHitReactPhysicsBlend::Tick(float DeltaTime)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(FPhysicsBlend::Tick);

	// Reset blend weight request
	RequestedBlendWeight = 0.f;
	MaxBlendWeight = 0.f;

	// Grab the weights from each physics state
	struct FBlendWeight
	{
		FBlendWeight(float InWeight, float InMaxWeight, float InScalar)
			: Weight(InWeight)
			, MaxWeight(InMaxWeight)
			, Scalar(InScalar)
		{}
		
		float Weight;
		float MaxWeight;
		float Scalar;
	};
	
	TArray<FBlendWeight> Weights;
	float HighestMaxBlendWeight = 0.f;

	// Update existing physics states
	int32 NumStates = PhysicsStates.Num();
	PhysicsStates.RemoveAll([NumStates, &DeltaTime, &Weights, &HighestMaxBlendWeight](FHitReactPhysicsStateWeighted& PhysicsState)
	{
		// Remove inactive states
		FHitReactPhysicsState& State = PhysicsState.State;
		if (!PhysicsState.bActive && !State.IsActive())
		{
			// We don't remove the active PhysicsState even if it's State has completed
			return true;
		}

		float CurrentWeight = 1.f;
		if (NumStates > 1)
		{
			// Retrieve params for weight transition
			const FHitReactBlendParams& Params = PhysicsState.bActive ?
				PhysicsState.Profile->BlendParams.TransitionIn : PhysicsState.Profile->BlendParams.TransitionOut;

			// Update the weight
			if (PhysicsState.bActive)
			{
				PhysicsState.Weight = FMath::Min<float>(PhysicsState.Weight + DeltaTime / Params.BlendTime, 1.f);
				if (FMath::IsNearlyEqual(PhysicsState.Weight, 1.f))
				{
					PhysicsState.Weight = 1.f;
				}
			}
			else
			{
				PhysicsState.Weight = FMath::Max<float>(PhysicsState.Weight - DeltaTime / Params.BlendTime, 0.f);
				if (FMath::IsNearlyZero(PhysicsState.Weight))
				{
					PhysicsState.Weight = 0.f;
				}
			}
			
			// Smooth the weight
			CurrentWeight = Params.Ease(PhysicsState.Weight);
		}

		// Interpolate the physics state
		State.Tick(DeltaTime);
		
		// Determine physics blend weight
		const float StateAlpha = State.GetBlendStateAlpha();
		float BlendWeight = 0.f;
		switch (State.GetBlendState())
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
		BlendWeight = FMath::Min<float>(BlendWeight, PhysicsState.Profile->MaxBlendWeight);

		// Output the weight
		Weights.Add({ BlendWeight, PhysicsState.Profile->MaxBlendWeight, CurrentWeight });

		// Update the highest max blend weight
		HighestMaxBlendWeight = FMath::Max<float>(HighestMaxBlendWeight, PhysicsState.Profile->MaxBlendWeight);

		// Remove if weight is 0, but never remove the active state
		if (!PhysicsState.bActive && (PhysicsState.Weight <= 0.f || State.HasCompleted()))
		{
			return true;
		}
		return false;
	});

	// Compute the requested & max blend weight
	for (const FBlendWeight& Weight : Weights)
	{
		RequestedBlendWeight += Weight.Weight * Weight.Scalar;
		MaxBlendWeight += Weight.MaxWeight * Weight.Scalar;
	}

	// Clamp the blend weights
	RequestedBlendWeight = FMath::Min<float>(RequestedBlendWeight, HighestMaxBlendWeight);
	MaxBlendWeight = FMath::Min<float>(MaxBlendWeight, HighestMaxBlendWeight);
}

bool FHitReactPhysicsBlend::IsActive() const
{
	for (const FHitReactPhysicsStateWeighted& PhysicsState : PhysicsStates)
	{
		if (PhysicsState.State.IsActive())
		{
			return true;
		}
	}
	return false;
}

bool FHitReactPhysicsBlend::HasCompleted() const
{
	for (const FHitReactPhysicsStateWeighted& PhysicsState : PhysicsStates)
	{
		if (!PhysicsState.State.HasCompleted())
		{
			return false;
		}
	}
	return true;
}
