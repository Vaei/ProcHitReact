// Copyright (c) Jared Taylor. All Rights Reserved.


#include "HitReactTags.h"

namespace FHitReactTags
{
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(HitReact_Profile_Default, "HitReact.Profile.Default", "Default hit react profile");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(HitReact_Profile_Limp, "HitReact.Profile.Limp", "Limp profile, makes the character as floppy as possible, primarily for testing purposes");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(HitReact_Profile_NoArms, "HitReact.Profile.NoArms", "Blend weight for arms is ignored");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(HitReact_Profile_NoLegs, "HitReact.Profile.NoLegs", "Blend weight for legs is ignored");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(HitReact_Profile_NoLimbs, "HitReact.Profile.NoLimbs", "Blend weight for arms and legs are ignored");
}