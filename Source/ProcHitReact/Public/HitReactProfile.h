// Copyright (c) Jared Taylor

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Params/HitReactParams.h"
#include "System/HitReactVersioning.h"
#include "HitReactProfile.generated.h"

/**
 * Profiles define how hit reactions are applied to a skeletal mesh
 */
UCLASS(Blueprintable, BlueprintType)
class PROCHITREACT_API UHitReactProfile : public UDataAsset
{
	GENERATED_BODY()

public:
#if WITH_EDITORONLY_DATA
	/** Description of this profile -- editor only */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=HitReact, meta=(MultiLine="true"))
	FString Description;
#endif

	/**
	 * The blend parameters to apply
	 * Interpolation state handling for hit reactions
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=HitReact)
	FHitReactPhysicsStateParams BlendParams;

	/** Maximum weight provided to physical animation (0 is disabled, 1 is full) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=HitReact, meta=(UIMin="0", ClampMin="0", UIMax="1", ClampMax="1", Delta="0.05", ForceUnits="%"))
	float MaxBlendWeight;
	
	/**
	 * Hit reacts will not trigger until Cooldown has lapsed when repeating this profile
	 * Trigger may still be prevented by global cooldown even if this is met -- global cooldown overrides this one
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=HitReact, meta=(UIMin="0", ClampMin="0", UIMax="1", Delta="0.01", ForceUnits="s"))
	float Cooldown;

	/**
	 * Scale the impulse based on the number of times the bone has been hit prior to completing the hit react
	 * The first array element is the scalar for the first subsequent hit, and so on
	 * This is used to throttle the impulse applied to the bone as it is hit multiple times
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Physics)
	TArray<FHitReactSubsequentImpulse> SubsequentImpulseScalars;
	
	/**
	 * Bone-specific override params
	 * All active profiles accumulate these parameters, so if a bone is included in multiple profiles, values will be
	 *  averages, however bDisablePhysics will be true if any profile has it set to true
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=HitReact)
	TMap<FName, FHitReactBoneOverride> BoneOverrides;

	/**
	 * Physical animation profile to apply to this bone and any below
	 * Requires a Physical Animation Component to exist on the owning actor
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=HitReact)
	FName PhysicalAnimProfile;

	/**
	 * Constraint profile to apply to all bones
	 * This is applied to the physics asset on the mesh
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=HitReact)
	FName ConstraintProfile;

	/*
	* Max LOD that this hit react is allowed to run
	* For example if you have LODThreshold to be 2, it will run until LOD 2 (based on 0 index)
	* when the component LOD becomes 3, it will stop update/evaluate
	* currently transition would be issue and that has to be re-visited
	*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Performance, meta=(DisplayName="LOD Threshold", ClampMin="-1", UIMin="-1"))
	int32 LODThreshold;
	
public:
	UHitReactProfile()
		: MaxBlendWeight(0.4f)
		, Cooldown(0.015f)
		, SubsequentImpulseScalars({
			{ 0.1f, 0.35f },
			{ 0.25f, 0.5f },
			{ 0.35f, 0.7f },
			{ 0.5f, 0.9f } })
		, PhysicalAnimProfile(NAME_None)
		, ConstraintProfile(NAME_None)
		, LODThreshold(-1)
	{}

#if WITH_EDITOR
#if UE_5_03_OR_LATER
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#else
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) override;
#endif
#endif
};
