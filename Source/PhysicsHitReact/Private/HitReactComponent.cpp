// Copyright (c) Jared Taylor. All Rights Reserved


#include "HitReactComponent.h"

#include "HitReact.h"
#include "HitReactTags.h"
#include "PhysicsEngine/PhysicalAnimationComponent.h"

#if WITH_GAMEPLAY_ABILITIES
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#endif

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "Framework/Notifications/NotificationManager.h"
#endif

#include UE_INLINE_GENERATED_CPP_BY_NAME(HitReactComponent)

namespace FHitReactCVars
{
#if ENABLE_DRAW_DEBUG
	static int32 DebugHitReactResult = 0;
	FAutoConsoleVariableRef CVarDebugHitReactResult(
		TEXT("p.HitReact.Debug.Result"),
		DebugHitReactResult,
		TEXT("Draw debug strings when hit reactions are applied or rejected. Does not inform when rejecting due to blacklist or cooldown. 0: Disable, 1: Enable, 2: Enable for all but dedicated servers, 3: Enable local client only\n")
		TEXT("0: Disable, 1: Enable, 2: Enable for all but dedicated servers, 3: Enable local client only"),
		ECVF_Default);

	static int32 DebugHitReactBlendWeights = 0;
	FAutoConsoleVariableRef CVarDebugHitReactBlendWeights(
		TEXT("p.HitReact.Debug.BlendWeights"),
		DebugHitReactBlendWeights,
		TEXT("Draw debug string showing the value of each currently simulated physics blend. 0: Disable, 1: Enable, 2: Enable for all but dedicated servers, 3: Enable local client only.\n")
		TEXT("0: Disable, 1: Enable, 2: Enable for all but dedicated servers, 3: Enable local client only"),
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
#endif
}

#define LOCTEXT_NAMESPACE "HitReact"

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

	// Assign built in profiles
	Profiles = FHitReactBuiltInProfiles::GetBuiltInProfiles();
}

bool UHitReactComponent::HitReactWithApplyParams(const FHitReactApplyParams& ApplyParams,
	const FHitReactImpulseWorldParams& WorldParams, float ImpulseScalar)
{
	return HitReact(ApplyParams, ApplyParams.ImpulseParams, WorldParams, ImpulseScalar);
}

bool UHitReactComponent::HitReact(const FHitReactParams& Params, FHitReactImpulseParams ImpulseParams,
	FHitReactImpulseWorldParams WorldParams, float ImpulseScalar)
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

	// Check if hit react is globally disabled
	if (IsHitReactSystemDisabled())
	{ 
		return false;
	}

	// Default to the default profile if none supplied
	FGameplayTag ProfileToUse = Params.ProfileToUse;
	if (!Params.ProfileToUse.IsValid())
	{
		ProfileToUse = FHitReactTags::HitReact_Profile_Default;
	}

	// Profile must exist
	const FHitReactProfile* Profile = Profiles.Find(ProfileToUse);
	if (!Profile)
	{
		DebugHitReactResult(TEXT("Invalid hit react profile"), true);
		const FString ErrorString = FString::Printf(TEXT("HitReact: Invalid hit react profile { %s } for { %s }"),
			*ProfileToUse.ToString(), *GetOwner()->GetName());
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
	
	// Simulated bone must be valid
	FName SimulatedBoneName = Params.SimulatedBoneName;
	if (SimulatedBoneName.IsNone())
	{
		DebugHitReactResult(TEXT("Simulated bone name cannot be None"), true);
		return false;
	}

	// Check if the simulated bone is blacklisted
	if (Profile->SimulatedBoneMapping.BlacklistBones.Contains(SimulatedBoneName))
	{
		return false;
	}

	// Get the impulse bone name
	FName ImpulseBoneName = Params.GetImpulseBoneName();
	
	// Check if the impulse bone is blacklisted
	if (Profile->ImpulseBoneMapping.BlacklistBones.Contains(ImpulseBoneName))
	{
		return false;
	}

	// Remap the simulated bone name
	if (Profile->SimulatedBoneMapping.RemapBones.Contains(SimulatedBoneName))
	{
		SimulatedBoneName = Profile->SimulatedBoneMapping.RemapBones[SimulatedBoneName];
#if WITH_EDITOR
		if (!ensure(!SimulatedBoneName.IsNone()))
#else
		if (BoneName.IsNone())
#endif
		{
			DebugHitReactResult(TEXT("Simulated bone name cannot be None - Was remapped to None!"), true);
			return false;
		}
	}

	// Remap the impulse bone name
	if (Profile->ImpulseBoneMapping.RemapBones.Contains(ImpulseBoneName))
	{
		ImpulseBoneName = Profile->ImpulseBoneMapping.RemapBones[ImpulseBoneName];
	}

	// Add the hit react to the physics blend map, or retrieve the existing one
	FHitReact& Physics = PhysicsBlends.FindOrAdd(SimulatedBoneName);

	// Determine the correct bone parameters to use
	const bool bUseCached = Physics.CachedBoneParams && Physics.CachedProfile == Profile;  // Only if profile hasn't changed
	const FHitReactBoneApplyParams* BoneApplyParams = bUseCached ? Physics.CachedBoneParams : &Profile->DefaultBoneApplyParams;

	if (BoneApplyParams->PhysicsBlendParams.GetTotalTime() <= 0.f)
	{
		DebugHitReactResult(TEXT("Invalid physics blend params -- total time cannot be 0"), true);
		return false;
	}

	// Override bone apply params for this bone if desired
	if (Profile->OverrideBoneApplyParams.Contains(SimulatedBoneName))
	{
		BoneApplyParams = &Profile->OverrideBoneApplyParams[SimulatedBoneName];
	}

	// Throttle hit reacts to prevent rapid application
	if (LastHitReactTime >= 0.f)
	{
		const float TimeSinceLastHitReact = GetWorld()->TimeSince(LastHitReactTime);
		if (TimeSinceLastHitReact < BoneApplyParams->Cooldown)
		{
			return false;
		}
	}

	// Trigger the hit reaction
	bool bResult = Physics.HitReact(Mesh, PhysicalAnimation, SimulatedBoneName, ImpulseBoneName,
		Params.bIncludeSelf, Profile, BoneApplyParams, ImpulseParams, WorldParams, ImpulseScalar);

	// Track the last hit react time if successful to throttle rapid application
	if (bResult)
	{
		LastHitReactTime = GetWorld()->GetTimeSeconds();

		// Sort hit reacts so the child bones are simulated last,
		//	i.e. they overwrite the blend weight set by their parents calling SetAllBodiesBelowPhysicsBlendWeight, etc.
		if (PhysicsBlends.Num() > 1)
		{
			PhysicsBlends.ValueSort([](const FHitReact& A, const FHitReact& B)
			{
				return A.CachedBoneIndex < B.CachedBoneIndex;
			});
		}
	}

	// Print the result if desired
	DebugHitReactResult(bResult ? TEXT("Hit react applied") : TEXT("Hit react failed"), !bResult);

	return bResult;
}

void UHitReactComponent::ToggleHitReactSystem(bool bEnabled, bool bInterpolateState, bool bUseDefaultBlendParams,
	FHitReactPhysicsStateParamsSimple BlendParams)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UHitReactComponent::ToggleHitReactSystem);

	// Dedicated servers don't need cosmetic hit reacts - unless perhaps you're doing some kind of replay system
	if (GetNetMode() == NM_DedicatedServer && !bApplyHitReactOnDedicatedServer)
	{
		return;
	}

	// Set the global alpha interpolation parameters if we're interpolating
	if (bInterpolateState)
	{
		const FHitReactPhysicsStateParamsSimple& Params = bUseDefaultBlendParams ? GlobalToggleParams : BlendParams;
		GlobalPhysicsToggle.BlendParams = Params;
	}
	
	// Only if the state changed
	if (GlobalPhysicsToggle.bToggleEnabled != bEnabled)
	{
		GlobalPhysicsToggle.bToggleEnabled = bEnabled;
		OnHitReactToggleStateChanged.Broadcast(GetHitReactToggleState());
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
	
	GlobalPhysicsToggle.Tick(DeltaTime);

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

	// No need to update physics if the system is disabled or there are no physics blends
	if (PhysicsBlends.Num() == 0)
	{
		return;
	}

#if ENABLE_DRAW_DEBUG
	FString DebugBlendWeightString = "";
	bool bDebugPhysicsBlendWeights = ShouldCVarDrawDebug(FHitReactCVars::DebugHitReactBlendWeights);
#endif

	// Update physics blends
	const float GlobalAlpha = GlobalPhysicsToggle.GetBlendStateAlpha();
	const float GlobalScalar = GlobalAlpha;
	TArray<FName> CompletedPhysicsBlends = {};
	for (auto& Pair : PhysicsBlends)
	{
		FHitReact& Physics = Pair.Value;

		// Update the physics blend - returns True if completed
		if (Physics.Tick(GlobalScalar, DeltaTime))
		{
			// Remove the physics blend if it has completed
			CompletedPhysicsBlends.Add(Pair.Key);
		}

#if ENABLE_DRAW_DEBUG
		if (bDebugPhysicsBlendWeights)
		{
			if (Physics.PhysicsState.IsActive())
			{
				DebugBlendWeightString += FString::Printf(TEXT("%s: [ %s ] %.2f\n"), *Pair.Key.ToString(),
					*Physics.PhysicsState.GetBlendStateString(), Physics.PhysicsState.GetBlendStateAlpha());
			}
			else
			{
				DebugBlendWeightString += FString::Printf(TEXT("%s: [ %s ]\n"), *Pair.Key.ToString(),
					*Physics.PhysicsState.GetBlendStateString());
			}
		}
#endif
	}

	// Remove any completed physics blends
	for (const FName& BoneName : CompletedPhysicsBlends)
	{
		PhysicsBlends.Remove(BoneName);
	}

	// Draw debug strings if desired
#if ENABLE_DRAW_DEBUG
	if (bDebugPhysicsBlendWeights && !DebugBlendWeightString.IsEmpty())
	{
		GEngine->AddOnScreenDebugMessage(GetUniqueDrawDebugKey(692), 1.2f, FColor::Orange, DebugBlendWeightString);
	}
#endif
}

void UHitReactComponent::Activate(bool bReset)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UHitReactComponent::Activate);

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

#if WITH_EDITOR
	// Validate the data before activating, and log any warnings or errors, and prevent activation if invalid
	// This is probably too slow to run in shipping builds
	TArray<FText> Warnings;
	TArray<FText> Errors;
	const EDataValidationResult ValidationResult = IsHitReactDataValid(Warnings, Errors);

	if (Warnings.Num() > 0 || Errors.Num() > 0)
	{
		FMessageLog("PIE").Warning(FText::FromString("PhysicsHitReact system will not display these warnings or errors in shipping builds"));
	}

	for (const FText& Warning : Warnings)
	{
		FMessageLog("PIE").Warning(Warning);
	};

	for (const FText& Error : Errors)
	{
		FMessageLog("PIE").Error(Error);
	};

	// Prevent activation if invalid -- warnings are allowed
	if (ValidationResult == EDataValidationResult::Invalid && Errors.Num() > 0)
	{
		// Pop open the message log immediately, to save the user wondering why the system isn't working in their current session
		// FMessageLog("PIE").Open(EMessageSeverity::Error);
		FNotificationInfo Info(FText::FromString("PhysicsHitReact system disabled\nSee message log for details"));
		Info.ExpireDuration = 3.0f; // Duration in seconds
		Info.bFireAndForget = true;
		FSlateNotificationManager::Get().AddNotification(Info);
		return;
	}
#endif

	// Call the pre-activate event, which can be overridden in blueprint or C++ to cast and cache the owner
	PreActivate(bReset);
	
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
			GlobalPhysicsToggle.BlendParams = GlobalToggleParams;  // Use the default parameters
			GlobalPhysicsToggle.Initialize(true);
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

bool UHitReactComponent::ShouldCVarDrawDebug(int32 CVarValue) const
{
#if ENABLE_DRAW_DEBUG
	// Invalid data
	if (!GEngine || !Mesh || !Mesh->GetOwner())
	{
		return false;
	}

	// Possibly skip drawing on dedicated servers
	if (GetNetMode() == NM_DedicatedServer && (CVarValue == 2 || CVarValue == 3))
	{
		return false;
	}

	if (CVarValue == 1 || (CVarValue == 3 && Mesh->GetOwner()->GetLocalRole() == ROLE_AutonomousProxy))
	{
		return true;
	}

#endif
	return false;
}

void UHitReactComponent::DebugHitReactResult(const FString& Result, bool bFailed) const
{
#if ENABLE_DRAW_DEBUG
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
}

#if WITH_EDITOR
void UHitReactComponent::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

#if !WITH_GAMEPLAY_ABILITIES
	if (bToggleStateUsingTags && PropertyChangedEvent.GetPropertyName().IsEqual(
		GET_MEMBER_NAME_CHECKED(ThisClass, bToggleStateUsingTags)))
	{
		FMessageLog MessageLog("AssetCheck");
		MessageLog.Error(FText::FromString(TEXT(
			"HitReact: bToggleStateUsingTags requires Gameplay Abilities to be enabled in your .uproject file.")));
		MessageLog.Open(EMessageSeverity::Error);
		bToggleStateUsingTags = false;
	}
#endif
}

#if UE_5_03_OR_LATER
EDataValidationResult UHitReactComponent::IsDataValid(class FDataValidationContext& Context)
{
	TArray<FText> ValidationWarnings;
	TArray<FText> ValidationErrors;
	const EDataValidationResult Result = IsHitReactDataValid(ValidationWarnings, ValidationErrors);
	
	for (const FText& Warning : ValidationWarnings)
	{
		Context.AddWarning(Warning);
	}
	
	for (const FText& Error : ValidationErrors)
	{
		Context.AddError(Error);
	}

	if (Result == EDataValidationResult::Invalid)
	{
		return EDataValidationResult::Invalid;
	}
	
	return Super::IsDataValid(Context);
}
#else

EDataValidationResult UHitReactComponent::IsDataValid(TArray<FText>& ValidationErrors)
{
	TArray<FText> ValidationWarnings = {};
	if (IsHitReactDataValid(ValidationWarnings, ValidationErrors) == EDataValidationResult::Invalid)
	{
		return EDataValidationResult::Invalid;
	}
	return Super::IsDataValid(ValidationErrors);
}

#endif  // UE_5_03_OR_LATER
#endif  // WITH_EDITOR

EDataValidationResult UHitReactComponent::IsHitReactDataValid(TArray<FText>& ValidationWarnings,
	TArray<FText>& ValidationErrors) const
{
	for (const auto& Pair : Profiles)
	{
		const FGameplayTag& ProfileTag = Pair.Key;
		const FHitReactProfile& Profile = Pair.Value;

		// Check for invalid bone names
		for (const auto& BonePair : Profile.OverrideBoneApplyParams)
		{
			const FName& BoneName = BonePair.Key;
			if (BoneName.IsNone())
			{
				ValidationErrors.Add(FText::Format(
					LOCTEXT("HitReactComponent_InvalidBoneName", "HitReact: Invalid bone name in profile {0}"),
					FText::FromName(ProfileTag.GetTagName())));
				return EDataValidationResult::Invalid;
			}
		}

		for (const auto& BonePair : Profile.SimulatedBoneMapping.RemapBones)
		{
			const FName& BoneName = BonePair.Value;
			if (BoneName.IsNone())
			{
				ValidationErrors.Add(FText::Format(
					LOCTEXT("HitReactComponent_InvalidBoneName", "HitReact: Invalid simulated bone name in profile {0}"),
					FText::FromName(ProfileTag.GetTagName())));
				return EDataValidationResult::Invalid;
			}

			if (Profile.SimulatedBoneMapping.BlacklistBones.Contains(BoneName))
			{
				ValidationErrors.Add(FText::Format(
					LOCTEXT("HitReactComponent_InvalidBoneName", "HitReact: Simulated bone name {0} is both remapped and blacklisted in profile {1}"),
					FText::FromName(BoneName), FText::FromName(ProfileTag.GetTagName())));
				return EDataValidationResult::Invalid;
			}
		}

		for (const auto& BonePair : Profile.ImpulseBoneMapping.RemapBones)
		{
			const FName& BoneName = BonePair.Value;
			if (BoneName.IsNone())
			{
				ValidationErrors.Add(FText::Format(
					LOCTEXT("HitReactComponent_InvalidBoneName", "HitReact: Invalid impulse bone name in profile {0}"),
					FText::FromName(ProfileTag.GetTagName())));
				return EDataValidationResult::Invalid;
			}

			if (Profile.ImpulseBoneMapping.BlacklistBones.Contains(BoneName))
			{
				ValidationErrors.Add(FText::Format(
					LOCTEXT("HitReactComponent_InvalidBoneName", "HitReact: Impulse bone name {0} is both remapped and blacklisted in profile {1}"),
					FText::FromName(BoneName), FText::FromName(ProfileTag.GetTagName())));
				return EDataValidationResult::Invalid;
			}
		}

		for (const auto& BoneName : Profile.SimulatedBoneMapping.BlacklistBones)
		{
			if (BoneName.IsNone())
			{
				ValidationErrors.Add(FText::Format(
					LOCTEXT("HitReactComponent_InvalidBoneName", "HitReact: Invalid bone name in profile {0}"),
					FText::FromName(ProfileTag.GetTagName())));
				return EDataValidationResult::Invalid;
			}
		}

		for (const auto& BoneName : Profile.ImpulseBoneMapping.BlacklistBones)
		{
			if (BoneName.IsNone())
			{
				ValidationErrors.Add(FText::Format(
					LOCTEXT("HitReactComponent_InvalidBoneName", "HitReact: Invalid bone name in profile {0}"),
					FText::FromName(ProfileTag.GetTagName())));
				return EDataValidationResult::Invalid;
			}
		}

		for (const auto& BonePair : Profile.OverrideBoneParams)
		{
			const FName& BoneName = BonePair.Key;
			if (BoneName.IsNone())
			{
				ValidationErrors.Add(FText::Format(
					LOCTEXT("HitReactComponent_InvalidBoneName", "HitReact: Invalid bone name in profile {0}"),
					FText::FromName(ProfileTag.GetTagName())));
				return EDataValidationResult::Invalid;
			}
		}
	}
	return EDataValidationResult::Valid;
}

#undef LOCTEXT_NAMESPACE