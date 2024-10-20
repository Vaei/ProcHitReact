// Copyright (c) Jared Taylor. All Rights Reserved.


#include "AlphaInterp.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AlphaInterp)

float FInterpState::ApplyTo(const FInterpProperties& Properties, float Target, float InDeltaTime)
{
	// If paused, return the current value
	if (bPaused)
	{
		return InterpolatedValue;
	}

	// Track the last target value to evaluate completion
	LastTargetValue = Target;

	// Map the range
	if (Properties.MapRange.bMapRange)
	{
		Target = Properties.MapRange.MapRange(Target);
	}

	// Apply scale and bias
	Target *= Properties.Scale + Properties.Bias;

	// Clamp the range
	if (Properties.ClampRange.bClampRange)
	{
		Target = Properties.ClampRange.Clamp(Target);
	}

	// Interpolate the value if initialized
	if (bInitialized)
	{
		const bool bIncreasing = Target >= InterpolatedValue;
		const FInterpValue& InterpValue = bIncreasing ? Properties.InterpIn : Properties.InterpOut;
		if (InterpValue.bInterpolate)
		{
			Target = InterpValue.Interpolate(InterpolatedValue, Target, InDeltaTime, Properties.InterpType);
		}
	}

	// Mark as initialized
	bInitialized = true;

	// Update the value
	InterpolatedValue = Target;

	// Return the value
	return Target;
}

float FInterpState::ApplyTo(const FInterpProperties& Properties, float Target) const
{
	if (bPaused)
	{
		return InterpolatedValue;
	}
	
	if (Properties.MapRange.bMapRange)
	{
		Target = Properties.MapRange.MapRange(Target);
	}

	Target *= Properties.Scale + Properties.Bias;

	if (Properties.ClampRange.bClampRange)
	{
		Target = Properties.ClampRange.Clamp(Target);
	}

	return Target;
}
