// Copyright (c) Jared Taylor. All Rights Reserved


#include "HitReactImpulseParams.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(HitReactImpulseParams)

FHitReactImpulseParamsBase& FHitReactApplyParams::GetImpulseParamsBase(const EHitReactImpulseType& ImpulseType)
{
	switch (ImpulseType)
	{
	case EHitReactImpulseType::Linear:
		return ImpulseParams.LinearImpulse;
	case EHitReactImpulseType::Angular:
		return ImpulseParams.AngularImpulse;
	case EHitReactImpulseType::Radial:
		return ImpulseParams.RadialImpulse;
	}
	return ImpulseParams.LinearImpulse;
}

const FHitReactImpulseParamsBase& FHitReactApplyParams::GetImpulseParamsBase(
	const EHitReactImpulseType& ImpulseType) const
{
	switch (ImpulseType)
	{
	case EHitReactImpulseType::Linear:
		return ImpulseParams.LinearImpulse;
	case EHitReactImpulseType::Angular:
		return ImpulseParams.AngularImpulse;
	case EHitReactImpulseType::Radial:
		return ImpulseParams.RadialImpulse;
	}
	return ImpulseParams.LinearImpulse;
}
