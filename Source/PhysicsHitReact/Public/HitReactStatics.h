// Copyright (c) Jared Taylor. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "HitReactStatics.generated.h"

struct FHitReactImpulseWorldParams;
struct FHitReactApplyParams;

/**
 * Function library for hit reactions
 */
UCLASS()
class PHYSICSHITREACT_API UHitReactStatics : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * Set the world space parameters for a hit react impulse
	 * @param ApplyParams The hit react apply parameters
	 * @param WorldParams The world space parameters to apply
	 * @return A copy of the apply parameters with the world space parameters set
	 */
	UFUNCTION(BlueprintCallable, Category=HitReact)
	static FHitReactApplyParams SetHitReactApplyWorldParams(const FHitReactApplyParams& ApplyParams, const FHitReactImpulseWorldParams& WorldParams);

	/** 
	 * Set the world space linear direction for a hit react impulse
	 * @param ApplyParams The hit react apply parameters
	 * @param LinearDirection The world space linear direction to apply
	 * @return A copy of the apply parameters with the world space linear direction set
	 */
	UFUNCTION(BlueprintCallable, Category=HitReact)
	static FHitReactApplyParams SetHitReactApplyWorldLinearDirection(const FHitReactApplyParams& ApplyParams, const FVector& LinearDirection);

	/**
	 * Set the world space angular direction for a hit react impulse
	 * @param ApplyParams The hit react apply parameters
	 * @param AngularDirection The world space angular direction to apply
	 * @return A copy of the apply parameters with the world space angular direction set
	 */
	UFUNCTION(BlueprintCallable, Category=HitReact)
	static FHitReactApplyParams SetHitReactApplyWorldAngularDirection(const FHitReactApplyParams& ApplyParams, const FVector& AngularDirection);

	/**
	 * Set the world space radial location for a hit react impulse
	 * @param ApplyParams The hit react apply parameters
	 * @param RadialLocation The world space radial location to apply
	 * @return A copy of the apply parameters with the world space radial location set
	 */
	UFUNCTION(BlueprintCallable, Category=HitReact)
	static FHitReactApplyParams SetHitReactApplyWorldRadialLocation(const FHitReactApplyParams& ApplyParams, const FVector& RadialLocation);
};
