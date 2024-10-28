﻿// Copyright (c) Jared Taylor. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "HitReactPhysicsState.generated.h"

UENUM(BlueprintType)
enum class EHitReactBlendState : uint8
{
	Pending			UMETA(ToolTip="Pending - HitReact has not yet started"),
	BlendIn			UMETA(ToolTip="Blend in - When a HitReact is first applied, we use this to go from 0 to 1"),
	BlendHold		UMETA(ToolTip="Blend hold - When BlendIn completes, we hold here, fully blended, for a period of time"),
	BlendOut		UMETA(ToolTip="Blend out - When BlendHold completes, we use this to go from 1 to 0"),
	Completed		UMETA(ToolTip="Completed - HitReact has completed and is no longer active")
};

USTRUCT(BlueprintType)
struct PHYSICSHITREACT_API FHitReactBlendParams
{
	GENERATED_BODY()

	FHitReactBlendParams(float InBlendTime = 0.2f, const EAlphaBlendOption& InBlendOption = EAlphaBlendOption::Linear,
		const TObjectPtr<UCurveFloat>& InCustomCurve = nullptr)
		: BlendTime(InBlendTime)
		, BlendOption(InBlendOption)
		, CustomCurve(InCustomCurve)
	{}
	
	/**
	 * Blend Time
	 * Set to 0 to disable blending
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=HitReact, meta=(ClampMin="0", UIMin="0", UIMax="1", ForceUnits="s"))
	float BlendTime;

	/** Type of blending used (Linear, Cubic, etc.) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=HitReact, meta=(EditCondition="BlendTime > 0"))
	EAlphaBlendOption BlendOption;
	
	/** If you're using Custom BlendOption, you can specify curve */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=HitReact, meta=(DisplayAfter="BlendOption", EditCondition="BlendTime > 0 && BlendOption == EAlphaBlendOption::Custom"))
	TObjectPtr<UCurveFloat> CustomCurve;

	bool IsValid() const
	{
		return BlendTime > SMALL_NUMBER;
	}

	float Ease(float InAlpha) const
	{
		return FAlphaBlend::AlphaToBlendOption(InAlpha, BlendOption, CustomCurve.Get());
	}
};

/**
 * Interpolation state handling for hit reactions
 * Supports blend in, hold, and blend out
 */
USTRUCT(BlueprintType)
struct PHYSICSHITREACT_API FHitReactPhysicsStateParams
{
	GENERATED_BODY()

	FHitReactPhysicsStateParams(float InBlendInTime = 0.2f, float InBlendOutTime = 0.2f, const EAlphaBlendOption& InBlendOption = EAlphaBlendOption::HermiteCubic)
		: BlendIn(InBlendInTime, InBlendOption)
		, BlendHoldTime(0.f)
		, BlendOut(InBlendOutTime, InBlendOption)
		, DecayTime(0.05f)
		, MaxAccumulatedDecayTime(0.25f)
	{}
	
	/**
	 * Simulated physics blend in
	 * When a HitReact is first applied, we use this to go from 0 to 1
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=HitReact)
	FHitReactBlendParams BlendIn;

	/**
	 * Simulated physics blend hold
	 * When BlendIn completes, we hold here, fully blended, for a period of time
	 * Set to 0 to disable hold
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=HitReact, meta=(ClampMin="0", UIMin="0", UIMax="1", ForceUnits="s"))
	float BlendHoldTime;

	/** Simulated physics blend out */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=HitReact)
	FHitReactBlendParams BlendOut;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=HitReact, meta=(ClampMin="0", UIMin="0", UIMax="1", ForceUnits="s"))
	float DecayTime;

	/**
	 * Maximum delay that can accumulate
	 * Will not exceed the accumulation of all blend times regardless
	 * Will not exceed the current elapsed state time
	 * Set to 0 to disable this clamp
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=HitReact, meta=(ClampMin="0", UIMin="0", UIMax="1", ForceUnits="s"))
	float MaxAccumulatedDecayTime;

	float GetTotalTime() const
	{
		return BlendIn.BlendTime + BlendHoldTime + BlendOut.BlendTime;
	}
};

/**
 * Interpolation state handling for hit reactions
 * Supports blend in, hold, and blend out
 */
USTRUCT(BlueprintType)
struct PHYSICSHITREACT_API FHitReactPhysicsState
{
	GENERATED_BODY()

	FHitReactPhysicsState()
		: BlendState(EHitReactBlendState::Pending)
		, ElapsedTime(0.f)
		, DecayTime(0.f)
	{}

	UPROPERTY()
	FHitReactPhysicsStateParams Params;

protected:
	/** Current state of the HitReact */
	UPROPERTY(Transient)
	EHitReactBlendState BlendState;

	/** Range of 0 to GetTotalTime() */
	UPROPERTY(Transient)
	float ElapsedTime;

	/** Decay is applied when the HitReact is reapplied, effectively an offset applied over time */
	UPROPERTY(Transient)
	float DecayTime;

	/** Update the State based on the elapsed time */
	void UpdateBlendState();

public:
	/** @return Current state of the HitReact */
	EHitReactBlendState GetBlendState() const
	{
		return BlendState;
	}

	FString GetBlendStateString() const;

	/** @return Current elapsed time */
	float GetElapsedTime() const
	{
		return ElapsedTime;
	}
	
	/** @return Total time for the entire blend */
	float GetTotalTime() const
	{
		return Params.BlendIn.BlendTime + Params.BlendHoldTime + Params.BlendOut.BlendTime;
	}

	/** @return True if the HitReact has started */
	bool HasStarted() const
	{
		return BlendState != EHitReactBlendState::Pending;
	}

	/** @return True if the HitReact has completed */
	bool HasCompleted() const
	{
		return BlendState == EHitReactBlendState::Completed;
	}

	/** @return True if the HitReact is active */
	bool IsActive() const
	{
		return BlendState != EHitReactBlendState::Pending && BlendState != EHitReactBlendState::Completed;
	}

	void Reset();

	/**
	 * Activate the HitReact
	 * @return True if activated, False if already active
	 */
	bool TryActivate();

	void Finish();

	float GetBlendTime() const;

	void Decay(float Time)
	{
		DecayTime += Time;
		DecayTime = FMath::Clamp<float>(DecayTime, 0.f, Params.MaxAccumulatedDecayTime);
	}

	bool IsDecaying() const
	{
		return DecayTime > 0.f;
	}

	/** Directly set the elapsed time and Update the State */
	void SetElapsedTime(float InElapsedTime);

	/** @return Total time for the current state */
	float GetTotalStateTime() const;

	/** @return Total time elapsed for the current state */
	float GetElapsedStateTime() const;

	/** @return Alpha value for the current state */
	float GetBlendStateAlpha() const;

	/** @return Blend params for the current state */
	const FHitReactBlendParams* GetBlendParams() const;

	/**
	 * Called every frame to update the state
	 * @return True if completed and ready to disable, remove, uninitialize, etc.
	 */
	bool Tick(float DeltaTime);
};

USTRUCT(BlueprintType)
struct PHYSICSHITREACT_API FHitReactPhysicsStateParamsSimple
{
	GENERATED_BODY()

	FHitReactPhysicsStateParamsSimple(float InBlendInTime = 0.25f, float InBlendOutTime = 0.25f, const EAlphaBlendOption& InBlendOption = EAlphaBlendOption::HermiteCubic)
		: BlendIn(InBlendInTime, InBlendOption)
		, BlendOut(InBlendOutTime, InBlendOption)
	{}

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=HitReact)
	FHitReactBlendParams BlendIn;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=HitReact)
	FHitReactBlendParams BlendOut;
};

/**
 * Simple interpolation state handling for hit reaction global toggle
 * Supports blend in and blend out
 */
USTRUCT(BlueprintType)
struct PHYSICSHITREACT_API FHitReactPhysicsStateSimple
{
	GENERATED_BODY()

	FHitReactPhysicsStateSimple()
		: bToggleEnabled(false)
		, ElapsedTime(0.f)
	{}

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=HitReact)
	FHitReactPhysicsStateParamsSimple BlendParams;
	
	UPROPERTY(Transient)
	bool bToggleEnabled;

	UPROPERTY(Transient)
	float ElapsedTime;

	void Initialize(bool bStartEnabled)
	{
		bToggleEnabled = bStartEnabled;
		ElapsedTime = GetStateTime();
	}

	float GetTargetAlpha() const
	{
		return bToggleEnabled ? 1.f : 0.f;
	}

	bool HasCompleted() const
	{
		return bToggleEnabled ? ElapsedTime >= BlendParams.BlendIn.BlendTime : ElapsedTime <= 0.f;
	}

	const FHitReactBlendParams& GetBlendParams() const
	{
		return bToggleEnabled ? BlendParams.BlendIn : BlendParams.BlendOut;
	}

	float GetStateTime() const
	{
		return GetBlendParams().BlendTime;
	}

	float GetStateAlpha() const
	{
		const float Alpha = ElapsedTime / GetStateTime();
		return FMath::Clamp<float>(Alpha, 0.f, 1.f);
	}

	float GetBlendStateAlpha() const
	{
		const float Alpha = GetStateAlpha();
		return FMath::Clamp<float>(GetBlendParams().Ease(Alpha), 0.f, 1.f);
	}

	void SetElapsedTime(float InElapsedTime)
	{
		ElapsedTime = FMath::Clamp<float>(InElapsedTime, 0.f, GetStateTime());
	}

	/** @return True if reached our target */
	bool Tick(float DeltaTime);
};