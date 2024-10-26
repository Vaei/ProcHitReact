// Copyright (c) Jared Taylor. All Rights Reserved


#include "HitReactImpulseParams.h"

#include "HitReactTags.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(HitReactImpulseParams)

FHitReactApplyParams::FHitReactApplyParams()
	: ProfileToUse(FHitReactTags::HitReact_Profile_Default)
	, BoneName(NAME_None)
    , bIncludeSelf(true)
{}

bool FHitReactApplyParams::IsValidToApply() const
{
	return ProfileToUse.IsValid() && BoneName != NAME_None;
}

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
