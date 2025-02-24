// Copyright (c) Jared Taylor

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "HitReactTypes.h"
#include "Physics/HitReactPhysics.h"
#include "Components/ActorComponent.h"
#include "Params/HitReactImpulse.h"
#include "Params/HitReactParams.h"
#include "Params/HitReactTrigger.h"
#include "ThirdParty/AsyncMixinProc.h"
#include "System/HitReactVersioning.h"
#include "HitReact.generated.h"

class UHitReactProfile;
class UPhysicalAnimationComponent;

DECLARE_DYNAMIC_DELEGATE(FOnHitReactInitialized);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHitReactToggleStateChanged, EHitReactToggleState, NewState);

/**
 * Component for applying hit reactions to a skeletal mesh
 */
UCLASS(Config=Game, ClassGroup=(Custom), Blueprintable, meta=(BlueprintSpawnableComponent))
class PROCHITREACT_API UHitReact : public UActorComponent, public FAsyncMixinProc
{
	GENERATED_BODY()

public:
	/** Hit react profiles available for use when applying hit reacts */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=HitReact)
	TArray<TSoftObjectPtr<UHitReactProfile>> AvailableProfiles;

	/** If true, update at the SimulationRate instead of each Tick */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=HitReact, meta=(InlineEditConditionToggle))
	bool bUseFixedSimulationRate = true;

	/**
	 * Rate at which to update the hit react simulation
	 * Higher values will result in more accurate simulation, but may be more expensive
	 * 60 recommended for high quality, 30 for balanced quality, 15 for low quality, 120 for cinematic quality
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=HitReact, meta=(EditCondition="bUseFixedSimulationRate", UIMin="1", ClampMin="1", UIMax="120", Delta="1"))
	float SimulationRate = 30.f;

	/** Limits for the number of bones that can be simulated for hit reacts to improve performance and visuals */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=HitReact)
	FHitReactBoneLimits BoneLimits;
	
	/** Settings that apply to all hit reacts regardless of profile */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=HitReact)
	FHitReactGlobals Globals;

	/** Whether to apply hit reacts on dedicated servers */
	UPROPERTY(Config, EditDefaultsOnly, BlueprintReadOnly, AdvancedDisplay, Category=HitReact)
	bool bApplyHitReactOnDedicatedServer = false;

	/** Global interp toggle parameters for enabling and disabling the hit react system */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=HitReact)
	FHitReactGlobalToggle GlobalToggle;
	
protected:
	/** Bones currently being simulated */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category=HitReact)
	TArray<FHitReactPhysics> PhysicsBlends;

	/** Pending impulse to apply on the next Tick */
	UPROPERTY()
	FHitReactPendingImpulse PendingImpulse;

	/** Loaded profiles from AvailableProfiles ready to be used */
	UPROPERTY(Transient, VisibleInstanceOnly, BlueprintReadOnly, Category="HitReact|Internal")
	TArray<const UHitReactProfile*> ActiveProfiles;

	/** True if the profiles have been loaded */
	UPROPERTY(Transient, VisibleInstanceOnly, BlueprintReadOnly, Category="HitReact|Internal")
	bool bProfilesLoaded = false;

	/** True if the hit react system has completed it's initialization */
	UPROPERTY(Transient, VisibleInstanceOnly, BlueprintReadOnly, Category="HitReact|Internal")
	bool bHasInitialized = false;
	
	/** Last time a hit reaction was applied - prevent rapid application causing poor results */
	UPROPERTY(Transient, VisibleInstanceOnly, BlueprintReadOnly, Category="HitReact|Internal")
	float LastHitReactTime = -1.f;

	/** Last time a hit reaction was applied for a specific profile */
	UPROPERTY(Transient, VisibleInstanceOnly, BlueprintReadOnly, Category="HitReact|Internal")
	TMap<TSoftObjectPtr<const UHitReactProfile>, float> LastProfileHitReactTimes;

	/** True if the physical animation profile was changed, and should be removed upon completion of all hit reacts */
	UPROPERTY(Transient, VisibleInstanceOnly, BlueprintReadOnly, Category="HitReact|Internal")
	bool bPhysicalAnimationProfileChanged;

	/** True if the constraint profile was changed, and should be removed upon completion of all hit reacts */
	UPROPERTY(Transient, VisibleInstanceOnly, BlueprintReadOnly, Category="HitReact|Internal")
	bool bConstraintProfileChanged;

	/** True if the collision was changed, and should be reverted upon completion of all hit reacts */
	UPROPERTY(Transient, VisibleInstanceOnly, BlueprintReadOnly, Category="HitReact|Internal")
	bool bCollisionEnabledChanged;

	/** Default collision state to revert to when hit reacts are completed */
	UPROPERTY()
	TEnumAsByte<ECollisionEnabled::Type> DefaultCollisionEnabled;

	/** Mesh to simulate hit reactions on */
	UPROPERTY(Transient, DuplicateTransient, BlueprintReadOnly, Category="HitReact|References")
	TObjectPtr<USkeletalMeshComponent> Mesh;

	/** Owner Pawn, if the owner is actually a Pawn, otherwise nullptr */
	UPROPERTY(Transient, DuplicateTransient, BlueprintReadOnly, Category="HitReact|References")
	TObjectPtr<APawn> OwnerPawn;

	/** Physical animation component to apply physical animation profiles */
	UPROPERTY(Transient, DuplicateTransient, BlueprintReadOnly, Category="HitReact|References")
	TObjectPtr<UPhysicalAnimationComponent> PhysicalAnimation;

public:
	/** Called when the hit react system is toggled on or off */
	UPROPERTY(BlueprintAssignable, Category=HitReact)
	FOnHitReactToggleStateChanged OnHitReactToggleStateChanged;

public:
	const TArray<FHitReactPhysics>& GetPhysicsBlends() const { return PhysicsBlends; }
	
public:
	UHitReact(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	/**
	 * Trigger a hit reaction on the specified bone
	 * @param Params The hit react input parameters
	 * @param Impulse The impulse parameters to apply
	 * @param World The world space parameters to apply
	 * @param ImpulseScalar Universal scalar to apply to all impulses included in ImpulseParams
	 * @return True if the hit react was applied
	 */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category=HitReact)
	bool HitReact(const FHitReactInputParams& Params, FHitReactImpulseParams Impulse,
		const FHitReactImpulse_WorldParams& World, float ImpulseScalar = 1.f);
		
	/**
	 * Trigger a hit reaction on the specified bone using FHitReactTrigger Params
	 * Typically used when replicating FHitReactApplyParams, for convenience
	 * @param Params The hit react trigger parameters
	 * @param World The world space parameters to apply
	 * @param ImpulseScalar The scalar to apply to the impulse
	 * @return True if the hit react was applied
	 */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category=HitReact, meta=(DisplayName="Hit React Trigger"))
	bool HitReactTrigger(const FHitReactTrigger& Params, const FHitReactImpulse_WorldParams& World,
		float ImpulseScalar = 1.f);

	/**
	 * Trigger a hit reaction on the specified bone using ApplyParamsLinear
	 * Typically used when replicating FHitReactApplyParamsLinear, for convenience
	 * @param Params The hit react trigger parameters
	 * @param World The world space parameters to apply
	 * @param ImpulseScalar The scalar to apply to the impulse
	 * @return True if the hit react was applied
	 */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category=HitReact, meta=(DisplayName="Hit React Trigger (Linear)"))
	bool HitReactTrigger_Linear(const FHitReactTrigger_Linear& Params, const FHitReactImpulse_WorldParams& World,
		float ImpulseScalar = 1.f);

	/**
	 * Trigger a hit reaction on the specified bone using ApplyParamsAngular
	 * Typically used when replicating FHitReactApplyParamsAngular, for convenience
	 * @param Params The hit react trigger parameters
	 * @param World The world space parameters to apply
	 * @param ImpulseScalar The scalar to apply to the impulse
	 * @return True if the hit react was applied
	 */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category=HitReact, meta=(DisplayName="Hit React Trigger (Angular)"))
	bool HitReactTrigger_Angular(const FHitReactTrigger_Angular& Params, const FHitReactImpulse_WorldParams& World,
		float ImpulseScalar = 1.f);

	/**
	 * Trigger a hit reaction on the specified bone using ApplyParamsRadial
	 * Typically used when replicating FHitReactApplyParamsRadial, for convenience
	 * @param Params The hit react trigger parameters
	 * @param World The world space parameters to apply
	 * @param ImpulseScalar The scalar to apply to the impulse
	 * @return True if the hit react was applied
	 */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category=HitReact, meta=(DisplayName="Hit React Trigger (Radial)"))
	bool HitReactTrigger_Radial(const FHitReactTrigger_Radial& Params, const FHitReactImpulse_WorldParams& World,
		float ImpulseScalar = 1.f);
	
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void TickGlobalToggle(float DeltaTime);

	void ApplyImpulse(const FHitReactPendingImpulse& Impulse) const;
	
	void ApplyImpulse(const FHitReactImpulseParams& Impulse, const FHitReactImpulse_WorldParams& World, float ImpulseScalar,
		const UHitReactProfile* Profile, FName ImpulseBoneName) const;

public:
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

	virtual void OnFinishedLoading() override;

	/** Called when the hit react system is initialized */
	UFUNCTION(BlueprintCallable, Category=HitReact)
	bool OnHitReactInitialized(FOnHitReactInitialized Delegate);

protected:
	TArray<TSharedRef<FOnHitReactInitialized>> RegisteredInitDelegates;
	
public:
	/**
	 * Instantly disables the system entirely if currently running, clearing all active hit reacts, if false
	 * To disable the system smoothly use ToggleHitReactSystem instead
	 * Consider using IsAnyHitReactInProgress to check if the system is currently running
	 * @return False if no hit reacts can be applied
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCosmetic, Category=HitReact)
	bool CanHitReact() const;
	virtual bool CanHitReact_Implementation() const { return true; }
	
public:
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

	/** Current toggle state of the hit react system */
	UFUNCTION(BlueprintPure, BlueprintCosmetic, Category=HitReact)
	EHitReactToggleState GetHitReactToggleState() const;

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
	/**
	 * Mesh needs to use QueryAndPhysics or PhysicsOnly for CollisionEnabled 
	 * @return True if mesh needs to change to valid collision properties
	 */
	bool NeedsCollisionEnabled() const;
	
public:
	/** Get the mesh to simulate from the owner */
	UFUNCTION(BlueprintNativeEvent, BlueprintCosmetic, Category=HitReact)
	USkeletalMeshComponent* GetMeshFromOwner() const;

	/** Get the cached mesh to simulate */
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
	UFUNCTION()
	virtual void OnMeshPoseInitialized();

	virtual void ResetHitReactSystem();
	
protected:
	bool ShouldCVarDrawDebug(int32 CVarValue) const;
	bool IsLocallyControlledPlayer() const;

	uint64 GetUniqueDrawDebugKey(int32 Offset) const { return (GetUniqueID() + Offset) % UINT32_MAX; }

private:
	/**
	 * Notify user of the result of a hit react
	 * Useful for debugging
	 */
	void DebugHitReactResult(const FString& Result, bool bFailed) const;

#if WITH_EDITOR
#if UE_5_03_OR_LATER
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#else
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) override;
#endif
#endif
};
