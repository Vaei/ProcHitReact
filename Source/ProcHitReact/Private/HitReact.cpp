// Copyright (c) Jared Taylor


#include "HitReact.h"

#include "HitReactProfile.h"
#include "HitReactStatics.h"
#include "Misc/DataValidation.h"
#include "PhysicsEngine/PhysicalAnimationComponent.h"
#include "HAL/IConsoleManager.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "Logging/MessageLog.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"

#if UE_ENABLE_DEBUG_DRAWING
#include "Engine/Engine.h"  // GEngine
#include "DrawDebugHelpers.h"
#endif

#include "Engine/AssetManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(HitReact)

namespace FHitReactCVars
{
#if UE_ENABLE_DEBUG_DRAWING
	static int32 DebugHitReactResult = 0;
	FAutoConsoleVariableRef CVarDebugHitReactResult(
		TEXT("p.HitReact.Debug.Result"),
		DebugHitReactResult,
		TEXT("Draw debug strings when hit reactions are applied or rejected. Does not inform when rejecting due to blacklist or cooldown. 0: Disable, 1: Enable, 2: Enable except for dedicated servers, 3: Enable local player only\n")
		TEXT("0: Disable, 1: Enable, 2: Enable except for dedicated servers, 3: Enable local player only"),
		ECVF_Default);

	static int32 DebugHitReactBlendWeights = 0;
	FAutoConsoleVariableRef CVarDebugHitReactBlendWeights(
		TEXT("p.HitReact.Debug.BlendWeights"),
		DebugHitReactBlendWeights,
		TEXT("Draw debug string showing the value of each currently simulated physics blend. 0: Disable, 1: Enable, 2: Enable except for dedicated servers, 3: Enable local player only\n")
		TEXT("0: Disable, 1: Enable, 2: Enable except for dedicated servers, 3: Enable local player only"),
		ECVF_Default);

	static int32 DebugHitReactNum = 0;
	FAutoConsoleVariableRef CVarDebugHitReactNum(
		TEXT("p.HitReact.Debug.Count"),
		DebugHitReactNum,
		TEXT("Draw debug string showing the number of hit reacts currently in effect. 0: Disable, 1: Enable, 2: Enable except for dedicated servers, 3: Enable local player only\n")
		TEXT("0: Disable, 1: Enable, 2: Enable except for dedicated servers, 3: Enable local player only"),
		ECVF_Default);
#endif

#if !UE_BUILD_SHIPPING
	static int32 HitReactDisabled = 0;
	FAutoConsoleVariableRef CVarHitReactDisabled(
		TEXT("p.HitReact.Disabled"),
		HitReactDisabled,
		TEXT("If true, disable hit react globally.\n")
		TEXT("0: Do nothing, 1: Disable hit react"),
		ECVF_Cheat);

	static int32 DrawHitReact = 0;
	FAutoConsoleVariableRef CVarDrawHitReact(
		TEXT("p.HitReact.Draw"),
		DrawHitReact,
		TEXT("Optionally draw debug shapes when hit reactions are applied. Green for Linear. Yellow for Angular. Blue for Radial.\n")
		TEXT("0: Disable, 1: Enable"),
		ECVF_Default);

	static float DrawHitReactRadialScale = 0.05f;
	FAutoConsoleVariableRef CVarDrawHitReactRadialScale(
		TEXT("p.HitReact.Draw.RadialScale"),
		DrawHitReactRadialScale,
		TEXT("Scale the radial radius when drawing the shape.\n"),
		ECVF_Default);
#endif
}

#define LOCTEXT_NAMESPACE "HitReact"

UHitReact::UHitReact(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, bPhysicalAnimationProfileChanged(false)
	, bConstraintProfileChanged(false)
	, bCollisionEnabledChanged(false)
	, DefaultCollisionEnabled(ECollisionEnabled::NoCollision)
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	PrimaryComponentTick.bAllowTickOnDedicatedServer = false;
	PrimaryComponentTick.TickGroup = TG_PrePhysics;

	bAutoActivate = true;
}

bool UHitReact::HitReact(const FHitReactInputParams& Params, FHitReactImpulseParams Impulse,
	const FHitReactImpulse_WorldParams& World, float ImpulseScalar)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UHitReact::HitReact);

	// Avoid GC issues
	if (!IsValid(GetOwner()))
	{
		return false;
	}

	// Dedicated servers generally don't need cosmetic hit reacts
	if (GetNetMode() == NM_DedicatedServer && !bApplyHitReactOnDedicatedServer)
	{
		// DebugHitReactResult(TEXT("Dedicated server hit react disabled"), true);
		return false;
	}
	
	// Check if hit react is globally disabled
	if (IsHitReactSystemDisabled())
	{ 
		return false;
	}

	// Must have a valid mesh and owner
	if (!Mesh || !IsValid(Mesh->GetOwner()))
	{
		DebugHitReactResult(TEXT("Invalid mesh or owner"), true);
		return false;
	}

	// Extended runtime options
	if (!CanHitReact())
	{
		DebugHitReactResult(TEXT("Hit react not allowed"), true);
		return false;
	}

	// Must have profiles loaded (async)
	if (!bProfilesLoaded)
	{
		DebugHitReactResult(TEXT("Profiles not loaded"), true);
		return false;
	}

	// Need a valid physics asset
	if (!Mesh->GetPhysicsAsset())
	{
		DebugHitReactResult(TEXT("No physics asset available"), true);
		return false;
	}

	// Need a valid mesh asset
	if (!Mesh->GetSkeletalMeshAsset())
	{
		DebugHitReactResult(TEXT("No skeletal mesh asset available"), true);
		return false;
	}

	// Conditionally override the collision enabled state
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

	// If physics state is invalid - i.e. collision is disabled - or it does not have a valid bodies, this will crash right away
	// Since we have done our checks and updated collision this shouldn't really be false
	if (UNLIKELY(!Mesh->IsPhysicsStateCreated() || !Mesh->bHasValidBodies))
	{
		DebugHitReactResult(TEXT("Invalid Bodies"), true);
		return false;
	}

	// Ensure profile is loaded and available
	const UHitReactProfile* Profile = nullptr;
	if (Params.ProfileToUse.IsValid())
	{
		const UHitReactProfile** ProfilePtr = ActiveProfiles.FindByPredicate([&Params](const UHitReactProfile* InProfile)
		{
			return InProfile == Params.ProfileToUse.Get();
		});

		Profile = ProfilePtr ? *ProfilePtr : nullptr;
	}

	// No valid profile found
	if (!Profile)
	{
		DebugHitReactResult(FString::Printf(TEXT("Requested profile { %s } is not available"), *Params.ProfileToUse.ToString()), true);
		return false;
	}

	// Don't apply hit react if the LOD threshold is not met
	if (Profile->LODThreshold >= 0)
	{
		if (Mesh->GetPredictedLODLevel() > Profile->LODThreshold)
		{
			DebugHitReactResult(FString::Printf(TEXT("LOD threshold not met for profile { %s }"), *Params.ProfileToUse.ToString()), true);
			return false;
		}
	}

	// Throttle hit reacts to prevent rapid application
	if (Globals.Cooldown > 0.f && LastHitReactTime >= 0.f)
	{
		if (GetWorld()->TimeSince(LastHitReactTime) < Globals.Cooldown)
		{
			return false;
		}
	}

	// Throttle hit reacts to prevent rapid application also for the profile
	float& LastProfileTime = LastProfileHitReactTimes.FindOrAdd(Profile);
	if (Profile->Cooldown > 0.f)
	{
		if (GetWorld()->TimeSince(LastProfileTime) < Profile->Cooldown)
		{
			return false;
		}
	}

	// Apply the constraint profile to the mesh
	if (!Profile->ConstraintProfile.IsNone())
	{
		bConstraintProfileChanged = true;
		Mesh->SetConstraintProfileForAll(Profile->ConstraintProfile);
	}

	// Gather disabled bones and their descendents
	TArray<FName> DisabledBones = {};
	TMap<FName, float> MaxBoneWeights = {};
	for (const auto& Pair : Profile->BoneOverrides)
	{
		const FName& BoneName = Pair.Key;
		const FHitReactBoneOverride& Override = Pair.Value;
		if (Override.bDisablePhysics || Override.MaxBlendWeight < 1.f)
		{
			// Iterate all descendents
			UHitReactStatics::ForEach(Mesh, BoneName, Override.bIncludeSelf,
				[this, &Override, &DisabledBones, &MaxBoneWeights](const FBodyInstance* BI)
			{
				const FName ChildBoneName = UHitReactStatics::GetBoneName(Mesh, BI);
					
				// Disable all descendents
				if (Override.bDisablePhysics)
				{
					DisabledBones.Add(ChildBoneName);
				}

				// Limit the blend weight for all descendents
				if (Override.MaxBlendWeight < 1.f)
				{
					MaxBoneWeights.Add(ChildBoneName, Override.MaxBlendWeight);
				}
			});
		}
	}

	// Apply the hit react to each bone below the specified bone
	bool bApplied = false;
	bool bAppliedProfile = false;
	FName SimulatedBoneName = NAME_None;  // First bone that was valid and applied to
	UHitReactStatics::ForEach(Mesh, Params.SimulatedBoneName, Params.bIncludeSelf,
		[this, &Profile, &bAppliedProfile, &Params, &bApplied, &DisabledBones, &MaxBoneWeights, &SimulatedBoneName]
		(const FBodyInstance* BI)
	{
		// Determine the bone name to Simulate
		FName BoneName = UHitReactStatics::GetBoneName(Mesh, BI);
		if (Globals.BlacklistedBones.Contains(BoneName))
		{
			// Don't simulate this bone, but continue to the next (return is continue within the lambda)
			return;
		}

		// Don't simulate disabled bones
		if (DisabledBones.Contains(BoneName))
		{
			// Don't simulate this bone, but continue to the next (return is continue within the lambda)
			return;
		}
			
		// Optionally don't apply hit react if we have reached the maximum number of active hit reacts
		if (BoneLimits.bLimitSimulatedBones && PhysicsBlends.Num() >= BoneLimits.MaxSimulatedBones)
		{
			switch (BoneLimits.MaxHitReactHandling)
			{
			case EHitReactMaxHandling::RemoveOldest:
				PhysicsBlends.RemoveAt(0);
				break;
			case EHitReactMaxHandling::PreventNewest:
				return;
			}
		}

		// Apply the animation profile to the first valid bone
		if (!bAppliedProfile)
		{
			bAppliedProfile = true;
			if (PhysicalAnimation && !Profile->PhysicalAnimProfile.IsNone())
			{
				bPhysicalAnimationProfileChanged = true;
				PhysicalAnimation->ApplyPhysicalAnimationProfileBelow(BoneName, Profile->PhysicalAnimProfile, Params.bIncludeSelf);
			}
		}

		// Console command: Log LogHitReact VeryVerbose
		UE_LOG(LogHitReact, VeryVerbose, TEXT("Simulating bone %s"), *BoneName.ToString());

		// Scale the blend weight by the alpha value
		float MaxBlendWeightForBone = MaxBoneWeights.Contains(BoneName) ? MaxBoneWeights[BoneName] : 1.f;
		
		// Apply the hit react to the bone
		FHitReactPhysics& Physics = PhysicsBlends.Add_GetRef({});
		const bool bResult = Physics.HitReact(Mesh, Profile, BoneName, MaxBlendWeightForBone);
		bApplied |= bResult;

		if (bResult)
		{
			SimulatedBoneName = BoneName;
		}
	});

	if (bApplied)
	{
		// Apply physics impulse on next tick
		if (Impulse.CanBeApplied())
		{
			FName ImpulseBoneName = Params.ImpulseBoneName.IsNone() ? SimulatedBoneName : Params.ImpulseBoneName;
			PendingImpulse = { Impulse, World, ImpulseScalar, Profile, ImpulseBoneName };
		}

		// Wake up the hit react system
		WakeHitReact();

		// Track the last hit react time
		LastHitReactTime = GetWorld()->GetTimeSeconds();
		LastProfileTime = LastHitReactTime;
	}
	
	// Print the result
	DebugHitReactResult(bApplied ? TEXT("Hit react applied") : TEXT("Hit react failed to apply"), !bApplied);

	return bApplied;
}

bool UHitReact::HitReactTrigger(const FHitReactTrigger& Params, const FHitReactImpulse_WorldParams& World,
	float ImpulseScalar)
{
	return HitReact(Params, Params.Impulse, World, ImpulseScalar);
}

bool UHitReact::HitReactTrigger_Linear(const FHitReactTrigger_Linear& Params, const FHitReactImpulse_WorldParams& World,
	float ImpulseScalar)
{
	FHitReactImpulseParams ImpulseParams;
	ImpulseParams.LinearImpulse = Params.LinearImpulse;
	return HitReact(Params, ImpulseParams, World, ImpulseScalar);
}

bool UHitReact::HitReactTrigger_Angular(const FHitReactTrigger_Angular& Params,
	const FHitReactImpulse_WorldParams& World, float ImpulseScalar)
{
	FHitReactImpulseParams ImpulseParams;
	ImpulseParams.AngularImpulse = Params.AngularImpulse;
	return HitReact(Params, ImpulseParams, World, ImpulseScalar);
}

bool UHitReact::HitReactTrigger_Radial(const FHitReactTrigger_Radial& Params, const FHitReactImpulse_WorldParams& World,
	float ImpulseScalar)
{
	FHitReactImpulseParams ImpulseParams;
	ImpulseParams.RadialImpulse = Params.RadialImpulse;
	return HitReact(Params, ImpulseParams, World, ImpulseScalar);
}

void UHitReact::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UHitReact::TickComponent);

	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Reset the hit react system if we're not allowed to hit react
	if (!CanHitReact())
	{
		ResetHitReactSystem();
		SleepHitReact();
		PendingImpulse = {};
		return;
	}

	// Tick the global toggle state
	TickGlobalToggle(DeltaTime);
	
	if (PhysicsBlends.Num() == 0)
	{
		if (ShouldSleep() && !IsSleeping())
		{
			// Disable tick
			SleepHitReact();
		}
		PendingImpulse = {};
		return;
	}

	if (!bProfilesLoaded) // Wait for profiles to load
	{
		return;
	}
	
#if UE_ENABLE_DEBUG_DRAWING
	FString DebugBlendWeightString = "";
	bool bDebugPhysicsBlendWeights = ShouldCVarDrawDebug(FHitReactCVars::DebugHitReactBlendWeights);
#endif

	const float GlobalAlpha = GlobalToggle.State.GetBlendStateAlpha();
	TArray<FName> CompletedBlends = {};  // These bones are pending removal
	TMap<FHitReactPhysics*, int32> BoneBlendCount = {};  // Count the number of blends per bone
	PhysicsBlends.RemoveAll([this, DeltaTime, &GlobalAlpha, &BoneBlendCount, &bDebugPhysicsBlendWeights, &DebugBlendWeightString](FHitReactPhysics& Physics)
	{
		// Cache the previous blend weight
		const float LastBlendWeight = Physics.RequestedBlendWeight;

		// Update the physics blend
		Physics.Tick(DeltaTime);

		// Compute the delta blend weight
		const float DeltaBlendWeight = Physics.RequestedBlendWeight - LastBlendWeight;

		// Accumulate the blend weight delta for the bone
		UHitReactStatics::AccumulateBlendWeight(Mesh, Physics, DeltaBlendWeight, GlobalAlpha);

		if (!Physics.HasCompleted())
		{
			BoneBlendCount.FindOrAdd(&Physics)++;
		}

#if UE_ENABLE_DEBUG_DRAWING
		// Debug drawing
		if (bDebugPhysicsBlendWeights)
		{
			if (Physics.IsActive())
			{
				DebugBlendWeightString += FString::Printf(TEXT("%s: [ %s ] %.2f\n"), *Physics.SimulatedBoneName.ToString(),
					*Physics.PhysicsState.GetBlendStateString(), Physics.PhysicsState.GetBlendStateAlpha());
			}
			else
			{
				DebugBlendWeightString += FString::Printf(TEXT("%s: [ %s ]\n"), *Physics.SimulatedBoneName.ToString(),
					*Physics.PhysicsState.GetBlendStateString());
			}
		}
#endif

		return Physics.HasCompleted();
	});

	// Average the blend weight for each bone
	for (const auto& Pair : BoneBlendCount)
	{
		FHitReactPhysics* Physics = Pair.Key;
		const int32 Count = Pair.Value;
		if (Count > 1)
		{
			UHitReactStatics::SetBlendWeight(Mesh, *Physics, Physics->RequestedBlendWeight / Count, GlobalAlpha);
		}
	}

	// Restore our Mesh if all physics blends have been completed
	if (PhysicsBlends.Num() == 0)
	{
		// Restore the collision enabled state
		if (bCollisionEnabledChanged)
		{
			Mesh->SetCollisionEnabled(DefaultCollisionEnabled);
			bCollisionEnabledChanged = false;
		}

		// Remove the constraint profile
		if (bConstraintProfileChanged)
		{
			Mesh->SetConstraintProfileForAll(FName(NAME_None));
			bConstraintProfileChanged = false;
		}

		// Remove the physical anim profile
		if (bPhysicalAnimationProfileChanged)
		{
			PhysicalAnimation->ApplyPhysicalAnimationProfileBelow(FName(NAME_None), FName(NAME_None), false);
			bPhysicalAnimationProfileChanged = false;
		}
	}

	// Finalize the physics simulation for the mesh
	UHitReactStatics::FinalizeMeshPhysics(Mesh);

	if (PendingImpulse.IsValid())
	{
		ApplyImpulse(PendingImpulse);
		PendingImpulse = {};
	}
	
	// Draw debug strings if desired
#if UE_ENABLE_DEBUG_DRAWING
	if (bDebugPhysicsBlendWeights && !DebugBlendWeightString.IsEmpty())
	{
		GEngine->AddOnScreenDebugMessage(GetUniqueDrawDebugKey(692), DeltaTime * 2.f, FColor::Orange, DebugBlendWeightString);
	}
	if (ShouldCVarDrawDebug(FHitReactCVars::DebugHitReactNum))
	{
		GEngine->AddOnScreenDebugMessage(GetUniqueDrawDebugKey(901), DeltaTime * 2.f, FColor::Yellow, FString::Printf(TEXT("Num Hit Reacts: %d"), PhysicsBlends.Num()));
	}
#endif
	
	// Put the system to sleep if there is nothing to do
	if (ShouldSleep())
	{
		// Disable tick
		SleepHitReact();
	}
}

void UHitReact::TickGlobalToggle(float DeltaTime)
{
	// Check if the system is enabled globally via CVar
	bool bDisabledGlobal = false;
#if !UE_BUILD_SHIPPING
	bDisabledGlobal = FHitReactCVars::HitReactDisabled == 1;
	if (bDisabledGlobal)
	{
		if (IsHitReactSystemEnabled())
		{
			ToggleHitReactSystem(false, true);
		}
	}
#endif

	// Check if we need to toggle this ability on or off
#if WITH_GAMEPLAY_ABILITIES
	if (bToggleStateUsingTags && !bDisabledGlobal)
	{
		AbilitySystemComponent = AbilitySystemComponent.IsValid() ?
			AbilitySystemComponent.Get() : UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(GetOwner());
		if (AbilitySystemComponent.IsValid())
		{
			// Possibly disable the system
			if (IsHitReactSystemEnabled())
			{
				// Check if we need to disable the system
				if (AbilitySystemComponent->HasAnyMatchingGameplayTags(DisableTags))
				{
					ToggleHitReactSystem(false, true);
				}
			}

			// Possibly enable the system
			if (IsHitReactSystemDisabled())
			{
				// Check if we need to enable the system
				if (AbilitySystemComponent->HasAnyMatchingGameplayTags(EnableTags))
				{
					ToggleHitReactSystem(true, true);
				}
			}
		}
	}
#endif

	// Update the global alpha interpolation
	const EHitReactToggleState LastToggleState = GetHitReactToggleState();
	
	GlobalToggle.State.Tick(DeltaTime);

	// State has changed
	if (GetHitReactToggleState() != LastToggleState)
	{
		// Reset the system if we've disabled it
		if (GetHitReactToggleState() == EHitReactToggleState::Disabled)
		{
			ResetHitReactSystem();
		}

		// Broadcast the state change
		OnHitReactToggleStateChanged.Broadcast(GetHitReactToggleState());
	}
}

void UHitReact::ApplyImpulse(const FHitReactPendingImpulse& Impulse) const
{
	ApplyImpulse(Impulse.Impulse, Impulse.World, Impulse.ImpulseScalar, Impulse.Profile, Impulse.ImpulseBoneName);
}

void UHitReact::ApplyImpulse(const FHitReactImpulseParams& Impulse, const FHitReactImpulse_WorldParams& World, float ImpulseScalar, const UHitReactProfile* Profile, FName ImpulseBoneName) const
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UHitReact::ApplyImpulse);
	
	if (!ensure(!ImpulseBoneName.IsNone()))
	{
		return;
	}
	
	// Retrieve impulse parameters
	const FHitReactImpulse_Linear& LinearParams = Impulse.LinearImpulse;
	const FHitReactImpulse_Angular& AngularParams = Impulse.AngularImpulse;
	const FHitReactImpulse_Radial& RadialParams = Impulse.RadialImpulse;

	// Throttle impulse based on number of applications
	float ThrottleScalar = 1.f;
	if (Profile->SubsequentImpulseScalars.Num() > 0)
	{
		// Find the scalar for the number of applications based on the last hit react time
		const float TimeSinceLastHitReact = GetWorld()->TimeSince(LastHitReactTime);
		for (int32 i = 0; i < Profile->SubsequentImpulseScalars.Num(); i++)
		{
			const FHitReactSubsequentImpulse& SubsequentImpulse = Profile->SubsequentImpulseScalars[i];
			if (TimeSinceLastHitReact < SubsequentImpulse.ElapsedTime)
			{
				ThrottleScalar = SubsequentImpulse.ImpulseScalar;
				break;
			}
		}
	}

	// Linear impulse
	if (LinearParams.CanBeApplied())
	{
		// Calculate linear impulse
		FVector Linear = LinearParams.GetImpulse(World.LinearDirection) * ImpulseScalar * ThrottleScalar;

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
		FVector Angular = AngularParams.GetImpulse(World.AngularDirection) * ImpulseScalar * ThrottleScalar;

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
		float Radial = RadialParams.Impulse * ImpulseScalar * ThrottleScalar;

		// Apply Radial impulse
		if (!FMath::IsNearlyZero(Radial))
		{
			const ERadialImpulseFalloff Falloff = RadialParams.Falloff == EHitReactFalloff::Linear ? RIF_Linear : RIF_Constant;
				
			// Convert falloff
			Mesh->AddRadialImpulse(World.RadialLocation, RadialParams.Radius, RadialParams.Impulse,
			                       Falloff, RadialParams.IsVelocityChange());

#if UE_ENABLE_DEBUG_DRAWING
			if (FHitReactCVars::DrawHitReact > 0)
			{
				const FVector Center = World.RadialLocation;
				const float Radius = FHitReactCVars::DrawHitReactRadialScale * RadialParams.Radius;
				DrawDebugSphere(Mesh->GetWorld(), Center, Radius, 8, FColor::Blue, false, 1.5f);
			}
#endif
		}
	}
}

void UHitReact::Activate(bool bReset)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UHitReact::Activate);

	// Validate the owner and world
	if (!GetWorld() || !GetWorld()->IsGameWorld() || !GetOwner())
	{
		return;
	}

	// Dedicated servers don't need cosmetic hit reacts - unless perhaps you're doing some kind of replay system
	if (GetNetMode() == NM_DedicatedServer && !bApplyHitReactOnDedicatedServer)
	{
		return;
	}

	const bool bWasActive = IsActive();

	if (!IsActive())
	{
		OwnerPawn = IsValid(GetOwner()) ? Cast<APawn>(GetOwner()) : nullptr;
		
		// Call the pre-activate event, which can be overridden in blueprint or C++ to cast and cache the owner
		PreActivate(bReset);
	}
	
	// Cache anything we need from the owner
	if (Mesh != GetMeshFromOwner() || PhysicalAnimation != GetPhysicalAnimationComponentFromOwner() || bReset)
	{
		// Unbind from the old mesh
		if (PhysicalAnimation && Mesh && PhysicalAnimation->GetSkeletalMesh() == Mesh)
		{
			PhysicalAnimation->SetSkeletalMeshComponent(nullptr);
		}

		// Cache the new mesh and physical animation
		Mesh = GetMeshFromOwner();
		PhysicalAnimation = GetPhysicalAnimationComponentFromOwner();

		// Bind to the new mesh
		if (PhysicalAnimation)
		{
			PhysicalAnimation->SetSkeletalMeshComponent(Mesh);
		}
	}

	if (IsValid(Mesh))
	{
		Super::Activate(bReset);
		if (IsActive() && (!bWasActive || bReset))
		{
			// Load the profiles
			bProfilesLoaded = false;
			ActiveProfiles.Empty();
			CancelAsyncLoading();
			for (TSoftObjectPtr<UHitReactProfile>& ProfilePtr : AvailableProfiles)
			{
				if (ProfilePtr.IsNull()) { continue; }
				AsyncLoad(ProfilePtr, [this, InnerSoftProfile = MoveTemp(ProfilePtr)]() 
				{
					ActiveProfiles.Add(InnerSoftProfile.Get());
				});
			}
			StartAsyncLoading();
		}
	}
	else
	{
		const FString ErrorString = FString::Printf(TEXT(
			"HitReactComponent: Mesh attempted initialization before valid for %s. System will not run."),
			*GetOwner()->GetName());
#if !UE_BUILD_SHIPPING
		FMessageLog("PIE").Error(FText::FromString(ErrorString));
#else
		UE_LOG(LogHitReact, Error, TEXT("%s"), *ErrorString);
#endif
	}
}

void UHitReact::Deactivate()
{
	Super::Deactivate();

	bHasInitialized = false;
	bProfilesLoaded = false;
	if (!IsActive())
	{
		ResetHitReactSystem();
	}
}

void UHitReact::OnFinishedLoading()
{
	bProfilesLoaded = true;
	bHasInitialized = true;
			
	// Bind to the mesh's OnAnimInitialized event
	if (Mesh->OnAnimInitialized.IsAlreadyBound(this, &ThisClass::OnMeshPoseInitialized))
	{
		Mesh->OnAnimInitialized.RemoveDynamic(this, &ThisClass::OnMeshPoseInitialized);
	}
	Mesh->OnAnimInitialized.AddDynamic(this, &ThisClass::OnMeshPoseInitialized);

	// Initialize the tick function
	PrimaryComponentTick.bAllowTickOnDedicatedServer = bApplyHitReactOnDedicatedServer;
	PrimaryComponentTick.GetPrerequisites().Reset();
	AddTickPrerequisiteComponent(Mesh);
	PrimaryComponentTick.SetTickFunctionEnable(true);

	// Limit tick rate
	if (bUseFixedSimulationRate)
	{
		PrimaryComponentTick.TickInterval = 1.f / FMath::Max(1.f, SimulationRate);
	}
	
	// Initialize the global alpha interpolation
	GlobalToggle.State.BlendParams = GlobalToggle.Params;  // Use the default parameters
	GlobalToggle.State.Initialize(true);

	// Broadcast the initialization event
	for (const TSharedRef<FOnHitReactInitialized>& Delegate : RegisteredInitDelegates)
	{
		Delegate->Execute();
	}
}

bool UHitReact::OnHitReactInitialized(FOnHitReactInitialized Delegate)
{
	if (ensure(Delegate.IsBound()))
	{
		if (bHasInitialized)
		{
			Delegate.Execute();
		}
		else
		{
			const TSharedRef<FOnHitReactInitialized> SharedDelegate = MakeShared<FOnHitReactInitialized>(MoveTemp(Delegate));
			RegisteredInitDelegates.Add(SharedDelegate);
		}

		return true;
	}

	return false;
}

void UHitReact::ToggleHitReactSystem(bool bEnabled, bool bInterpolateState, bool bUseDefaultBlendParams,
	FHitReactPhysicsStateParamsSimple BlendParams)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UHitReact::ToggleHitReactSystem);

	// Dedicated servers don't need cosmetic hit reacts - unless perhaps you're doing some kind of replay system
	if (GetNetMode() == NM_DedicatedServer && !bApplyHitReactOnDedicatedServer)
	{
		return;
	}

	// Set the global alpha interpolation parameters if we're interpolating
	if (bInterpolateState)
	{
		const FHitReactPhysicsStateParamsSimple& Params = bUseDefaultBlendParams ? GlobalToggle.Params : BlendParams;
		GlobalToggle.State.BlendParams = Params;
	}
	
	// Only if the state changed
	if (GlobalToggle.State.bToggleEnabled != bEnabled)
	{
		GlobalToggle.State.bToggleEnabled = bEnabled;
		WakeHitReact();
		OnHitReactToggleStateChanged.Broadcast(GetHitReactToggleState());
	}
}

EHitReactToggleState UHitReact::GetHitReactToggleState() const
{
	if (GlobalToggle.State.HasCompleted())
	{
		return GlobalToggle.State.bToggleEnabled ? EHitReactToggleState::Enabled : EHitReactToggleState::Disabled;
	}
		
	return GlobalToggle.State.bToggleEnabled ? EHitReactToggleState::Enabling : EHitReactToggleState::Disabling;
}

bool UHitReact::ShouldSleep() const
{
	if (!bHasInitialized || !bProfilesLoaded || ActiveProfiles.Num() == 0)
	{
		return true;
	}
	if (IsHitReactSystemToggleInProgress())
	{
		return false;
	}
	if (PhysicsBlends.Num() == 0)
	{
		return true;
	}
	return false;
}

bool UHitReact::IsSleeping() const
{
	return bHasInitialized && !PrimaryComponentTick.IsTickFunctionEnabled();
}

void UHitReact::WakeHitReact()
{
	if (IsSleeping())
	{
		PrimaryComponentTick.SetTickFunctionEnable(true);
	}
}

void UHitReact::SleepHitReact()
{
	PrimaryComponentTick.SetTickFunctionEnable(false);
}

bool UHitReact::NeedsCollisionEnabled() const
{
	return Mesh->GetCollisionEnabled() != ECollisionEnabled::QueryAndPhysics && Mesh->GetCollisionEnabled() != ECollisionEnabled::PhysicsOnly;
}

USkeletalMeshComponent* UHitReact::GetMeshFromOwner_Implementation() const
{
	// Default implementation, override in subclass or blueprint
	// By default, get the first found skeletal mesh component
	// For ACharacter, this is always ACharacter::Mesh, because it overrides FindComponentByClass() to return it
	return GetOwner()->GetComponentByClass<USkeletalMeshComponent>();
}

UPhysicalAnimationComponent* UHitReact::GetPhysicalAnimationComponentFromOwner_Implementation() const
{
	// We don't need to have this component
	return GetOwner()->GetComponentByClass<UPhysicalAnimationComponent>();
}

void UHitReact::OnMeshPoseInitialized()
{
	ResetHitReactSystem();
}

void UHitReact::ResetHitReactSystem()
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UHitReact::ResetHitReactSystem);

	if (PhysicsBlends.Num() > 0)
	{
		PhysicsBlends.Reset();
	
		if (Mesh)
		{
			Mesh->SetAllBodiesPhysicsBlendWeight(0.f);
			Mesh->SetAllBodiesSimulatePhysics(false);
		}
	}
}

bool UHitReact::ShouldCVarDrawDebug(int32 CVarValue) const
{
#if UE_ENABLE_DEBUG_DRAWING
	// Invalid data
	if (!GEngine || !Mesh || !Mesh->GetOwner())
	{
		return false;
	}
	
	switch (CVarValue)
	{
	case 0: return false;								// All disabled
	case 1: return true;								// All enabled
	case 2: return GetNetMode() != NM_DedicatedServer;  // All enabled except dedicated servers
	case 3: return IsLocallyControlledPlayer();			// Local client only
	default: return false;								// Not supported
	}
#else
	return false;
#endif
}

bool UHitReact::IsLocallyControlledPlayer() const
{
	return OwnerPawn && OwnerPawn->GetController<APlayerController>() && OwnerPawn->IsLocallyControlled();
}

void UHitReact::DebugHitReactResult(const FString& Result, bool bFailed) const
{
#if UE_ENABLE_DEBUG_DRAWING
	if (!ShouldCVarDrawDebug(FHitReactCVars::DebugHitReactResult))
	{
		return;
	}
	
	// Draw the debug message
	const FString OwnerName = GetOwner() ? GetOwner()->GetName() : TEXT("Unknown");
	const FColor DebugColor = bFailed ? FColor::Red : FColor::Green;
	GEngine->AddOnScreenDebugMessage(-1, 2.4f, DebugColor, FString::Printf(
		TEXT("HitReact: %s - Application: %s"), *OwnerName, *Result));
#endif

#if WITH_EDITOR
	if (bFailed)
	{
		const FString ErrorString = FString::Printf(TEXT("HitReact: %s - Application: %s"), *OwnerName, *Result);
		FMessageLog("PIE").Error(FText::FromString(ErrorString));
	}
#endif
}

#if WITH_EDITOR
#if UE_5_03_OR_LATER
EDataValidationResult UHitReact::IsDataValid(class FDataValidationContext& Context) const
#else
EDataValidationResult UHitReact::IsDataValid(class FDataValidationContext& Context)
#endif
{
	if (AvailableProfiles.Num() == 0)
	{
		Context.AddWarning(LOCTEXT("NoProfiles", "No profiles available. HitReact system will not run."));
	}
	
	return Super::IsDataValid(Context);
}
#endif

#undef LOCTEXT_NAMESPACE