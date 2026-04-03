# MABS Projectiles

## What it is

This document explains the built-in projectile path in MABS.

## Why it exists

Projectiles are one of the most common expected delivery modes for practical combat abilities, and Phase 6 now gives them authored spawn, travel, and impact presentation coverage.

## Built-in runtime type

`AMABSProjectileBase` in `MABSGameplay`:

* replicates movement
* stores replicated source ability context
* routes impact handling back through `UMABSAbilityComponent`
* applies the authored instant effect on a valid impact
* locally activates authored projectile travel VFX and SFX from the replicated projectile actor

## Authored fields

Use:

* `TargetType = Actor`
* `DeliveryMode = Projectile`
* `ProjectileActorClass`
* `ProjectileSpawnOffset`
* `DeliveryPresentation.Cue`
* `DeliveryPresentation.ProjectileTravel`
* `ImpactPresentation.Cue`

`ProjectileActorClass` must derive from `AMABSProjectileBase`.

## Commit behavior

Projectile abilities use a spawn-success model:

* if the projectile spawns, the ability commits
* cost is spent and cooldown starts on spawn success
* the effect applies later on authoritative impact

Projectile presentation behavior is:

* delivery cue plays at projectile spawn time
* travel presentation starts from the projectile actor itself
* impact presentation plays on authoritative hit

## Debug events

Use:

* `DeliveryPresentationTriggered`
* `ProjectileSpawned`
* `ProjectileTravelPresentationTriggered`
* `ProjectileImpact`
* `ImpactPresentationTriggered`

## Example

Example fireball:

* `ProjectileActorClass = BP_MABS_Fireball`
* `ProjectileSpawnSocketName = hand_r`
* `DeliveryPresentation.Cue.VFX = P_FireballCast`
* `DeliveryPresentation.ProjectileTravel.TravelVFX = P_FireballTrail`
* `ImpactPresentation.Cue.VFX = P_FireballImpact`
