# MABS Projectiles

## What it is

This document explains the built-in projectile path in MABS.

## Why it exists

Projectiles are one of the most common expected delivery modes for practical combat abilities.

## Built-in runtime type

Phase 4 adds `AMABSProjectileBase` in `MABSGameplay`.

It:

* replicates movement
* stores the source ability context
* routes impact handling back through `UMABSAbilityComponent`
* applies the authored instant effect on a valid impact

## Authored fields

Use:

* `TargetType = Actor`
* `DeliveryMode = Projectile`
* `ProjectileActorClass`
* `ProjectileSpawnOffset`

`ProjectileActorClass` must derive from `AMABSProjectileBase`.

## Commit behavior

Projectile abilities use a spawn-success model:

* if the projectile spawns, the ability commits
* cost is spent and cooldown starts on spawn success
* the effect applies later on authoritative impact

## Debug events

Use:

* `DeliveryStarted`
* `ProjectileSpawned`
* `ProjectileSpawnFailed`
* `ProjectileImpact`
* `ProjectileImpactRejected`

## Example

Example fireball:

* `TargetType = Actor`
* `DeliveryMode = Projectile`
* `ProjectileActorClass = BP_MABS_Fireball`
* `ProjectileSpawnOffset = (100, 0, 0)`
* `InstantEffectType = Damage`
