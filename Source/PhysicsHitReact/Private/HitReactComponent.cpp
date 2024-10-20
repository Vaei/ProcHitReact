// Copyright (c) Jared Taylor. All Rights Reserved


#include "HitReactComponent.h"

#include "HitReactTags.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(HitReactComponent)

namespace FHitReactCVars
{
#if ENABLE_DRAW_DEBUG
	static int32 DebugHitReactResult = 0;
	FAutoConsoleVariableRef CVarDebugHitReactResult(
		TEXT("p.HitReact.Debug.Result"),
		DebugHitReactResult,
		TEXT("Optionally draw debug strings when hit reactions are applied or rejected.\n")
		TEXT("0: Disable, 1: Enable, 2: Enable for all but dedicated servers, 3: Enable local client only"),
		ECVF_Default);
#endif
}

UHitReactComponent::UHitReactComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	Profiles = {
		{ FHitReactTags::HitReact_Profile_Default, FHitReactProfile() }
	};

	FHitReactProfile& NoArms = Profiles.Add(FHitReactTags::HitReact_Profile_NoArms, FHitReactProfile());
	NoArms.OverrideBoneParams = {
		{ TEXT("clavicle_l"), { true, 0.f, 0.f } },
		{ TEXT("clavicle_r"), { true, 0.f, 0.f } }
	};
}

bool UHitReactComponent::HitReact(FGameplayTag ProfileToUse, FName BoneName, bool bOnlyBonesBelow,
	FVector Direction, const float Magnitude, EHitReactUnits Units, bool bFactorMass)
{
	// Avoid GC issues
	if (!IsValid(GetOwner()))
	{
		return false;
	}
	
	// Dedicated servers don't need cosmetic hit reacts - unless perhaps you're doing some kind of replay system
	if (GetNetMode() == NM_DedicatedServer && !bApplyHitReactOnDedicatedServer)
	{
		DebugHitReactResult(TEXT("Dedicated server hit react disabled"), true);
		return false;
	}

	// Profile must exist
	const FHitReactProfile* Profile = Profiles.Find(ProfileToUse);
	if (!Profile)
	{
		DebugHitReactResult(TEXT("Invalid hit react profile"), true);
		const FString ErrorString = FString::Printf(TEXT("HitReact: Invalid hit react profile { %s } for { %s }"), *ProfileToUse.ToString(), *GetOwner()->GetName());
#if !UE_BUILD_SHIPPING
		FMessageLog("PIE").Error(FText::FromString(ErrorString));
#else
		UE_LOG(LogHitReact, Error, TEXT("%s"), *ErrorString);
#endif
		return false;
	}

	// Must have a valid mesh and owner
	if (!Mesh || !IsValid(Mesh->GetOwner()))
	{
		DebugHitReactResult(TEXT("Invalid mesh or owner"), true);
		return false;
	}

	// Must be bound to a skeletal mesh, or we'll never update
	if (!PostMeshPoseUpdateHandle.IsValid())
	{
		DebugHitReactResult(TEXT("Mesh pose update not bound"), true);
		return false;
	}

	// Extended runtime options
	if (!CanHitReact())
	{
		DebugHitReactResult(TEXT("Hit react not allowed"), true);
		return false;
	}

	// Get the physics blend for this bone
	FHitReact& Physics = PhysicsBlends.FindOrAdd(BoneName);

	// Determine the correct bone properties to use
	const bool bUseCached = Physics.CachedBoneParams && Physics.CachedProfile == Profile;  // Only if profile hasn't changed
	const FHitReactBoneApplyParams* Params = bUseCached ? Physics.CachedBoneParams : &Profile->DefaultBoneApplyParams;

	// Override bone apply params for this bone if desired
	if (Profile->OverrideBoneApplyParams.Contains(BoneName))
	{
		Params = &Profile->OverrideBoneApplyParams[BoneName];
	}

	// Trigger the hit reaction
	bool bResult = Physics.HitReact(Mesh, BoneName, bOnlyBonesBelow, Profile, Params, Direction, Magnitude, Units, bFactorMass);
	
	DebugHitReactResult(bResult ? TEXT("Hit react applied") : TEXT("Hit react failed"), !bResult);

	return bResult;
}

void UHitReactComponent::ToggleHitReactSystem(bool bEnabled,
	bool bInterpolateState, const FInterpProperties& InterpProperties)
{
	// Set the global alpha interpolation properties if we're interpolating
	if (bInterpolateState)
	{
		GlobalAlphaInterp.Properties = InterpProperties;
	}

	// Determine the new toggle state based on whether we're interpolating or not
	EHitReactToggleState NewState;
	if (bInterpolateState)
	{
		NewState = bEnabled ? EHitReactToggleState::Enabling : EHitReactToggleState::Disabling;
	}
	else
	{
		NewState = bEnabled ? EHitReactToggleState::Enabled : EHitReactToggleState::Disabled;
	}

	// Update the state if it's changed
	if (NewState != HitReactToggleState)
	{
		HitReactToggleState = NewState;
		OnHitReactToggleStateChanged.Broadcast(HitReactToggleState);
	}
}

bool UHitReactComponent::CanHitReact_Implementation() const
{
	return true;
}

bool UHitReactComponent::ShouldPauseHitReactSystem_Implementation() const
{
	return false;
}

int32 UHitReactComponent::GetNumHitReactsInProgress() const
{
	return PhysicsBlends.Num();
}

bool UHitReactComponent::IsAnyHitReactInProgress() const
{
	return PhysicsBlends.Num() > 0;
}

void UHitReactComponent::OnMeshPoseInitialized()
{
	ResetHitReactSystem();
}

void UHitReactComponent::PostMeshPoseUpdate()
{
	const float DeltaTime = GetWorld()->GetDeltaSeconds();

	// Reset the hit react system if we're not allowed to hit react
	if (!CanHitReact())
	{
		ResetHitReactSystem();
		return;
	}

	// Pause the hit react system if we're not allowed to hit react
	if (ShouldPauseHitReactSystem())
	{
		return;
	}

	// Update the global alpha interpolation and toggle state if destination has not been reached
	if (HitReactToggleState == EHitReactToggleState::Enabling || HitReactToggleState == EHitReactToggleState::Disabling)
	{
		// Update the global alpha interpolation
		const float TargetAlpha = HitReactToggleState == EHitReactToggleState::Enabling ? 1.f : 0.f;
		GlobalAlphaInterp.Interpolate(TargetAlpha, DeltaTime);

		// If we've completed the interpolation, update the toggle state
		if (GlobalAlphaInterp.HasCompleted())
		{
			// Update the toggle state
			HitReactToggleState = HitReactToggleState == EHitReactToggleState::Enabling ? EHitReactToggleState::Enabled : EHitReactToggleState::Disabled;

			// Clear physics blends if the system is disabled
			if (HitReactToggleState == EHitReactToggleState::Disabled)
			{
				ResetHitReactSystem();
			}

			// Broadcast the state change
			OnHitReactToggleStateChanged.Broadcast(HitReactToggleState);
		}
	}

	// No need to update physics if the system is disabled or there are no physics blends
	if (PhysicsBlends.Num() == 0)
	{
		return;
	}

	// Update physics blends
	const float GlobalScalar = GlobalAlphaInterp.GetInterpolatedValue();
	TArray<FName> CompletedPhysicsBlends = {}; 
	for (auto& Pair : PhysicsBlends)
	{
		FHitReact& Physics = Pair.Value;

		// Update the physics blend - returns True if completed
		if (Physics.Update(GlobalScalar, DeltaTime))
		{
			// Remove the physics blend if it has completed
			CompletedPhysicsBlends.Add(Pair.Key);
		}
	}

	// Remove any completed physics blends
	for (const FName& BoneName : CompletedPhysicsBlends)
	{
		PhysicsBlends.Remove(BoneName);
	}
}

void UHitReactComponent::ResetHitReactSystem()
{
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

void UHitReactComponent::OnRegister()
{
	Super::OnRegister();

	if (GetWorld() && GetWorld()->IsGameWorld())
	{
		SetupHitReactComponent();
	}
}

void UHitReactComponent::InitializeComponent()
{
	Super::InitializeComponent();

	SetupHitReactComponent();
}

void UHitReactComponent::SetupHitReactComponent()
{
	// Cache anything we need from the owner
	Mesh = GetMeshFromOwner();

	if (ensureAlways(IsValid(Mesh)))
	{
		Mesh->OnAnimInitialized.AddDynamic(this, &ThisClass::OnMeshPoseInitialized);
		PostMeshPoseUpdateHandle = Mesh->RegisterOnBoneTransformsFinalizedDelegate(
			FOnBoneTransformsFinalizedMultiCast::FDelegate::CreateUObject(this, &ThisClass::PostMeshPoseUpdate));

		GlobalAlphaInterp.Initialize(1.f);
	}
	else
	{
		const FString ErrorString = FString::Printf(TEXT("HitReactComponent: Mesh attempted initialization before valid for %s. System will not run."), *GetOwner()->GetName());
#if !UE_BUILD_SHIPPING
		FMessageLog("PIE").Error(FText::FromString(ErrorString));
#else
		UE_LOG(LogHitReact, Error, TEXT("%s"), *ErrorString);
#endif
	}
}

void UHitReactComponent::DebugHitReactResult(const FString& Result, bool bFailed) const
{
#if ENABLE_DRAW_DEBUG
	if (!GEngine || !Mesh || !Mesh->GetOwner())
	{
		return;
	}
	if (GetNetMode() == NM_DedicatedServer && (FHitReactCVars::DebugHitReactResult == 2 || FHitReactCVars::DebugHitReactResult == 3))
	{
		return;
	}
	if (FHitReactCVars::DebugHitReactResult == 1 || (FHitReactCVars::DebugHitReactResult == 3 && Mesh->GetOwner()->GetLocalRole() == ROLE_AutonomousProxy))
	{
		const FString OwnerName = GetOwner() ? GetOwner()->GetName() : TEXT("Unknown");
		const FColor DebugColor = bFailed ? FColor::Red : FColor::Green;
		GEngine->AddOnScreenDebugMessage(-1, 2.4f, DebugColor, FString::Printf(TEXT("HitReact: %s - Application: %s"), *OwnerName, *Result));
	}
#endif
}

USkeletalMeshComponent* UHitReactComponent::GetMeshFromOwner_Implementation() const
{
	// Default implementation, override in subclass or blueprint
	// By default, get the first found skeletal mesh component
	// For ACharacter, this is always ACharacter::Mesh, because it overrides FindComponentByClass() to return it
	return GetOwner()->GetComponentByClass<USkeletalMeshComponent>();
}
