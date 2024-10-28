// Copyright (c) Jared Taylor. All Rights Reserved.


#include "HitReactTags.h"

namespace FHitReactTags
{
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(HitReact_Profile_Default, "HitReact.Profile.Default", "Default hit react profile");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(HitReact_Profile_Default_NoArms, "HitReact.Profile.Default.NoArms", "Blend weight for arms is ignored");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(HitReact_Profile_Default_NoLegs, "HitReact.Profile.Default.NoLegs", "Blend weight for legs is ignored");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(HitReact_Profile_Default_NoLimbs, "HitReact.Profile.Default.NoLimbs", "Blend weight for arms and legs are ignored");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(HitReact_Profile_TakeShot, "HitReact.Profile.TakeShot", "Designed for rapid re-application of hit reacts, such as taking multiple shots");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(HitReact_Profile_TakeShot_NoArms, "HitReact.Profile.TakeShot.NoArms", "Designed for rapid re-application of hit reacts, such as taking multiple shots, while ignoring arm bones");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(HitReact_Profile_Flop, "HitReact.Profile.Flop", "Makes the character as floppy as possible, primarily for testing purposes");
}