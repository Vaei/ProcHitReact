// Copyright (c) Jared Taylor. All Rights Reserved.


#include "AlphaInterp.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AlphaInterp)

float FInterpValue::Interpolate(float Current, float Target, float DeltaTime, const EInterpFunc& InterpFunc) const
{
	if (DeltaTime <= 0.f) { return Current; }
	switch (InterpFunc)
	{
		case EInterpFunc::FInterpTo: return FMath::FInterpTo(Current, Target, DeltaTime, InterpRate);
		case EInterpFunc::FInterpConstantTo: return FMath::FInterpConstantTo(Current, Target, DeltaTime, InterpRate);
		default: return Current;
	}
}

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
		const bool bDecaying = Params.bEnableDecay && !FMath::IsNearlyZero(DecayValue);
		const bool bIncreasing = Target >= InterpolatedValue;
		const FInterpValue& InterpValue = bIncreasing ? Params.InterpIn : Params.InterpOut;

		// Interpolate the value
		if (InterpValue.bInterpolate)
		{
			// If not decaying or decay mode allows interpolation, interpolate the value
			if (!bDecaying || Params.DecayMode == EDecayMode::AllowInterpolation)
			{
				Target = InterpValue.Interpolate(InterpolatedValue, Target, InDeltaTime, InterpValue.InterpType);
			}
			else
			{
				Target = InterpolatedValue;
			}
		}

		// If decaying, apply the decay
		if (bDecaying)
		{
			const float LastDecayValue = DecayValue;
			if (Params.InterpDecay.bInterpolate)
			{
				DecayValue = Params.InterpDecay.Interpolate(DecayValue, 0.f, InDeltaTime, Params.InterpDecay.InterpType);
			}
			else
			{
				DecayValue = 0.f;
			}

			// Apply the decay to interpolated value
			const float DecayAmount = FMath::Abs<float>(DecayValue - LastDecayValue);
			const float Decay = bIncreasing ? -DecayAmount : DecayAmount;
			Target += Decay;

			// Reapply the clamp after decay
			if (Params.ClampRange.bClampRange)
			{
				Target = Params.ClampRange.Clamp(Target);
			}
		}
	}

	// Mark as initialized
	bInitialized = true;

	// Update the value
	InterpolatedValue = Target;

	// Return the value
	return Target;
}

void FInterpState::ApplyDecay(const FInterpParams& Params, float DecayAmount)
{
	if (!Params.bEnableDecay)
	{
		return;
	}
	
	DecayValue += DecayAmount;

	// Clamp the decay value, if necessary
	if (Params.bClampDecay)
	{
		DecayValue = FMath::Min(DecayValue, Params.MaxDecay);
	}
}

void FInterpState::Reset()
{
	bInitialized = false;
	bPaused = false;
	InterpolatedValue = 0.f;
	LastTargetValue = 0.f;
	DecayValue = 0.f;
}
