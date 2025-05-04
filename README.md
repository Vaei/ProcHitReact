# Proc Hit React <img align="right" width=128, height=128 src="https://github.com/Vaei/ProcHitReact/blob/main/Resources/Icon128.png">

> [!IMPORTANT]
> **Procedural physics-driven hit reactions for skeletal meshes!**
> <br>High levels of customization and extendability
> <br>Fully extendable in Blueprint
> <br>And its **Free!**

> [!WARNING]
> Use `git clone` instead of downloading as a zip, or you will not receive content
> <br>[Or download the pre-compiled binaries here](https://github.com/Vaei/ProcHitReact/wiki/How-to-Use)
> <br>Install this as a project plugin, not an engine plugin

> [!TIP]
> Suitable for both singleplayer and multiplayer games
> <br>Supports UE5.3+

> [!CAUTION]
> Many features are still in beta
> <br>ProcHitReact is sufficiently feature rich that every use-case can't feasibly be tested

## Watch Me

> [!TIP]
> [Showcase Video](https://youtu.be/y_BLLSzCjd4)

## How to Use
> [!IMPORTANT]
> [Read the Wiki to Learn How to use Hit React](https://github.com/Vaei/TurnInPlace/wiki/How-to-Use)

## Why Make a Hit Reaction Plugin?

ProcHitReact addresses all these common limitations:

* Supports multiple overlapping hit reactions
  * Repeated hit reacts are handled seamlessly without snapping
  * Child bone blend weights are not overwritten by parent bones
  * Seamlessly blends differing profiles and parameters
* Option to disable blend weights or physics sim on individual bones
  * Supports holding weapons
* Consideration for real-world application in complex projects

## Features
### Ease of Use
In many cases, all it takes is putting a component on your character, assigning profiles, and calling `HitReact()` on it.

When setting up something out of the ordinary further setup is necessary, but it can be done even in blueprint.

### Impulses
Supports Linear, Angular, and Radial impulses.

### Per-Bone Tuning
Supports disabling physics or using custom blend weights on child bones of the simulated bone.

This means we can disable physics only on the arms for our character who is aiming a rifle!

Blacklist bones that don't simulate well, such as the pelvis which can cause the entire mesh to simulate!

### Profiles
Supports extendable data-asset profiles, simply specify the profile you want to use when triggering a Hit React.

The included HitReact component has many profiles setup for you out of the box with tested defaults.

### Global Toggle
You can toggle the entire system on and off, with or without interpolation.

If you have the Gameplay Abilities (GAS) plugin enabled in your project, you can use tags to enable and disable the system. This is an optional dependency, ProcHitReact does not require that your project uses GAS and will not enable it for you.

### Sleep & Wake
The system automatically stops itself from ticking when it doesn't need to.

### Powerful Blending
Featuring a purpose-built interpolation framework, you can customize the blending to your liking.

The available states are: Blend In, Hold, and Blend Out. Hold will maintain your physics at max blend weight for the duration.

Reapplication can be throttled by setting a Cooldown.

Physics bodies have custom simulation behaviour, nothing so primitive as using `SetAllBodiesBelowPhysicsBlendWeight()`

### Networking
Generally hit reacts are entirely cosmetic and should be applied via gameplay cues or other generalized multicast/replication events.

The data types used by ProcHitReact, including the application parameters, are net serialized for you, so you can replicate these too.

Dedicated servers don't process hit reacts, unless you enable the setting.

ProcHitReact was designed with multiplayer games in mind.

### Debugging Capability
See the [debugging section on the Wiki](https://github.com/Vaei/ProcHitReact/wiki/Debugging) to learn how to Debug ProcHitReact.

## Changelog

### 1.0.0
* Initial Release
