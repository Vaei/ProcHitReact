// Copyright (c) Jared Taylor


#include "HitReactTags.h"

namespace FHitReactTags
{
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(HitReact_Profile_Default, "HitReact.Profile.Default", "Default hit react profile");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(HitReact_Profile_Default_NoArms, "HitReact.Profile.Default.NoArms", "Blend weight for arms is ignored");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(HitReact_Profile_Default_NoLegs, "HitReact.Profile.Default.NoLegs", "Blend weight for legs is ignored");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(HitReact_Profile_Default_NoLimbs, "HitReact.Profile.Default.NoLimbs", "Blend weight for arms and legs are ignored");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(HitReact_Profile_BumpPawn, "HitReact.Profile.BumpPawn", "Designed for bumping into other pawns, a very slow and lazy hit react");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(HitReact_Profile_TakeHit, "HitReact.Profile.TakeHit", "Designed for getting hit by a weapon or a slow firing projectile");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(HitReact_Profile_TakeHit_NoArms, "HitReact.Profile.TakeHit.NoArms", "Designed for getting hit by a weapon or a slow firing projectile, while ignoring arm bones");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(HitReact_Profile_TakeHit_NoLegs, "HitReact.Profile.TakeHit.NoLegs", "Designed for getting hit by a weapon or a slow firing projectile, while ignoring leg bones");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(HitReact_Profile_TakeHit_NoLimbs, "HitReact.Profile.TakeHit.NoLimbs", "Designed for getting hit by a weapon or a slow firing projectile, while ignoring arm and leg bones");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(HitReact_Profile_Twitch, "HitReact.Profile.Twitch", "Designed for rapid re-application of hit reacts, such as taking multiple shots from a fast firing weapon");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(HitReact_Profile_Twitch_NoArms, "HitReact.Profile.Twitch.NoArms", "Designed for rapid re-application of hit reacts, such as taking multiple shots from a fast firing weapon, while ignoring arm bones");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(HitReact_Profile_Twitch_NoLegs, "HitReact.Profile.Twitch.NoLegs", "Designed for rapid re-application of hit reacts, such as taking multiple shots from a fast firing weapon, while ignoring leg bones");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(HitReact_Profile_Twitch_NoLimbs, "HitReact.Profile.Twitch.NoLimbs", "Designed for rapid re-application of hit reacts, such as taking multiple shots from a fast firing weapon, while ignoring arm and leg bones");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(HitReact_Profile_Flop, "HitReact.Profile.Flop", "Makes the character as floppy as possible, primarily for testing purposes");
}