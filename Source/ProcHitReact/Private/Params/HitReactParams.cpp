// Copyright (c) Jared Taylor. All Rights Reserved.


#include "Params/HitReactParams.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(HitReactParams)

TMap<FGameplayTag, FHitReactProfile> FHitReactBuiltInProfiles::GetBuiltInProfiles()
{
	return {
		{ FHitReactTags::HitReact_Profile_Default, BuiltInProfile_Default() },
		{ FHitReactTags::HitReact_Profile_Default_NoArms, BuiltInProfile_Default_NoArms() },
		{ FHitReactTags::HitReact_Profile_Default_NoLegs, BuiltInProfile_Default_NoLegs() },
		{ FHitReactTags::HitReact_Profile_Default_NoLimbs, BuiltInProfile_Default_NoLimbs() },
		{ FHitReactTags::HitReact_Profile_TakeHit, BuiltInProfile_TakeHit() },
		{ FHitReactTags::HitReact_Profile_TakeHit_NoArms, BuiltInProfile_TakeHit_NoArms() },
		{ FHitReactTags::HitReact_Profile_TakeHit_NoLegs, BuiltInProfile_TakeHit_NoLegs() },
		{ FHitReactTags::HitReact_Profile_TakeHit_NoLimbs, BuiltInProfile_TakeHit_NoLimbs() },
		{ FHitReactTags::HitReact_Profile_Twitch, BuiltInProfile_Twitch() },
		{ FHitReactTags::HitReact_Profile_Twitch_NoArms, BuiltInProfile_Twitch_NoArms() },
		{ FHitReactTags::HitReact_Profile_Twitch_NoLegs, BuiltInProfile_Twitch_NoLegs() },
		{ FHitReactTags::HitReact_Profile_Twitch_NoLimbs, BuiltInProfile_Twitch_NoLimbs() },
		{ FHitReactTags::HitReact_Profile_BumpPawn, BuiltInProfile_BumpPawn() },
		{ FHitReactTags::HitReact_Profile_Flop, BuiltInProfile_Flop() }
	};
}

FHitReactProfile FHitReactBuiltInProfiles::BuiltInProfile_Default()
{
	// Default profile
	FHitReactProfile Profile = {};
	Profile.OverrideBoneParams = {
		{ TEXT("neck_01"), { true, false, 0.f, 0.3f } },
		{ TEXT("clavicle_l"), { true, false, 0.f, 0.2f } },
		{ TEXT("clavicle_r"), { true, false, 0.f, 0.2f } }
	};
	return Profile;
}

FHitReactProfile FHitReactBuiltInProfiles::BuiltInProfile_Default_NoArms()
{
	// No arms profile
	FHitReactProfile Profile = BuiltInProfile_Default();
	Profile.OverrideBoneParams = {
		{ TEXT("neck_01"), { true, false, 0.f, 0.3f } },
		{ TEXT("clavicle_l"), { true, true, 0.f, 0.f } },
		{ TEXT("clavicle_r"), { true, true, 0.f, 0.f } }
	};
	return Profile;
}

FHitReactProfile FHitReactBuiltInProfiles::BuiltInProfile_Default_NoLegs()
{
	// No legs profile
	FHitReactProfile Profile = BuiltInProfile_Default();
	Profile.OverrideBoneParams = {
		{ TEXT("neck_01"), { true, false, 0.f, 0.3f } },
		{ TEXT("thigh_l"), { true, true, 0.f, 0.f } },
		{ TEXT("thigh_r"), { true, true, 0.f, 0.f } }
	};
	return Profile;
}

FHitReactProfile FHitReactBuiltInProfiles::BuiltInProfile_Default_NoLimbs()
{
	// No limbs profile
	FHitReactProfile Profile = BuiltInProfile_Default();
	Profile.OverrideBoneParams = {
		{ TEXT("neck_01"), { true, false, 0.f, 0.3f } },
		{ TEXT("clavicle_l"), { true, true, 0.f, 0.f } },
		{ TEXT("clavicle_r"), { true, true, 0.f, 0.f } },
		{ TEXT("thigh_l"), { true, true, 0.f, 0.f } },
		{ TEXT("thigh_r"), { true, true, 0.f, 0.f } }
	};
	return Profile;
}

FHitReactProfile FHitReactBuiltInProfiles::BuiltInProfile_TakeHit()
{
	FHitReactProfile Profile = {};
	Profile.DefaultBoneApplyParams.PhysicsBlendParams.BlendIn.BlendTime = 0.25f;
	Profile.DefaultBoneApplyParams.PhysicsBlendParams.BlendHoldTime = 0.15f;
	Profile.DefaultBoneApplyParams.PhysicsBlendParams.BlendOut.BlendTime = 0.5f;
	Profile.DefaultBoneApplyParams.PhysicsBlendParams.DecayTime = 0.2f;
	Profile.DefaultBoneApplyParams.PhysicsBlendParams.MaxAccumulatedDecayTime = 0.7f;
	Profile.DefaultBoneApplyParams.PhysicsBlendParams.BlendIn.BlendOption = EAlphaBlendOption::CircularIn;
	Profile.DefaultBoneApplyParams.PhysicsBlendParams.BlendOut.BlendOption = EAlphaBlendOption::CircularOut;
	Profile.DefaultBoneApplyParams.MaxBlendWeight = 0.3f;
	Profile.DefaultBoneApplyParams.Cooldown = 0.35f;
	Profile.DefaultBoneApplyParams.DecayExistingPhysics = 0.25f;
	Profile.OverrideBoneParams = {};
	return Profile;
}

FHitReactProfile FHitReactBuiltInProfiles::BuiltInProfile_TakeHit_NoArms()
{
	FHitReactProfile Profile = BuiltInProfile_TakeHit();
	Profile.OverrideBoneParams = {
		{ TEXT("neck_01"), { true, false, 0.f, 0.3f } },
		{ TEXT("clavicle_l"), { true, true, 0.f, 0.f } },
		{ TEXT("clavicle_r"), { true, true, 0.f, 0.f } }
	};
	return Profile;
}

FHitReactProfile FHitReactBuiltInProfiles::BuiltInProfile_TakeHit_NoLegs()
{
	FHitReactProfile Profile = BuiltInProfile_TakeHit();
	Profile.OverrideBoneParams = {
		{ TEXT("neck_01"), { true, false, 0.f, 0.3f } },
		{ TEXT("thigh_l"), { true, true, 0.f, 0.f } },
		{ TEXT("thigh_r"), { true, true, 0.f, 0.f } }
	};
	return Profile;
}

FHitReactProfile FHitReactBuiltInProfiles::BuiltInProfile_TakeHit_NoLimbs()
{
	FHitReactProfile Profile = BuiltInProfile_TakeHit();
	Profile.OverrideBoneParams = {
		{ TEXT("neck_01"), { true, false, 0.f, 0.3f } },
		{ TEXT("clavicle_l"), { true, true, 0.f, 0.f } },
		{ TEXT("clavicle_r"), { true, true, 0.f, 0.f } },
		{ TEXT("thigh_l"), { true, true, 0.f, 0.f } },
		{ TEXT("thigh_r"), { true, true, 0.f, 0.f } }
	};
	return Profile;
}

FHitReactProfile FHitReactBuiltInProfiles::BuiltInProfile_Twitch()
{
	FHitReactProfile Profile = {};
	Profile.DefaultBoneApplyParams.MaxBlendWeight = 0.2f;
	Profile.DefaultBoneApplyParams.PhysicsBlendParams.DecayTime = 0.15f;
	Profile.DefaultBoneApplyParams.PhysicsBlendParams.MaxAccumulatedDecayTime = 0.35f;
	Profile.DefaultBoneApplyParams.PhysicsBlendParams.BlendOut.BlendTime = 0.27f;
	Profile.DefaultBoneApplyParams.DecayExistingPhysics = 0.1f;
	Profile.DefaultBoneApplyParams.Cooldown = 0.035f;
	Profile.OverrideBoneParams = {
		{ TEXT("neck_01"), { true, false, 0.f, 0.15f } },
		{ TEXT("clavicle_l"), { true, false, 0.f, 0.12f } },
		{ TEXT("clavicle_r"), { true, false, 0.f, 0.12f } }
	};
	return Profile;
}

FHitReactProfile FHitReactBuiltInProfiles::BuiltInProfile_Twitch_NoArms()
{ 
	FHitReactProfile Profile = BuiltInProfile_Twitch();
	Profile.OverrideBoneParams = {
		{ TEXT("neck_01"), { true, false, 0.f, 0.15f } },
		{ TEXT("clavicle_l"), { true, true, 0.f, 0.f } },
		{ TEXT("clavicle_r"), { true, true, 0.f, 0.f } }
	};
	return Profile;
}

FHitReactProfile FHitReactBuiltInProfiles::BuiltInProfile_Twitch_NoLegs()
{
	FHitReactProfile Profile = BuiltInProfile_Twitch();
	Profile.OverrideBoneParams = {
		{ TEXT("neck_01"), { true, false, 0.f, 0.15f } },
		{ TEXT("thigh_l"), { true, true, 0.f, 0.f } },
		{ TEXT("thigh_r"), { true, true, 0.f, 0.f } }
	};
	return Profile;
}

FHitReactProfile FHitReactBuiltInProfiles::BuiltInProfile_Twitch_NoLimbs()
{
	FHitReactProfile Profile = BuiltInProfile_Twitch();
	Profile.OverrideBoneParams = {
		{ TEXT("neck_01"), { true, false, 0.f, 0.15f } },
		{ TEXT("clavicle_l"), { true, true, 0.f, 0.f } },
		{ TEXT("clavicle_r"), { true, true, 0.f, 0.f } },
		{ TEXT("thigh_l"), { true, true, 0.f, 0.f } },
		{ TEXT("thigh_r"), { true, true, 0.f, 0.f } }
	};
	return Profile;
}

FHitReactProfile FHitReactBuiltInProfiles::BuiltInProfile_BumpPawn()
{
	FHitReactProfile Profile = {};
	Profile.DefaultBoneApplyParams.PhysicsBlendParams.BlendHoldTime = 0.2f;
	Profile.DefaultBoneApplyParams.PhysicsBlendParams.BlendOut.BlendTime = 0.7f;
	Profile.DefaultBoneApplyParams.PhysicsBlendParams.DecayTime = 0.2f;
	Profile.DefaultBoneApplyParams.PhysicsBlendParams.MaxAccumulatedDecayTime = 0.45f;
	Profile.DefaultBoneApplyParams.PhysicsBlendParams.BlendIn.BlendOption = EAlphaBlendOption::CircularIn;
	Profile.DefaultBoneApplyParams.PhysicsBlendParams.BlendOut.BlendOption = EAlphaBlendOption::CircularOut;
	Profile.DefaultBoneApplyParams.MaxBlendWeight = 0.3f;
	Profile.DefaultBoneApplyParams.Cooldown = 0.35f;
	Profile.DefaultBoneApplyParams.DecayExistingPhysics = 0.1f;
	Profile.OverrideBoneParams = {};
	return Profile;
}

FHitReactProfile FHitReactBuiltInProfiles::BuiltInProfile_Flop()
{
	// Make the character as floppy as possible, primarily used for testing purposes
	FHitReactProfile Profile = {};
	Profile.DefaultBoneApplyParams.MaxBlendWeight = 1.f;
	Profile.DefaultBoneApplyParams.PhysicsBlendParams = { 0.4f, 0.7f, EAlphaBlendOption::Sinusoidal };
	Profile.DefaultBoneApplyParams.PhysicsBlendParams.BlendHoldTime = 0.2f;
	Profile.DefaultBoneApplyParams.DecayExistingPhysics = 0.05f;
	return Profile;
}