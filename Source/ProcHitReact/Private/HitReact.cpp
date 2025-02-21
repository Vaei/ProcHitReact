// Copyright (c) Jared Taylor. All Rights Reserved.


#include "HitReact.h"

#include "PhysicsEngine/PhysicalAnimationComponent.h"
#include "PhysicsEngine/PhysicsAsset.h"
#include "HitReactTypes.h"
#include "Components/SkeletalMeshComponent.h"
#include "Chaos/ChaosEngineInterface.h"
#include "DrawDebugHelpers.h"

#if !UE_BUILD_SHIPPING
#include "Logging/MessageLog.h"
#endif

#include UE_INLINE_GENERATED_CPP_BY_NAME(HitReact)

namespace FHitReactCVars
{
#if UE_ENABLE_DEBUG_DRAWING
	static int32 DrawHitReact = 0;
	FAutoConsoleVariableRef CVarDrawHitReact(
		TEXT("p.HitReact.Draw"),
		DrawHitReact,
		TEXT("Optionally draw debug shapes when hit reactions are applied. Green for Linear. Yellow for Angular. Blue for Radial.\n")
		TEXT("0: Disable, 1: Enable"),
		ECVF_Default);

	static float DrawHitReactRadialScale = 0.1f;
	FAutoConsoleVariableRef CVarDrawHitReactRadialScale(
		TEXT("p.HitReact.Draw.RadialScale"),
		DrawHitReactRadialScale,
		TEXT("Scale the radial radius when drawing the shape.\n"),
		ECVF_Default);
#endif
}

FHitReact::FHitReact()
	: SimulatedBoneName(NAME_None)
    , CachedBoneIndex(INDEX_NONE)
    , bCachedBoneExists(false)
    , bHasCachedBoneExists(false)
    , bCachedIncludeSelf(false)
    , NumImpulseApplications(0)
    , Mesh(nullptr)
    , PhysicalAnimation(nullptr)
    , CachedBoneParams(nullptr)
    , CachedProfile(nullptr)
    , bCollisionEnabledChanged(false)
    , DefaultCollisionEnabled(ECollisionEnabled::NoCollision)
	, RequestedBlendWeight(0.f)
{}

const TMap<FName, FHitReactBoneParamsOverride>* FHitReact::GetOverrideBoneParams() const
{
	return CachedProfile ? &CachedProfile->OverrideBoneParams : nullptr;
}

bool FHitReact::NeedsCollisionEnabled() const
{
	return Mesh->GetCollisionEnabled() != ECollisionEnabled::QueryAndPhysics && Mesh->GetCollisionEnabled() != ECollisionEnabled::PhysicsOnly;
}

bool FHitReact::CanSimulate() const
{
	TRACE_CPUPROFILER_EVENT_SCOPE(FHitReact::CanSimulate);

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

	// Must have a physics asset
	if (!Mesh->GetPhysicsAsset())
	{
#if !UE_BUILD_SHIPPING
		const FString ErrorString = FString::Printf(TEXT("HitReact: No physics asset available : %s : %s : %s"),
			*Mesh->GetOwner()->GetName(), *Mesh->GetName(), *Mesh->GetSkeletalMeshAsset()->GetName());
		FMessageLog("PIE").Error(FText::FromString(ErrorString));
#endif
		return false;
	}

	// Bone must exist
	const FReferenceSkeleton& RefSkeleton = Mesh->GetSkeletalMeshAsset()->GetRefSkeleton();
	if (RefSkeleton.FindBoneIndex(SimulatedBoneName) == INDEX_NONE)
	{
#if !UE_BUILD_SHIPPING
		const FString ErrorString = FString::Printf(TEXT("HitReact: No bone found %s : %s : %s : %s : %s"),
			*SimulatedBoneName.ToString(), *Mesh->GetOwner()->GetName(),
			*Mesh->GetName(), *Mesh->GetSkeletalMeshAsset()->GetName(), *Mesh->GetPhysicsAsset()->GetName());
		FMessageLog("PIE").Error(FText::FromString(ErrorString));
#endif
		return false;
	}

	// Dev note - We simply trust that UHitReactComponent::HitReact() is the only caller,
	// which gates triggering hit reacts on dedicated servers for us

	return true;
}

void FHitReact::CacheBoneParams(const FName& InSimulatedBoneName)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(FHitReact::CacheBoneParams);

	// Reset cache if bone name has changed
	if (SimulatedBoneName != InSimulatedBoneName)
	{
		bHasCachedBoneExists = false;
		SimulatedBoneName = InSimulatedBoneName;
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
	const int32 BoneIndex = RefSkeleton.FindBoneIndex(SimulatedBoneName);
	if (BoneIndex == INDEX_NONE)
	{
		return;
	}

	bCachedBoneExists = true;
	CachedBoneIndex = BoneIndex;
}

bool FHitReact::HitReact(USkeletalMeshComponent* InMesh, UPhysicalAnimationComponent* InPhysicalAnimation,
	const FName& InSimulatedBoneName, const FName& ImpulseBoneName, bool bIncludeSelf, const FHitReactProfile* Profile,
	const FHitReactBoneApplyParams* BoneParams, const FHitReactImpulseParams& ImpulseParams,
	const FHitReactImpulseWorldParams& WorldSpaceParams, float ImpulseScalar)
{
	PhysicalAnimation = InPhysicalAnimation;
	Mesh = InMesh;
	CacheBoneParams(InSimulatedBoneName);

	CachedProfile = Profile;
	CachedBoneParams = BoneParams;
	PhysicsState.Params = CachedBoneParams->PhysicsBlendParams;

	if (CanSimulate())
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(FHitReact::HitReact::Simulate);

		if (NeedsCollisionEnabled())
		{
			bCollisionEnabledChanged = true;
			DefaultCollisionEnabled = Mesh->GetCollisionEnabled();
			switch (DefaultCollisionEnabled)
			{
			case ECollisionEnabled::NoCollision:
			case ECollisionEnabled::ProbeOnly:
				Mesh->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
				break;
			case ECollisionEnabled::QueryOnly:
			case ECollisionEnabled::QueryAndProbe:
				Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
				break;
			default: break;
			}
		}
		
		// Bone must have a body instance
		if (!Mesh->GetBodyInstance(SimulatedBoneName))  // This fails if collision is disabled, so can't do it in CanSimulate()
		{
#if !UE_BUILD_SHIPPING
			const FString ErrorString = FString::Printf(TEXT("HitReact: No body instance for bone %s : %s : %s : %s : %s"),
				*SimulatedBoneName.ToString(), *Mesh->GetOwner()->GetName(),
				*Mesh->GetName(), *Mesh->GetSkeletalMeshAsset()->GetName(), *Mesh->GetPhysicsAsset()->GetName());
			FMessageLog("PIE").Error(FText::FromString(ErrorString));
#endif
			return false;
		}

		// Reset physics state if desired
		if (CachedBoneParams->bReinitializeExistingPhysics)
		{
			PhysicsState.Reset();
		}

		// Activate physics state if not already active
		if (PhysicsState.TryActivate())
		{
			// Enable physics simulation
			bCachedIncludeSelf = bIncludeSelf;

			// Apply physical anim profile
			if (PhysicalAnimation && !CachedBoneParams->PhysicalAnimProfile.IsNone())
			{
				PhysicalAnimation->ApplyPhysicalAnimationProfileBelow(
					SimulatedBoneName, CachedBoneParams->PhysicalAnimProfile, bCachedIncludeSelf);
			}

			// Apply constraint profile
			if (!CachedBoneParams->ConstraintProfile.IsNone())
			{
				Mesh->SetConstraintProfileForAll(CachedBoneParams->ConstraintProfile);
			}
		}
		else
		{
			// Apply decay if set
			if (CachedBoneParams->DecayExistingPhysics > 0.f)
			{
				// Decay the physics state
				PhysicsState.Decay(CachedBoneParams->DecayExistingPhysics);
			}
		}

		// Retrieve impulse parameters
		const FHitReactLinearImpulseParams& LinearParams = ImpulseParams.LinearImpulse;
		const FHitReactAngularImpulseParams& AngularParams = ImpulseParams.AngularImpulse;
		const FHitReactRadialImpulseParams& RadialParams = ImpulseParams.RadialImpulse;

		// Throttle impulse based on number of applications
		const float ThrottleScalar = GetSubsequentImpulseScalar();
		NumImpulseApplications++;

		// Linear impulse
		if (LinearParams.CanBeApplied())
		{
			// Calculate linear impulse
			const FHitReactImpulseScalar& Scalar = CachedBoneParams->LinearImpulseScalar;
			FVector Linear = LinearParams.GetImpulse(WorldSpaceParams.LinearDirection) * Scalar.Scalar * ImpulseScalar * ThrottleScalar;
			if (Scalar.Max > 0.f)
			{
				Linear = Linear.GetClampedToMaxSize(Scalar.Max);
			}

			// Apply linear impulse
			if (!Linear.IsNearlyZero())
			{
				// Apply impulse to impulse bone if set, otherwise apply to simulated bone
				Mesh->AddImpulse(Linear, ImpulseBoneName, LinearParams.IsVelocityChange());
				
#if UE_ENABLE_DEBUG_DRAWING
				if (FHitReactCVars::DrawHitReact > 0)
				{
					const FVector Start = Mesh->GetSocketLocation(ImpulseBoneName);
					const FVector End = Start + Linear.GetSafeNormal() * 100.f;
					DrawDebugDirectionalArrow(Mesh->GetWorld(), Start, End, 10.f, FColor::Green, false, 1.5f);
				}
#endif
			}
		}

		// Angular impulse
		if (AngularParams.CanBeApplied())
		{
			// Calculate angular impulse
			const FHitReactImpulseScalar& Scalar = CachedBoneParams->AngularImpulseScalar;
			FVector Angular = AngularParams.GetImpulse(WorldSpaceParams.AngularDirection) * Scalar.Scalar * ImpulseScalar * ThrottleScalar;
			if (Scalar.Max > 0.f)
			{
				Angular = Angular.GetClampedToMaxSize(Scalar.Max);
			}

			// Apply Angular impulse
			if (!Angular.IsNearlyZero())
			{
				// Apply impulse to impulse bone if set, otherwise apply to simulated bone
				switch (AngularParams.AngularUnits)
				{
				case EHitReactUnits::Degrees:
					Mesh->AddAngularImpulseInDegrees(Angular, ImpulseBoneName, AngularParams.IsVelocityChange());
					break;
				case EHitReactUnits::Radians:
					Mesh->AddAngularImpulseInRadians(Angular, ImpulseBoneName, AngularParams.IsVelocityChange());
					break;
				}

#if UE_ENABLE_DEBUG_DRAWING
				if (FHitReactCVars::DrawHitReact > 0)
				{
					const FVector Start = Mesh->GetSocketLocation(ImpulseBoneName);
					const FVector End = Start + Angular.GetSafeNormal() * 100.f;
					DrawDebugDirectionalArrow(Mesh->GetWorld(), Start, End, 10.f, FColor::Yellow, false, 1.5f);
				}
#endif
			}
		}

		// Radial impulse
		if (RadialParams.CanBeApplied())
		{
			// Calculate Radial impulse
			const FHitReactImpulseScalar& Scalar = CachedBoneParams->RadialImpulseScalar;
			float Radial = RadialParams.Impulse * Scalar.Scalar * ImpulseScalar * ThrottleScalar;
			if (Scalar.Max > 0.f)
			{
				Radial = FMath::Min<float>(Radial, Scalar.Max);
			}

			// Apply Radial impulse
			if (!FMath::IsNearlyZero(Radial))
			{
				const ERadialImpulseFalloff Falloff = RadialParams.Falloff == EHitReactFalloff::Linear ? RIF_Linear : RIF_Constant;
				
				// Convert falloff
				Mesh->AddRadialImpulse(WorldSpaceParams.RadialLocation, RadialParams.Radius, RadialParams.Impulse,
					Falloff, RadialParams.IsVelocityChange());

#if UE_ENABLE_DEBUG_DRAWING
				if (FHitReactCVars::DrawHitReact > 0)
				{
					const FVector Center = WorldSpaceParams.RadialLocation;
					const float Radius = FHitReactCVars::DrawHitReactRadialScale * RadialParams.Radius;
					DrawDebugSphere(Mesh->GetWorld(), Center, Radius, 8, FColor::Blue, false, 1.5f);
				}
#endif
			}
		}
		
		return true;
	}

	return false;
}

EHitReactTickRequest FHitReact::Tick(float GlobalScalar, float DeltaTime)
{
	// Reset blend weight request
	RequestedBlendWeight = 0.f;

	// Must have valid mesh
	if (!Mesh)
	{
		PhysicsState.Reset();
		return EHitReactTickRequest::Invalid;
	}
	
	// Update physics state
	if (PhysicsState.IsActive())
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(FHitReact::Tick);

		// Interpolate physics state
		PhysicsState.Tick(DeltaTime);

		// Handle completion
		if (PhysicsState.HasCompleted())
		{
			// Finalize physics simulation
			return EHitReactTickRequest::Completed;
		}
		
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

		BlendWeight = FMath::Min<float>(BlendWeight, CachedBoneParams->MaxBlendWeight * GlobalScalar);
		RequestedBlendWeight = BlendWeight;
		return EHitReactTickRequest::Continue;
	}

	return PhysicsState.HasCompleted() ? EHitReactTickRequest::Completed : EHitReactTickRequest::Continue;
}

float FHitReact::GetSubsequentImpulseScalar() const
{
	// If no subsequent impulse scalars are set, don't throttle
	if (CachedBoneParams->SubsequentImpulseScalars.Num() == 0)
	{
		return 1.f;
	}

	// If we have a scalar for this application, use it
	if (CachedBoneParams->SubsequentImpulseScalars.IsValidIndex(NumImpulseApplications))
	{
		return CachedBoneParams->SubsequentImpulseScalars[NumImpulseApplications];
	}

	// Otherwise, use the last scalar
	return CachedBoneParams->SubsequentImpulseScalars.Last();
}
