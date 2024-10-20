// Copyright (c) Jared Taylor. All Rights Reserved.


#include "HitReactTypes.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(HitReactTypes)

DEFINE_LOG_CATEGORY(LogHitReact);

bool FHitReact::CanSimulate() const
{
	// Must have valid mesh and owner
	if (!Mesh || !IsValid(Mesh->GetOwner()))
	{
		return false;
	}

	// Must have an actual mesh assigned
	if (!Mesh->GetSkeletalMeshAsset())
	{
		return false;
	}

	// Bone must exist
	const FReferenceSkeleton& RefSkeleton = Mesh->GetSkeletalMeshAsset()->GetRefSkeleton();
	if (RefSkeleton.FindBoneIndex(BoneName) == INDEX_NONE)
	{
		return false;
	}

	// Bone must have a body instance
	if (!Mesh->GetBodyInstance(BoneName))
	{
#if !UE_BUILD_SHIPPING
		const FString ErrorString = FString::Printf(TEXT("HitReact: No body instance for bone %s : %s : %s"),
			*BoneName.ToString(), *Mesh->GetOwner()->GetName(), *Mesh->GetName());
		FMessageLog("PIE").Error(FText::FromString(ErrorString));
#endif
		return false;
	}

	// Mesh must have physics collision enabled
	const bool bPhysicsEnabled = Mesh->GetCollisionEnabled() == ECollisionEnabled::QueryAndPhysics || Mesh->GetCollisionEnabled() == ECollisionEnabled::PhysicsOnly;
	if (!bPhysicsEnabled)
	{
#if !UE_BUILD_SHIPPING
		const FString ErrorString = FString::Printf(TEXT("HitReact: Physics collision is disabled on character %s : %s"),
			*Mesh->GetOwner()->GetName(), *Mesh->GetName());
		FMessageLog("PIE").Error(FText::FromString(ErrorString));
#endif
		return false;
	}
	
	// Dev note - We simply trust that UHitReactComponent::HitReact() is the only caller,
	// which gates triggering hit reacts on dedicated servers for us

	return true;
}

void FHitReact::CacheBoneParams(const FName& InBoneName)
{
	// Reset cache if bone name has changed
	if (BoneName != InBoneName)
	{
		bHasCachedBoneExists = false;
		BoneName = InBoneName;
	}
	
	// Already cached
	if (bHasCachedBoneExists)
	{
		return;
	}

	// Reset cache
	bCachedBoneExists = false;

	// Must have valid mesh
	if (!Mesh)
	{
		return;
	}
	
	// Must have an actual mesh assigned
	if (!Mesh->GetSkeletalMeshAsset())
	{
		return;
	}

	// Bone must exist
	const FReferenceSkeleton& RefSkeleton = Mesh->GetSkeletalMeshAsset()->GetRefSkeleton();
	if (RefSkeleton.FindBoneIndex(BoneName) == INDEX_NONE)
	{
		return;
	}

	bCachedBoneExists = true;
}

bool FHitReact::HitReact(USkeletalMeshComponent* InMesh, const FName& InBoneName, bool bOnlyBonesBelow,
	const FHitReactProfile* Profile, const FHitReactBoneApplyParams* ApplyParams,
	const FVector& Direction, const float Magnitude, EHitReactUnits
	Units, bool bFactorMass)
{
	// Throttle hit reacts to prevent rapid application
	if (LastHitReactTime >= 0.f)
    {
        const float TimeSinceLastHitReact = InMesh->GetWorld()->TimeSince(LastHitReactTime);
        if (ApplyParams->Cooldown > TimeSinceLastHitReact)
        {
            return false;
        }
    }
	
	Mesh = InMesh;
	CacheBoneParams(InBoneName);

	CachedProfile = Profile;
	CachedBoneParams = ApplyParams;
	
	if (CanSimulate())
	{
		LastHitReactTime = InMesh->GetWorld()->GetTimeSeconds();

		// Reset physics state prior to reinitialization, if not active
		if (!PhysicsState.IsActive())
		{
			// Interp forward
			InterpDirection = EInterpDirection::Forward;
			
			// Initialize physics state
			PhysicsState.Initialize(0.f);

			// Enable physics simulation
			const bool bIncludeSelf = !bOnlyBonesBelow;
			Mesh->SetAllBodiesBelowSimulatePhysics(BoneName, true, bIncludeSelf);
			Mesh->SetAllBodiesBelowPhysicsBlendWeight(BoneName, CachedBoneParams->MinBlendWeight);
		}

		// Calculate impulse
		FVector Impulse = Direction * Magnitude * CachedBoneParams->ImpulseScalar;
		if (CachedBoneParams->MaxImpulse > 0.f)
		{
			Impulse = Impulse.GetClampedToMaxSize(CachedBoneParams->MaxImpulse);
		}

		// Apply impulse as velocity change if mass is not to be considered
		const bool bVelChange = !bFactorMass;

		// Apply impulse
		switch (Units)
		{
		case EHitReactUnits::Degrees:
			Mesh->AddAngularImpulseInDegrees(Impulse, BoneName, bVelChange);
			break;
		case EHitReactUnits::Radians:
			Mesh->AddAngularImpulseInRadians(Impulse, BoneName, bVelChange);
			break;
		}

		return true;
	}

	return false;
}

bool FHitReact::Update(float GlobalScalar, float DeltaTime)
{
	// Update physics state
	if (PhysicsState.IsActive() && Mesh)
	{
		// Hold if we have a delay set
		if (InterpDirection == EInterpDirection::Hold)
		{
			HoldTimeRemaining -= DeltaTime;
			if (HoldTimeRemaining <= 0.f)
			{
				InterpDirection = EInterpDirection::Reverse;
			}
		}
		else
		{
			// Interpolate physics blend weight
			const float Target = InterpDirection == EInterpDirection::Forward ? 1.f : 0.f;
			PhysicsState.Interpolate(Target, DeltaTime);

			// Handle completion
			if (PhysicsState.HasCompleted())
			{
				// Finalize the value
				PhysicsState.Finalize();
			
				// Toggle direction
				InterpDirection = InterpDirection == EInterpDirection::Forward ? EInterpDirection::Reverse : EInterpDirection::Forward;

				// Delay before reversing if we have a delay set
				if (InterpDirection == EInterpDirection::Reverse && CachedBoneParams && CachedBoneParams->HoldTime > 0.f)
				{
					InterpDirection = EInterpDirection::Hold;
					HoldTimeRemaining = CachedBoneParams->HoldTime;
				}

				// Reset physics state if we have gone back to the start
				if (InterpDirection == EInterpDirection::Forward)
				{
					// Reset physics state
					PhysicsState.Reset();

					// Finalize physics simulation
					Mesh->SetAllBodiesBelowPhysicsBlendWeight(BoneName, 0.f);
					Mesh->SetAllBodiesBelowSimulatePhysics(BoneName, false, true); // @TODO ???

					// Notify caller that we have completed, and can be removed
					return true;
				}
			}
		}

		// Update physics blend weight
		const float TargetBlendWeight = PhysicsState.GetInterpolatedValue() * GlobalScalar;
		const float BlendWeight = FMath::Clamp<float>(TargetBlendWeight, CachedBoneParams->MinBlendWeight, CachedBoneParams->MaxBlendWeight);
		SetAllBodiesBelowPhysicsBlendWeight(BlendWeight);
	}

	// Not completed
	return false;
}

void FHitReact::SetAllBodiesBelowPhysicsBlendWeight(float PhysicsBlendWeight,
	bool bSkipCustomPhysicsType, bool bIncludeSelf) const
{
	Mesh->SetAllBodiesBelowPhysicsBlendWeight(BoneName, PhysicsBlendWeight, bSkipCustomPhysicsType, bIncludeSelf);

	if (CachedProfile && CachedProfile->OverrideBoneParams.Num() > 0)
	{
		for (auto& Params : CachedProfile->OverrideBoneParams)
		{
			const float BlendWeight = FMath::Clamp<float>(PhysicsBlendWeight, Params.Value.MinBlendWeight, Params.Value.MaxBlendWeight);
			Mesh->SetAllBodiesBelowPhysicsBlendWeight(Params.Key, BlendWeight, bSkipCustomPhysicsType, true);
			if (Params.Value.bDisablePhysics)
			{
				Mesh->SetAllBodiesBelowSimulatePhysics(Params.Key, false, true);
			}
		}
	}
}
