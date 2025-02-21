// Copyright (c) Jared Taylor


#include "Params/HitReactTrigger.h"


#include UE_INLINE_GENERATED_CPP_BY_NAME(HitReactTrigger)

FHitReactImpulse& FHitReactTrigger::GetImpulseParamsBase(const EHitReactImpulseType& ImpulseType)
{
	switch (ImpulseType)
	{
	case EHitReactImpulseType::Linear:
		return Impulse.LinearImpulse;
	case EHitReactImpulseType::Angular:
		return Impulse.AngularImpulse;
	case EHitReactImpulseType::Radial:
		return Impulse.RadialImpulse;
	}
	return Impulse.LinearImpulse;
}

const FHitReactImpulse& FHitReactTrigger::GetImpulseParamsBase(const EHitReactImpulseType& ImpulseType) const
{
	switch (ImpulseType)
	{
	case EHitReactImpulseType::Linear:
		return Impulse.LinearImpulse;
	case EHitReactImpulseType::Angular:
		return Impulse.AngularImpulse;
	case EHitReactImpulseType::Radial:
		return Impulse.RadialImpulse;
	}
	return Impulse.LinearImpulse;
}