// Copyright (c) Jared Taylor. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "HitReact.h"
#include "HitReactTypes.h"
#include "Components/ActorComponent.h"
#include "Params/HitReactApplyParams.h"
#include "Params/HitReactParams.h"
#include "System/HitReactVersioning.h"
#include "HitReactComponent.generated.h"

class UPhysicalAnimationComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHitReactToggleStateChanged, EHitReactToggleState, NewState);

/**
 * Component for applying hit reactions to a skeletal mesh
 */
UCLASS(Config=Game, ClassGroup=(Custom), Blueprintable, meta=(BlueprintSpawnableComponent))
class PROCHITREACT_API UHitReactComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	/** Hit react profiles to use when applying hit reacts */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=HitReact, meta=(Categories="HitReact.Profile"))
	TMap<FGameplayTag, FHitReactProfile> Profiles;

	/** Whether to apply hit reacts on dedicated servers */
	UPROPERTY(Config)
	bool bApplyHitReactOnDedicatedServer = false;

	/** Global interp toggle parameters for enabling/disabling the hit react system */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=HitReact)
	FHitReactPhysicsStateParamsSimple GlobalToggleParams;

	/**
	 * Requires GameplayAbilities plugin to be loaded!
	 * 
	 * Whether to toggle the system using gameplay tags
	 * Disabling this can be a performance optimization if you know the system will not be toggled at runtime via tags
	 *	because we won't have to look for AbilitySystemComponent
	 * @see DisableTags, EnableTagsOverride
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=HitReact)
	bool bToggleStateUsingTags;
	
	/** If component owner has any gameplay tags assigned via their AbilitySystemComponent, this will be toggled to a disabled state using GlobalToggleParams */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=HitReact, meta=(EditCondition="bToggleStateUsingTags", EditConditionHides))
	FGameplayTagContainer DisableTags;

	/**
	 * If component owner has any gameplay tags assigned via their AbilitySystemComponent, this will be toggled to an enabled state using GlobalToggleParams
	 * @warning This overrides DisableTags!
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=HitReact, meta=(EditCondition="bToggleStateUsingTags", EditConditionHides))
	FGameplayTagContainer EnableTags;
	
protected:
	/** Bones currently being simulated */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category=HitReact)
	TMap<FName, FHitReact> PhysicsBlends;

	/** Mesh to simulate hit reactions on */
	UPROPERTY(Transient, DuplicateTransient, BlueprintReadOnly, Category=HitReact)
	TObjectPtr<USkeletalMeshComponent> Mesh;

	/** Owner Pawn, if the owner is actually a Pawn, otherwise nullptr */
	UPROPERTY(Transient, DuplicateTransient, BlueprintReadOnly, Category=HitReact)
	TObjectPtr<APawn> OwnerPawn;

	UPROPERTY(Transient, DuplicateTransient, BlueprintReadOnly, Category=HitReact)
	TObjectPtr<UPhysicalAnimationComponent> PhysicalAnimation;
	
	TWeakObjectPtr<class UAbilitySystemComponent> AbilitySystemComponent;

	/** Global physics interpolation for toggling the system on and off */
	UPROPERTY(Transient, BlueprintReadOnly, Category=HitReact)
	FHitReactPhysicsStateSimple GlobalPhysicsToggle;

	/** Current toggle state of the hit react system */
	EHitReactToggleState GetHitReactToggleState() const
	{
		if (GlobalPhysicsToggle.HasCompleted())
		{
			return GlobalPhysicsToggle.bToggleEnabled ? EHitReactToggleState::Enabled : EHitReactToggleState::Disabled;
		}
		
		return GlobalPhysicsToggle.bToggleEnabled ? EHitReactToggleState::Enabling : EHitReactToggleState::Disabling;
	}

	/** Last time a hit reaction was applied - prevent rapid application causing poor results */
	UPROPERTY()
	float LastHitReactTime = -1.f;

	UPROPERTY()
	bool bHasInitialized = false;

public:
	/** Called when the hit react system is toggled on or off */
	UPROPERTY(BlueprintAssignable, Category=HitReact)
	FOnHitReactToggleStateChanged OnHitReactToggleStateChanged;
	
public:
	UHitReactComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	/**
	 * Trigger a hit reaction on the specified bone
	 * @param Params The hit react parameters
	 * @param ImpulseParams The impulse parameters to apply
	 * @param WorldParams The world space parameters to apply
	 * @param ImpulseScalar Universal scalar to apply to all impulses included in ImpulseParams
	 * @return True if the hit react was applied
	 */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category=HitReact, meta=(Categories="HitReact.Profile"))
	bool HitReact(const FHitReactParams& Params, FHitReactImpulseParams ImpulseParams, FHitReactImpulseWorldParams WorldParams, float ImpulseScalar = 1.f);
	
	/**
	 * Trigger a hit reaction on the specified bone using ApplyParams
	 * Typically used when replicating FHitReactApplyParams, for convenience
	 * @param ApplyParams The hit react apply parameters
	 * @param WorldParams The world space parameters to apply
	 * @param ImpulseScalar The scalar to apply to the impulse
	 * @return True if the hit react was applied
	 */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category=HitReact, meta=(Categories="HitReact.Profile"))
	bool HitReactWithApplyParams(const FHitReactApplyParams& ApplyParams, const FHitReactImpulseWorldParams& WorldParams,
		float ImpulseScalar = 1.f);

	/**
	 * Trigger a hit reaction on the specified bone using ApplyParamsLinear
	 * Typically used when replicating FHitReactApplyParamsLinear, for convenience
	 * @param LinearApplyParams The hit react apply parameters
	 * @param WorldParams The world space parameters to apply
	 * @param ImpulseScalar The scalar to apply to the impulse
	 * @return True if the hit react was applied
	 */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category=HitReact, meta=(Categories="HitReact.Profile"))
	bool HitReactWithApplyParamsLinear(const FHitReactApplyParamsLinear& LinearApplyParams, const FHitReactImpulseWorldParams& WorldParams,
		float ImpulseScalar = 1.f);

	/**
	 * Trigger a hit reaction on the specified bone using ApplyParamsAngular
	 * Typically used when replicating FHitReactApplyParamsAngular, for convenience
	 * @param AngularApplyParams The hit react apply parameters
	 * @param WorldParams The world space parameters to apply
	 * @param ImpulseScalar The scalar to apply to the impulse
	 * @return True if the hit react was applied
	 */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category=HitReact, meta=(Categories="HitReact.Profile"))
	bool HitReactWithApplyParamsAngular(const FHitReactApplyParamsAngular& AngularApplyParams, const FHitReactImpulseWorldParams& WorldParams,
		float ImpulseScalar = 1.f);

	/**
	 * Trigger a hit reaction on the specified bone using ApplyParamsRadial
	 * Typically used when replicating FHitReactApplyParamsRadial, for convenience
	 * @param RadialApplyParams The hit react apply parameters
	 * @param WorldParams The world space parameters to apply
	 * @param ImpulseScalar The scalar to apply to the impulse
	 * @return True if the hit react was applied
	 */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category=HitReact, meta=(Categories="HitReact.Profile"))
	bool HitReactWithApplyParamsRadial(const FHitReactApplyParamsRadial& RadialApplyParams, const FHitReactImpulseWorldParams& WorldParams,
		float ImpulseScalar = 1.f);

	/**
	 * Toggle the hit react system on or off
	 * @param bEnabled - Whether to enable or disable the hit react system
	 * @param bInterpolateState - Whether to interpolate the state change
	 * @param bUseDefaultBlendParams - Whether to use the default blend parameters, if False, BlendParams will be used
	 * @param BlendParams - Interpolation parameters to use - will not be applied if bInterpolateState is false - requires bUseDefaultBlendParams to be false
	 */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category=HitReact)
	void ToggleHitReactSystem(bool bEnabled, bool bInterpolateState = true, bool bUseDefaultBlendParams = true,
		FHitReactPhysicsStateParamsSimple BlendParams = FHitReactPhysicsStateParamsSimple());

	/**
	 * Instantly disables the system entirely if currently running, clearing all active hit reacts, if false
	 * To disable the system smoothly use ToggleHitReactSystem instead
	 * Consider using IsAnyHitReactInProgress to check if the system is currently running
	 * @return False if no hit reacts can be applied
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCosmetic, Category=HitReact)
	bool CanHitReact() const;

	/** @return Number of hit reacts currently in progress */
	UFUNCTION(BlueprintPure, BlueprintCosmetic, Category=HitReact)
	int32 GetNumHitReactsInProgress() const;

	/** @return True if any hit reacts are currently in progress */
	UFUNCTION(BlueprintPure, BlueprintCosmetic, Category=HitReact)
	bool IsAnyHitReactInProgress() const;

	/** @return True if the hit react system is enabled or enabling */
	UFUNCTION(BlueprintPure, BlueprintCosmetic, Category=HitReact)
	bool IsHitReactSystemEnabled() const
	{
		return GetHitReactToggleState() == EHitReactToggleState::Enabled || GetHitReactToggleState() == EHitReactToggleState::Enabling;
	}

	/** @return True if the hit react system is currently enabling or disabling */
	UFUNCTION(BlueprintPure, BlueprintCosmetic, Category=HitReact)
	bool IsHitReactSystemToggleInProgress() const
	{
		return GetHitReactToggleState() == EHitReactToggleState::Enabling || GetHitReactToggleState() == EHitReactToggleState::Disabling;
	}

	/** @return True if the hit react system is disabled or disabling */
	UFUNCTION(BlueprintPure, BlueprintCosmetic, Category=HitReact)
	bool IsHitReactSystemDisabled() const { return !IsHitReactSystemEnabled(); }
	
protected:
	UFUNCTION()
	virtual void OnMeshPoseInitialized();

	virtual void ResetHitReactSystem();

	/**
	 * Stop ticking if True
	 * Will wake up when hit reacts are applied or global toggle state changes
	 */
	virtual bool ShouldSleep() const;

	/** Is tick disabled */
	bool IsSleeping() const;
	
	/** Resume ticking */
	virtual void WakeHitReact();

	/** Disable ticking */
	virtual void SleepHitReact();

public:
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/**
	 * Called prior to Activating the hit react system
	 * Convenient location to cast and cache the owner
	 * @param bReset - Whether we will reset the system before activating
	 */
	UFUNCTION(BlueprintNativeEvent, Category=HitReact)
	void PreActivate(bool bReset);
	virtual void PreActivate_Implementation(bool bReset) {}
	
	virtual void Activate(bool bReset) override;
	virtual void Deactivate() override;
	
	/** Get the mesh to simulate from the owner */
	UFUNCTION(BlueprintNativeEvent, BlueprintCosmetic, Category=HitReact)
	USkeletalMeshComponent* GetMeshFromOwner() const;
	
	UFUNCTION(BlueprintPure, BlueprintCosmetic, Category=HitReact)
	USkeletalMeshComponent* GetMesh() const { return Mesh; }

	/** Get the PhysicalAnimationComponent from the owner */
	UFUNCTION(BlueprintNativeEvent, BlueprintCosmetic, Category=HitReact)
	UPhysicalAnimationComponent* GetPhysicalAnimationComponentFromOwner() const;

	/**
	 * Get the PhysicalAnimationComponent
	 * Can be null if the owner does not have a PhysicalAnimationComponent
	 * Used to set animation profiles and apply physical animation settings
	 */
	UFUNCTION(BlueprintPure, BlueprintCosmetic, Category=HitReact)
	UPhysicalAnimationComponent* GetPhysicalAnimationComponent() const { return PhysicalAnimation; }
	
protected:
	uint64 GetUniqueDrawDebugKey(int32 Offset) const { return (GetUniqueID() + Offset) % UINT32_MAX; }
	bool ShouldCVarDrawDebug(int32 CVarValue) const;
	bool IsLocallyControlledPlayer() const;

private:
	/**
	 * Notify user of the result of a hit react
	 * Useful for debugging
	 */
	void DebugHitReactResult(const FString& Result, bool bFailed) const;
	
public:
#if WITH_EDITOR
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#if UE_5_03_OR_LATER
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#else
	virtual EDataValidationResult IsDataValid(TArray<FText>& ValidationErrors) override;
#endif
#endif

	virtual EDataValidationResult IsHitReactDataValid(TArray<FText>& ValidationWarnings, TArray<FText>& ValidationErrors) const;
};
