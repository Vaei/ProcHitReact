// Copyright (c) Jared Taylor. All Rights Reserved.


#include "HitReactTypes.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(HitReactTypes)

DEFINE_LOG_CATEGORY(LogHitReact);

TMap<FGameplayTag, FHitReactProfile> FHitReactBuiltInProfiles::GetBuiltInProfiles()
{
	auto Profiles = TMap<FGameplayTag, FHitReactProfile>();

	FHitReactProfile Default = {};
	FHitReactProfile NoArms = {};
	FHitReactProfile NoLegs = {};
	FHitReactProfile NoLimbs = {};
	FHitReactProfile TakeShot = {};
	FHitReactProfile TakeShotNoArms = {};
	FHitReactProfile Flop = {};
		
	// Make the character as floppy as possible, primarily used for testing purposes
	Flop.DefaultBoneApplyParams.MaxBlendWeight = 1.f;
	Flop.DefaultBoneApplyParams.PhysicsBlendParams = { 0.4f, 0.7f, EAlphaBlendOption::Sinusoidal };
	Flop.DefaultBoneApplyParams.PhysicsBlendParams.BlendHoldTime = 0.2f;
	Flop.DefaultBoneApplyParams.DecayExistingPhysics = 0.05f;

	Default.DefaultBoneApplyParams.Cooldown = 0.15f;
	Default.OverrideBoneParams = {
		{ TEXT("neck_01"), { true, false, 0.f, 0.3f } },
		{ TEXT("clavicle_l"), { false, false, 0.f, 0.2f } },
		{ TEXT("clavicle_r"), { false, false, 0.f, 0.2f } }
	};

	NoArms = Default;
	NoArms.OverrideBoneParams = {
		{ TEXT("neck_01"), { true, false, 0.f, 0.3f } },
		{ TEXT("clavicle_l"), { false, true, 0.f, 0.f } },
		{ TEXT("clavicle_r"), { false, true, 0.f, 0.f } }
	};

	NoLegs = Default;
	NoLegs.OverrideBoneParams = {
		{ TEXT("neck_01"), { true, false, 0.f, 0.3f } },
		{ TEXT("thigh_l"), { true, true, 0.f, 0.f } },
		{ TEXT("thigh_r"), { true, true, 0.f, 0.f } }
	};

	NoLimbs = Default;
	NoLimbs.OverrideBoneParams = {
		{ TEXT("neck_01"), { true, false, 0.f, 0.3f } },
		{ TEXT("clavicle_l"), { false, true, 0.f, 0.f } },
		{ TEXT("clavicle_r"), { false, true, 0.f, 0.f } },
		{ TEXT("thigh_l"), { true, true, 0.f, 0.f } },
		{ TEXT("thigh_r"), { true, true, 0.f, 0.f } }
	};

	TakeShot = Default;
	TakeShot.DefaultBoneApplyParams.Cooldown = 0.035f;
	
	TakeShotNoArms = TakeShot;
	TakeShotNoArms.OverrideBoneParams = {
		{ TEXT("neck_01"), { true, false, 0.f, 0.3f } },
		{ TEXT("clavicle_l"), { false, true, 0.f, 0.f } },
		{ TEXT("clavicle_r"), { false, true, 0.f, 0.f } }
	};

	// Assign the profiles
	Profiles = {
		{ FHitReactTags::HitReact_Profile_Default, Default },
		{ FHitReactTags::HitReact_Profile_Default_NoArms, NoArms },
		{ FHitReactTags::HitReact_Profile_Default_NoLegs, NoLegs },
		{ FHitReactTags::HitReact_Profile_Default_NoLimbs, NoLimbs },
		{ FHitReactTags::HitReact_Profile_TakeShot, TakeShot },
		{ FHitReactTags::HitReact_Profile_TakeShot_NoArms, TakeShotNoArms },
		{ FHitReactTags::HitReact_Profile_Flop, Flop }
	};
		
	return Profiles;
}
