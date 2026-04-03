# MABS Delivery

## What it is

This document explains the Phase 6 delivery layer in MABS.

## Why it exists

Earlier phases established delivery modes, timing, and socket-first origins. Phase 6 adds presentation on top of those same delivery moments without changing the server-authoritative gameplay model.

## Current delivery modes

### `Direct`

Uses the direct target path, now with:

* startup presentation
* delivery presentation
* impact presentation on successful effect application

### `HitTrace`

Uses an authority-side line trace or sphere trace at the authored delivery time.

The trace origin still prefers:

1. `HitTraceOriginSocketName`
2. `DeliveryOriginSocketName`
3. a safe fallback origin

Phase 6 adds:

* delivery cue at the origin
* optional tracer from trace start to hit or trace end
* impact cue on successful effect application

### `Melee`

Uses an authority-side sweep at the authored delivery time.

The sweep origin still prefers:

1. `MeleeOriginSocketName`
2. `DeliveryOriginSocketName`
3. actor transform fallback

Phase 6 adds:

* delivery cue at the melee origin
* impact cue on successful effect application

### `Projectile`

Spawns an authority-side projectile actor at the authored delivery time.

Spawn origin still prefers:

1. `ProjectileSpawnSocketName`
2. `DeliveryOriginSocketName`
3. viewpoint or actor fallback

Phase 6 adds:

* delivery cue at projectile spawn
* projectile-attached travel VFX and SFX
* impact cue on projectile hit

## Commit rules

`Direct`, `HitTrace`, and `Melee`:

* commit after delivery succeeds and the effect applies

`Projectile`:

* commits after authoritative projectile spawn succeeds
* applies the effect later on impact

Presentation is cosmetic:

* it triggers at startup, delivery, or impact timing points
* it does not decide gameplay success
* missing cosmetic data does not become gameplay authority

## How to use it

1. Set `DeliveryMode` on `UMABSAbilityDefinition`.
2. Set `StartupDuration`, `DeliveryTime`, and `RecoveryDuration`.
3. Fill the delivery-specific fields and socket fields.
4. Fill the relevant presentation groups for that delivery mode.
5. Grant the ability on authority.
6. Activate it by tag.

## Example

* self-heal uses `Direct` with startup and impact presentation
* rifle shot uses `HitTrace` with muzzle, tracer, and impact presentation
* sword slash uses `Melee` with swing and impact presentation
* fireball uses `Projectile` with spawn, travel, and impact presentation
