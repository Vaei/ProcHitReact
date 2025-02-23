// Copyright (c) Jared Taylor


#include "Physics/HitReactPhysicsState.h"

#include "HitReactProfile.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(HitReactPhysicsState)

void FHitReactPhysicsState::UpdateBlendState()
{
	if (BlendState == EHitReactBlendState::Completed)
	{
		return;
	}

	if (ElapsedTime < Params.BlendIn.BlendTime)
	{
		BlendState = EHitReactBlendState::BlendIn;
	}
	else if (ElapsedTime < Params.BlendIn.BlendTime + Params.BlendHoldTime)
	{
		BlendState = EHitReactBlendState::BlendHold;
	}
	else if (ElapsedTime < GetTotalTime())
	{
		BlendState = EHitReactBlendState::BlendOut;
	}
	else
	{
		BlendState = EHitReactBlendState::Completed;
	}
}

FString FHitReactPhysicsState::GetBlendStateString() const
{
	switch (BlendState)
	{
		case EHitReactBlendState::Pending: return "Pending";
		case EHitReactBlendState::BlendIn: return "BlendIn";
		case EHitReactBlendState::BlendHold: return "BlendHold";
		case EHitReactBlendState::BlendOut: return "BlendOut";
		case EHitReactBlendState::Completed: return "Completed";
		default: return "Unknown";
	}
}

void FHitReactPhysicsState::Reset()
{
	BlendState = EHitReactBlendState::Pending;
	ElapsedTime = 0.f;
}

bool FHitReactPhysicsState::CanActivate(const FHitReactPhysicsStateParams& WithParams)
{
	// Avoid division by zero and non-sane values
	if (WithParams.GetTotalTime() <= 0.f)
	{
		return false;
	}

	return true;
}

void FHitReactPhysicsState::Activate()
{
	BlendState = EHitReactBlendState::BlendIn;
	ElapsedTime = 0.f;
}

void FHitReactPhysicsState::Finish()
{
	BlendState = EHitReactBlendState::Completed;
	ElapsedTime = GetTotalTime();
}

float FHitReactPhysicsState::GetBlendTime() const
{
	switch (BlendState)
	{
	case EHitReactBlendState::Pending: return 0.f;
	case EHitReactBlendState::BlendIn: return Params.BlendIn.BlendTime;
	case EHitReactBlendState::BlendHold: return Params.BlendHoldTime;
	case EHitReactBlendState::BlendOut: return Params.BlendOut.BlendTime;
	case EHitReactBlendState::Completed: return 0.f;
	default: return 0.f;
	}
}

void FHitReactPhysicsState::SetElapsedTime(float InElapsedTime)
{
	ElapsedTime = FMath::Clamp<float>(InElapsedTime, 0.f, GetTotalTime());
	UpdateBlendState();
}

float FHitReactPhysicsState::GetTotalStateTime() const
{
	switch (BlendState)
	{
	case EHitReactBlendState::BlendIn:
		return Params.BlendIn.BlendTime;
	case EHitReactBlendState::BlendHold:
		return Params.BlendHoldTime;
	case EHitReactBlendState::BlendOut:
		return Params.BlendOut.BlendTime;
	default:
		return 0.f;
	}
}

float FHitReactPhysicsState::GetElapsedStateTime() const
{
	switch (BlendState)
	{
	case EHitReactBlendState::BlendIn:
		return FMath::Clamp(ElapsedTime, 0.f, Params.BlendIn.BlendTime);
	case EHitReactBlendState::BlendHold:
		return FMath::Clamp(ElapsedTime - Params.BlendIn.BlendTime, 0.f, Params.BlendHoldTime);
	case EHitReactBlendState::BlendOut:
		return FMath::Clamp(ElapsedTime - Params.BlendIn.BlendTime - Params.BlendHoldTime, 0.f, Params.BlendOut.BlendTime);
	default:
		return 0.f;
	}
}

void FHitReactPhysicsState::SetElapsedAlpha(float InAlpha)
{
	const float TotalStateTime = GetTotalTime();
	if (TotalStateTime <= 0.f)
	{
		return;
	}
	SetElapsedTime(InAlpha * TotalStateTime);
}

float FHitReactPhysicsState::GetBlendStateAlpha() const
{
	const float ElapsedStateTime = GetElapsedStateTime();
	const float TotalStateTime = GetTotalStateTime();
	if (TotalStateTime <= 0.f)
	{
		return 0.f;
	}
	const float Alpha = ElapsedStateTime / TotalStateTime;

	// Perform our easing function here
	if (const FHitReactBlendParams* BlendParams = GetBlendParams())
	{ 
		return FMath::Clamp<float>(BlendParams->Ease(Alpha), 0.f, 1.f);
	}
	return Alpha;
}

float FHitReactPhysicsState::GetElapsedAlpha() const
{
	const float TotalStateTime = GetTotalTime();
	if (TotalStateTime <= 0.f)
	{
		return 0.f;
	}
	return FMath::Clamp<float>(ElapsedTime / TotalStateTime, 0.f, 1.f);
}

const FHitReactBlendParams* FHitReactPhysicsState::GetBlendParams() const
{
	switch (BlendState)
	{
		case EHitReactBlendState::BlendIn: return &Params.BlendIn;
		case EHitReactBlendState::BlendOut: return &Params.BlendOut;
		default: return nullptr;
	}
}

bool FHitReactPhysicsState::Tick(float DeltaTime)
{
	// Nothing to do if we're completed
	if (HasCompleted())
	{
		return true;
	}

	// Process the decay state
	if (IsDecaying())
	{
		const float PrevDecayTime = DecayTime;

		// We want to retain the remaining delta time after decay is clamped, otherwise we'll lose time
		const float TargetDecayTime = DecayTime - DeltaTime;

		// Clamp the decay time
		DecayTime = FMath::Clamp<float>(TargetDecayTime, 0.f, Params.MaxAccumulatedDecayTime);

		// Calculate the remaining delta time after decay
		DeltaTime = FMath::Max<float>(0.f, TargetDecayTime - DecayTime);

		// Update the elapsed time to account for the decay
		const float DecayDelta = PrevDecayTime - DecayTime;
		SetElapsedTime(ElapsedTime - DecayDelta);

		if (!IsDecaying())
		{
			// ReSharper disable once CppExpressionWithoutSideEffects
			OnDecayComplete.ExecuteIfBound();
		}

		// DeltaTime is now the remaining time after decay, but may be zero
		if (DeltaTime < 1e-6f)
		{
			return HasCompleted();
		}
	}

	// Process the blend states
	if (!IsDecaying())
	{
		SetElapsedTime(ElapsedTime + DeltaTime);
	}

	// Notify if we've completed
	return HasCompleted();
}

bool FHitReactPhysicsStateSimple::Tick(float DeltaTime)
{
	if (HasCompleted())
	{
		return true;
	}
		
	if (bToggleEnabled)
	{
		SetElapsedTime(ElapsedTime + DeltaTime);
	}
	else
	{
		SetElapsedTime(ElapsedTime - DeltaTime);
	}
	
	return HasCompleted();
}
