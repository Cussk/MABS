# MABS Delivery

## What it is

This document explains the Phase 5 delivery layer in MABS.

## Why it exists

Earlier phases established delivery modes. Phase 5 makes them usable for authored combat timing by adding startup, scheduled delivery, recovery, and socket-first origins.

## Current delivery modes

### `Direct`

Still uses the direct target path, but now respects authored startup and recovery timing.

### `HitTrace`

Uses an authority-side line trace or sphere trace at the authored delivery time. The trace now starts from:

1. `HitTraceOriginSocketName`
2. `DeliveryOriginSocketName`
3. a safe fallback origin

### `Melee`

Uses an authority-side sweep at the authored delivery time. The sweep now starts from:

1. `MeleeOriginSocketName`
2. `DeliveryOriginSocketName`
3. actor transform fallback

### `Projectile`

Spawns an authority-side projectile actor at the authored delivery time. Spawn origin now prefers:

1. `ProjectileSpawnSocketName`
2. `DeliveryOriginSocketName`
3. viewpoint or actor fallback

## Commit rules

`Direct`, `HitTrace`, and `Melee`:

* commit after delivery succeeds and the effect applies

`Projectile`:

* commit after authoritative projectile spawn succeeds
* apply the effect later on impact

For all modes:

* cost affordability is still validated before startup
* cost spending and cooldown start only after successful delivery
* failed delivery returns the granted spec to `Idle`

## How to use it

1. Set `DeliveryMode` on `UMABSAbilityDefinition`.
2. Set `StartupDuration`, `DeliveryTime`, and `RecoveryDuration`.
3. Author `RecoveryDuration` as the total desired skill time from activation start.
4. Fill in the delivery-specific fields and optional socket fields.
5. Grant the ability on authority.
6. Activate it by tag.
7. Inspect timing, socket, and delivery debug events.

## Example

* self-heal uses `Direct` with short startup and recovery
* rifle shot uses `HitTrace` from a muzzle socket
* sword slash uses `Melee` from a weapon or hand socket
* fireball uses `Projectile` from a hand socket

## Not included

Phase 5 delivery does not include:

* AoE zones
* location placement
* chain projectiles
* homing projectiles
* multi-hit melee
