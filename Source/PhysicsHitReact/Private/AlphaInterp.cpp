// Copyright (c) Jared Taylor. All Rights Reserved.


#include "AlphaInterp.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AlphaInterp)

float FInterpState::ApplyTo(const FInterpParams& Params, float Target, float InDeltaTime)
{
	// If paused, return the current value
	if (bPaused)
	{
		return InterpolatedValue;
	}

	// Track the last target value to evaluate completion
	LastTargetValue = Target;

	// Map the range
	if (Params.MapRange.bMapRange)
	{
		Target = Params.MapRange.MapRange(Target);
	}

	// Apply scale and bias
	Target *= Params.Scale + Params.Bias;

	// Clamp the range
	if (Params.ClampRange.bClampRange)
	{
		Target = Params.ClampRange.Clamp(Target);
	}

	// Interpolate the value if initialized
	if (bInitialized)
	{
		const bool bIncreasing = Target >= InterpolatedValue;
		const FInterpValue& InterpValue = bIncreasing ? Params.InterpIn : Params.InterpOut;
		if (InterpValue.bInterpolate)
		{
			Target = InterpValue.Interpolate(InterpolatedValue, Target, InDeltaTime, Params.InterpType);
		}
	}

	// Mark as initialized
	bInitialized = true;

	// Update the value
	InterpolatedValue = Target;

	// Return the value
	return Target;
}

float FInterpState::ApplyTo(const FInterpParams& Params, float Target) const
{
	if (bPaused)
	{
		return InterpolatedValue;
	}
	
	if (Params.MapRange.bMapRange)
	{
		Target = Params.MapRange.MapRange(Target);
	}

	Target *= Params.Scale + Params.Bias;

	if (Params.ClampRange.bClampRange)
	{
		Target = Params.ClampRange.Clamp(Target);
	}

	return Target;
}
