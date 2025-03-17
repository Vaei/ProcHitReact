// Copyright (c) Jared Taylor


#include "Params/HitReactImpulse.h"

#include "HitReactProfile.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(HitReactImpulse)

FHitReactPendingImpulse::FHitReactPendingImpulse(const FHitReactImpulseParams& InImpulse,
	const FHitReactImpulse_WorldParams& InWorld, float InImpulseScalar,
	const TObjectPtr<const UHitReactProfile>& InProfile, FName InImpulseBoneName)
	: Impulse(InImpulse)
	, World(InWorld)
	, ImpulseScalar(InImpulseScalar)
	, Profile(InProfile)
	, ImpulseBoneName(InImpulseBoneName)
{}
