// Copyright (c) Jared Taylor. All Rights Reserved.


#include "Params/HitReactParams.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(HitReactParams)

TMap<FGameplayTag, FHitReactProfile> FHitReactBuiltInProfiles::GetBuiltInProfiles()
{
	auto Profiles = TMap<FGameplayTag, FHitReactProfile>();

	FHitReactProfile Default = {};
	FHitReactProfile NoArms = {};
	FHitReactProfile NoLegs = {};
	FHitReactProfile NoLimbs = {};
	FHitReactProfile BumpPawn = {};
	FHitReactProfile TakeShot = {};
	FHitReactProfile TakeShotNoArms = {};
	FHitReactProfile Flop = {};
		
	// Make the character as floppy as possible, primarily used for testing purposes
	Flop.DefaultBoneApplyParams.MaxBlendWeight = 1.f;
	Flop.DefaultBoneApplyParams.PhysicsBlendParams = { 0.4f, 0.7f, EAlphaBlendOption::Sinusoidal };
	Flop.DefaultBoneApplyParams.PhysicsBlendParams.BlendHoldTime = 0.2f;
	Flop.DefaultBoneApplyParams.DecayExistingPhysics = 0.05f;

	// Default profile
	Default.OverrideBoneParams = {
		{ TEXT("neck_01"), { true, false, 0.f, 0.3f } },
		{ TEXT("clavicle_l"), { true, false, 0.f, 0.2f } },
		{ TEXT("clavicle_r"), { true, false, 0.f, 0.2f } }
	};

	// No arms profile
	NoArms = Default;
	NoArms.OverrideBoneParams = {
		{ TEXT("neck_01"), { true, false, 0.f, 0.3f } },
		{ TEXT("clavicle_l"), { true, true, 0.f, 0.f } },
		{ TEXT("clavicle_r"), { true, true, 0.f, 0.f } }
	};

	// No legs profile
	NoLegs = Default;
	NoLegs.OverrideBoneParams = {
		{ TEXT("neck_01"), { true, false, 0.f, 0.3f } },
		{ TEXT("thigh_l"), { true, true, 0.f, 0.f } },
		{ TEXT("thigh_r"), { true, true, 0.f, 0.f } }
	};

	// No limbs profile
	NoLimbs = Default;
	NoLimbs.OverrideBoneParams = {
		{ TEXT("neck_01"), { true, false, 0.f, 0.3f } },
		{ TEXT("clavicle_l"), { true, true, 0.f, 0.f } },
		{ TEXT("clavicle_r"), { true, true, 0.f, 0.f } },
		{ TEXT("thigh_l"), { true, true, 0.f, 0.f } },
		{ TEXT("thigh_r"), { true, true, 0.f, 0.f } }
	};

	// Bump pawn profile
	BumpPawn = Default;
	BumpPawn.DefaultBoneApplyParams.PhysicsBlendParams.BlendHoldTime = 0.2f;
	BumpPawn.DefaultBoneApplyParams.PhysicsBlendParams.BlendOut.BlendTime = 0.7f;
	BumpPawn.DefaultBoneApplyParams.PhysicsBlendParams.DecayTime = 0.2f;
	BumpPawn.DefaultBoneApplyParams.PhysicsBlendParams.MaxAccumulatedDecayTime = 0.45f;
	BumpPawn.DefaultBoneApplyParams.PhysicsBlendParams.BlendIn.BlendOption = EAlphaBlendOption::CircularIn;
	BumpPawn.DefaultBoneApplyParams.PhysicsBlendParams.BlendOut.BlendOption = EAlphaBlendOption::CircularOut;
	BumpPawn.DefaultBoneApplyParams.MaxBlendWeight = 0.3f;
	BumpPawn.DefaultBoneApplyParams.Cooldown = 0.35f;
	BumpPawn.DefaultBoneApplyParams.DecayExistingPhysics = 0.1f;
	BumpPawn.OverrideBoneParams = {};

	// Take shot profile
	TakeShot = Default;
	TakeShot.DefaultBoneApplyParams.MaxBlendWeight = 0.2f;
	TakeShot.DefaultBoneApplyParams.PhysicsBlendParams.DecayTime = 0.15f;
	TakeShot.DefaultBoneApplyParams.PhysicsBlendParams.MaxAccumulatedDecayTime = 0.35f;
	TakeShot.DefaultBoneApplyParams.PhysicsBlendParams.BlendOut.BlendTime = 0.27f;
	TakeShot.DefaultBoneApplyParams.DecayExistingPhysics = 0.1f;
	TakeShot.DefaultBoneApplyParams.Cooldown = 0.035f;
	TakeShot.OverrideBoneParams = {
		{ TEXT("neck_01"), { true, false, 0.f, 0.15f } },
		{ TEXT("clavicle_l"), { true, false, 0.f, 0.12f } },
		{ TEXT("clavicle_r"), { true, false, 0.f, 0.12f } }
	};

	// Take shot no arms profile
	TakeShotNoArms = TakeShot;
	TakeShotNoArms.OverrideBoneParams = {
		{ TEXT("neck_01"), { true, false, 0.f, 0.15f } },
		{ TEXT("clavicle_l"), { true, true, 0.f, 0.f } },
		{ TEXT("clavicle_r"), { true, true, 0.f, 0.f } }
	};

	// Assign the profiles
	Profiles = {
		{ FHitReactTags::HitReact_Profile_Default, Default },
		{ FHitReactTags::HitReact_Profile_Default_NoArms, NoArms },
		{ FHitReactTags::HitReact_Profile_Default_NoLegs, NoLegs },
		{ FHitReactTags::HitReact_Profile_Default_NoLimbs, NoLimbs },
		{ FHitReactTags::HitReact_Profile_BumpPawn, BumpPawn },
		{ FHitReactTags::HitReact_Profile_TakeShot, TakeShot },
		{ FHitReactTags::HitReact_Profile_TakeShot_NoArms, TakeShotNoArms },
		{ FHitReactTags::HitReact_Profile_Flop, Flop }
	};

	// Return the profiles
	return Profiles;
}
