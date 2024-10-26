// Copyright (c) Jared Taylor. All Rights Reserved.


#include "HitReactStatics.h"

#include "HitReactImpulseParams.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(HitReactStatics)

FHitReactApplyParams UHitReactStatics::SetHitReactApplyWorldParams(const FHitReactApplyParams& ApplyParams, const FHitReactImpulseWorldParams& WorldParams)
{
	FHitReactApplyParams NewParams = ApplyParams;
	NewParams.WorldSpaceParams = WorldParams;
	return NewParams;
}

FHitReactApplyParams UHitReactStatics::SetHitReactApplyWorldLinearDirection(const FHitReactApplyParams& ApplyParams, const FVector& LinearDirection)
{
	FHitReactApplyParams NewParams = ApplyParams;
	NewParams.WorldSpaceParams.LinearDirection = LinearDirection;
	return NewParams;
}

FHitReactApplyParams UHitReactStatics::SetHitReactApplyWorldAngularDirection(const FHitReactApplyParams& ApplyParams, const FVector& AngularDirection)
{
	FHitReactApplyParams NewParams = ApplyParams;
	NewParams.WorldSpaceParams.AngularDirection = AngularDirection;
	return NewParams;
}

FHitReactApplyParams UHitReactStatics::SetHitReactApplyWorldRadialLocation(const FHitReactApplyParams& ApplyParams, const FVector& RadialLocation)
{
	FHitReactApplyParams NewParams = ApplyParams;
	NewParams.WorldSpaceParams.RadialLocation = RadialLocation;
	return NewParams;
}