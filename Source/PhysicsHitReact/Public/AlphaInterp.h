// Copyright (c) Jared Taylor. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Animation/InputScaleBias.h"

#include "AlphaInterp.generated.h"

/**
 * Which interpolation function to use
 */
UENUM(BlueprintType)
enum class EInterpFunc : uint8
{
	FInterpTo				UMETA(Tooltip = "Tries to reach Target based on distance from Current position, giving a nice smooth feeling when tracking a position. Frame-rate dependent results, suitable for cosmetic purposes only."),
	FInterpConstantTo		UMETA(Tooltip = "Tries to reach Target at a constant rate. Frame-rate independent results, suitable for gameplay mechanics."),
};

/**
 * How to handle decay when interpolating
 */
UENUM(BlueprintType)
enum class EDecayMode : uint8
{
	BlockInterpolation		UMETA(Tooltip = "Block interpolation when decaying"),
	AllowInterpolation		UMETA(Tooltip = "Allow Interpolation to continue when decaying"),
};

/**
 * Map value to a range based on interpolation state
 */
USTRUCT(BlueprintType)
struct PHYSICSHITREACT_API FInterpMapRange
{
	GENERATED_BODY()

	FInterpMapRange(const FInputRange& InInterpIn = {}, const FInputRange& InInterpOut = {})
		: bMapRange(false)
		, InterpInRange(InInterpIn)
		, InterpOutRange(InInterpOut)
	{}

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Interpolation)
	bool bMapRange;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Interpolation, meta=(EditCondition="bMapRange", EditConditionHides))
	FInputRange InterpInRange;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Interpolation, meta=(EditCondition="bMapRange", EditConditionHides))
	FInputRange InterpOutRange;

	float MapRange(float Value) const
	{
		return FMath::GetMappedRangeValueUnclamped(InterpInRange.ToVector2f(), InterpOutRange.ToVector2f(), Value);
	}
};

/**
 * Clamp the result to a range
 */
USTRUCT(BlueprintType)
struct PHYSICSHITREACT_API FInterpClampRange
{
	GENERATED_BODY()

	FInterpClampRange(const float InClampMin = 0.f, const float InClampMax = 1.f)
		: bClampRange(true)
		, ClampMin(InClampMin)
		, ClampMax(InClampMax)
	{}

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Interpolation)
	bool bClampRange;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Interpolation, meta=(EditCondition="bClampRange", EditConditionHides))
	float ClampMin;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Interpolation, meta=(EditCondition="bClampRange", EditConditionHides))
	float ClampMax;

	float Clamp(float Value) const
	{
		return FMath::Clamp<float>(Value, ClampMin, ClampMax);
	}
};

/**
 * Interpolation rate and whether to interpolate
 */
USTRUCT(BlueprintType)
struct PHYSICSHITREACT_API FInterpValue
{
	GENERATED_BODY()

	FInterpValue(const float InInterpRate = 10.f, const EInterpFunc InInterpType = EInterpFunc::FInterpTo)
		: bInterpolate(true)
		, InterpRate(InInterpRate)
		, InterpType(InInterpType)
	{}

	/** Whether to interpolate at all */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Interpolation)
	bool bInterpolate;

	/** How fast to interpolate */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Interpolation, meta=(EditCondition="bInterpolate", EditConditionHides))
	float InterpRate;
	
	/** Which interpolation function to use */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Interpolation, meta=(EditCondition="bInterpolate", EditConditionHides))
	EInterpFunc InterpType;

	float Interpolate(float Current, float Target, float DeltaTime, const EInterpFunc& InterpFunc) const;
};

/**
 * Parameters used for interpolation
 */
USTRUCT(BlueprintType)
struct PHYSICSHITREACT_API FInterpParams
{
	GENERATED_BODY()

	FInterpParams()
		: bEnableDecay(true)
		, bClampDecay(true)
		, MaxDecay(1.f)
		, DecayMode(EDecayMode::BlockInterpolation)
		, Scale(1.0f)
		, Bias(0.0f)
	{}

	FInterpParams(float InterpInRate, float InterpOutRate = 10.f, EInterpFunc InInterpType = EInterpFunc::FInterpTo,
		float InScale = 1.0f, float InBias = 0.0f)
		: InterpIn(InterpInRate, InInterpType)
		, InterpOut(InterpOutRate, InInterpType)
		, bEnableDecay(true)
		, bClampDecay(true)
		, MaxDecay(1.f)
		, DecayMode(EDecayMode::BlockInterpolation)
		, Scale(InScale)
		, Bias(InBias)
	{}

	/** Map interpolated value to InRange and OutRange */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Interpolation)
	FInterpMapRange MapRange;

	/** If true, clamp result to ClampMin and ClampMax */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Interpolation)
	FInterpClampRange ClampRange;

	/** Interpolation parameters when interpolating in */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Interpolation)
	FInterpValue InterpIn;

	/** Interpolation parameters when interpolating out */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Interpolation)
	FInterpValue InterpOut;

	/** Enable decay -- allows rewinding of the interpolation */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Interpolation)
	bool bEnableDecay;

	/** Clamp decay value */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Interpolation, meta=(EditCondition="bEnableDecay", EditConditionHides))
	bool bClampDecay;

	/** Clamp decay value */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Interpolation, meta=(EditCondition="bEnableDecay && bClampDecay", EditConditionHides))
	float MaxDecay;
	
	/** Interpolate parameters when adding decay (when interpolating in) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Interpolation, meta=(EditCondition="bEnableDecay", EditConditionHides))
	FInterpValue InterpDecay;
	
	/** How to handle interpolation when decay is present */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Interpolation, meta=(EditCondition="bEnableDecay", EditConditionHides))
	EDecayMode DecayMode;

	/** Scale the result */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Interpolation)
	float Scale;

	/** Bias the result */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Interpolation)
	float Bias;
};

/**
 * Interpolation state
 */
USTRUCT(BlueprintType)
struct PHYSICSHITREACT_API FInterpState
{
	GENERATED_BODY()

	FInterpState()
		: InterpolatedValue(0.f)
		, DecayValue(0.f)
		, LastTargetValue(0.f)
		, bInitialized(false)
		, bPaused(false)
	{}

protected:
	/** Current value */
	float InterpolatedValue;

	/** Decay value - when > 0, this will be smoothly deducted from InterpolatedValue */
	float DecayValue;
	
	/** Current value */
	float LastTargetValue;

public:
	/** Is initialized */
	bool bInitialized;

	/** Is currently paused */
	bool bPaused;

	float GetInterpolatedValue() const { return InterpolatedValue; }
	float GetLastTargetValue() const { return LastTargetValue; }
	
	bool HasCompleted(float Threshold = UE_KINDA_SMALL_NUMBER) const
	{
		return FMath::IsNearlyEqual(InterpolatedValue, LastTargetValue, Threshold);
	}

	bool IsInterpolating() const
	{
		return !HasCompleted();
	}

	bool IsInterpolatingIn() const
	{
		return InterpolatedValue <= LastTargetValue;
	}

	bool IsInterpolatingOut() const
	{
		return InterpolatedValue > LastTargetValue;
	}

	/** Apply scale, bias, and clamp to value */
	float ApplyTo(const FInterpParams& Params, float Target, float InDeltaTime);

	void ApplyDecay(const FInterpParams& Params, float DecayAmount);

	void ResetDecay()
	{
		DecayValue = 0.f;
	}

	void Reset();

	void SetPaused(bool bInPaused) { bPaused = bInPaused; }
};

/**
 * Interpolate a value
 * 
 * Usage:
 *		Declare:	FAlphaInterp Interpolation;
 *		Update:		Interpolation.Interpolate(TargetToggleAlpha, DeltaTime);
 *		Reset:		Interpolation.State.Reset()
 */
USTRUCT(BlueprintType)
struct PHYSICSHITREACT_API FAlphaInterp
{
	GENERATED_BODY()

	FAlphaInterp()
	{}

	/** Interpolation state */
	UPROPERTY()
	FInterpState State;

	/** Interpolation parameters to configure behaviour */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Interpolation)
	FInterpParams Params;

	/** Interpolate a value by calling on tick */
	float Interpolate(float TargetValue, float DeltaTime)
	{
		State.ApplyTo(Params, TargetValue, DeltaTime);
		return State.GetInterpolatedValue();
	}

	/** Add decay */
	void Decay(float DecayAmount)
	{
		State.ApplyDecay(Params, DecayAmount);
	}

	void RemoveAllDecay()
	{
		State.ResetDecay();
	}

	void Initialize(float InitializeTo = 0.f)
	{
		State.Reset();
		State.ApplyTo(Params, InitializeTo, 0.f);
	}

	void Finalize()
	{
		State.bInitialized = false;  // Disable interpolation for this frame
		State.ApplyTo(Params, State.GetLastTargetValue(), 0.f);
	}

	float GetInterpolatedValue() const
	{
		return State.GetInterpolatedValue();
	}

	float GetLastTargetValue() const
	{
		return State.GetLastTargetValue();
	}

	bool HasCompleted(float Threshold = UE_KINDA_SMALL_NUMBER) const
	{
		return State.HasCompleted(Threshold);
	}

	void Reset()
	{
		State.Reset();
	}

	bool IsActive() const
	{
		return State.bInitialized && !State.bPaused;
	}

	bool IsPaused() const
	{
		return State.bPaused;
	}
};