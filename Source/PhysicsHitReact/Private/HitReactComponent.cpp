// Copyright (c) Jared Taylor. All Rights Reserved


#include "HitReactComponent.h"

#include "HitReact.h"
#include "HitReactTags.h"
#include "PhysicsEngine/PhysicalAnimationComponent.h"

#if WITH_GAMEPLAY_ABILITIES
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#endif

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
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	PrimaryComponentTick.bAllowTickOnDedicatedServer = false;
	PrimaryComponentTick.TickGroup = TG_PrePhysics;

	bAutoActivate = true;

	// Requires ability system to be enabled
	bToggleStateUsingTags = false;
	
	Profiles = {
		{ FHitReactTags::HitReact_Profile_Default, FHitReactProfile() }
	};

	FHitReactProfile& NoArms = Profiles.Add(FHitReactTags::HitReact_Profile_NoArms, FHitReactProfile());
	NoArms.OverrideBoneParams = {
		{ TEXT("clavicle_l"), { false, true, 0.f, 0.f } },
		{ TEXT("clavicle_r"), { false, true, 0.f, 0.f } }
	};
}

bool UHitReactComponent::HitReact(FGameplayTag ProfileToUse, FName BoneName, bool bIncludeSelf,
	FHitReactImpulseParams ImpulseParams, FHitReactImpulseWorldParams WorldSpaceParams)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UHitReactComponent::HitReact);

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

	// Default to the default profile if none supplied
	if (!ProfileToUse.IsValid())
	{
		ProfileToUse = FHitReactTags::HitReact_Profile_Default;
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
	bool bResult = Physics.HitReact(Mesh, PhysicalAnimation, BoneName, bIncludeSelf, Profile, Params, ImpulseParams, WorldSpaceParams);
	
	DebugHitReactResult(bResult ? TEXT("Hit react applied") : TEXT("Hit react failed"), !bResult);

	return bResult;
}

void UHitReactComponent::ToggleHitReactSystem(bool bEnabled,
	bool bInterpolateState, const FInterpProperties& InterpProperties)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UHitReactComponent::ToggleHitReactSystem);

	// Dedicated servers don't need cosmetic hit reacts - unless perhaps you're doing some kind of replay system
	if (GetNetMode() == NM_DedicatedServer && !bApplyHitReactOnDedicatedServer)
	{
		return;
	}

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

void UHitReactComponent::ResetHitReactSystem()
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UHitReactComponent::ResetHitReactSystem);

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

void UHitReactComponent::TickComponent(float DeltaTime, enum ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UHitReactComponent::TickComponent);

	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
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

	// Check if we need to toggle this ability on or off
#if WITH_GAMEPLAY_ABILITIES
	if (bToggleStateUsingTags)
	{
		AbilitySystemComponent = AbilitySystemComponent.IsValid() ? AbilitySystemComponent.Get() : UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(GetOwner());
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

void UHitReactComponent::Activate(bool bReset)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UHitReactComponent::Activate);

	if (!GetWorld() || !GetWorld()->IsGameWorld() || !GetOwner())
	{
		Super::Activate(bReset);
		return;
	}

	// Dedicated servers don't need cosmetic hit reacts - unless perhaps you're doing some kind of replay system
	if (GetNetMode() == NM_DedicatedServer && !bApplyHitReactOnDedicatedServer)
	{
		Super::Activate(bReset);
		return;
	}
	
	const bool bWasActive = IsActive();
	
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

			// Initialize the global alpha interpolation
			GlobalAlphaInterp.Properties = GlobalToggleProperties;  // Use the default properties
			GlobalAlphaInterp.Initialize(1.f);
		}
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

void UHitReactComponent::Deactivate()
{
	Super::Deactivate();

	if (!IsActive())
	{
		ResetHitReactSystem();
	}
}

UPhysicalAnimationComponent* UHitReactComponent::GetPhysicalAnimationComponentFromOwner_Implementation() const
{
	// We don't need to have this component
	return GetOwner()->GetComponentByClass<UPhysicalAnimationComponent>();
}

USkeletalMeshComponent* UHitReactComponent::GetMeshFromOwner_Implementation() const
{
	// Default implementation, override in subclass or blueprint
	// By default, get the first found skeletal mesh component
	// For ACharacter, this is always ACharacter::Mesh, because it overrides FindComponentByClass() to return it
	return GetOwner()->GetComponentByClass<USkeletalMeshComponent>();
}

#if WITH_EDITOR
void UHitReactComponent::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

#if !WITH_GAMEPLAY_ABILITIES
	if (bToggleStateUsingTags && PropertyChangedEvent.GetPropertyName().IsEqual(GET_MEMBER_NAME_CHECKED(ThisClass, bToggleStateUsingTags)))
	{
		FMessageLog MessageLog("AssetCheck");
		MessageLog.Error(FText::FromString(TEXT("HitReact: bToggleStateUsingTags requires Gameplay Abilities to be enabled in your .uproject file.")));
		MessageLog.Open(EMessageSeverity::Error);
		bToggleStateUsingTags = false;
	}
#endif
}
#endif

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
