# MABS Projectiles

## What it is

This document explains the built-in projectile path in MABS after Phase 6.5 cue routing.

## Why it exists

Projectiles are one of the most common expected delivery modes for practical combat abilities. Phase 6 added spawn, travel, and impact presentation. Phase 6.5 keeps that feature set, but routes projectile travel through the same lightweight cue model used by the rest of the presentation layer.

## Built-in runtime type

`AMABSProjectileBase` in `MABSGameplay`:

* replicates movement
* stores replicated source ability context
* routes impact handling back through `UMABSAbilityComponent`
* applies the authored instant effect on a valid impact
* builds a lightweight projectile-travel cue event from authored data
* evaluates travel cue visibility locally on each instance
* activates authored projectile travel Niagara VFX and SFX when the cue policy allows it

## Authored fields

Use:

* `TargetType = Actor`
* `DeliveryMode = Projectile`
* `ProjectileActorClass`
* `ProjectileSpawnOffset`
* `DeliveryPresentation.Cue`
* `DeliveryPresentation.ProjectileTravel`
* `DeliveryPresentation.ProjectileTravel.VisibilityPolicy`
* `ImpactPresentation.Cue`

`ProjectileActorClass` must derive from `AMABSProjectileBase`.

## Runtime behavior

Projectile abilities still work the same way for gameplay:

* the server spawns the projectile
* successful spawn commits the ability
* the effect applies later on authoritative impact

Presentation behavior is now:

* delivery cue routes at projectile spawn time
* projectile travel builds a `FMABSProjectileTravelCueEvent`
* the replicated projectile realizes travel presentation locally only when the authored visibility policy allows it
* impact presentation routes through the same cue helper path used by other presentation phases

## Debug events

Use:

* `ProjectileSpawned`
* `ProjectileTravelPresentationTriggered`
* `PresentationCueRouted`
* `PresentationCueSkipped`
* `PresentationCueRealized`
* `ProjectileImpact`
* `ImpactPresentationTriggered`

## Example

Example fireball:

* `ProjectileActorClass = BP_MABS_Fireball`
* `ProjectileSpawnSocketName = hand_r`
* `DeliveryPresentation.Cue.VFX = NS_FireballCast`
* `DeliveryPresentation.ProjectileTravel.TravelVFX = NS_FireballTrail`
* `DeliveryPresentation.ProjectileTravel.VisibilityPolicy = RelevantClients`
* `ImpactPresentation.Cue.VFX = NS_FireballImpact`
