# MABS Delivery

## What it is

This document explains the Phase 7 delivery layer in MABS.

## Why it exists

Earlier phases established delivery modes, timing, sockets, and presentation. Phase 7 keeps those delivery paths, but lets a successful delivery branch into a combo follow-up window, resolve an optional AoE, and start optional periodic effects.

## Current delivery modes

### `Direct`

Uses the direct target path and may now:

* apply to the direct target only
* use self as the center for an AoE pulse
* start periodic effects on the resolved actor set

### `HitTrace`

Uses an authority-side line trace or sphere trace at the authored delivery time and may now:

* apply to the first valid hit actor
* expand to an AoE burst around the resolved hit
* start periodic effects on affected actors

### `Melee`

Uses an authority-side sweep at the authored delivery time and may now:

* apply to the first valid melee hit
* expand to a follow-up AoE around that hit
* open a combo window for a simple authored follow-up ability
* start periodic effects on affected actors

### `Projectile`

Spawns an authority-side projectile actor at the authored delivery time and may now:

* apply to the hit actor on impact
* resolve AoE from the impact point, even when the projectile hits world geometry
* start periodic effects on affected actors

## Commit rules

`Direct`, `HitTrace`, and `Melee`:

* commit after delivery succeeds and at least one gameplay effect applies

`Projectile`:

* commits after authoritative projectile spawn succeeds
* applies instant and periodic gameplay outcomes later on authoritative impact

Presentation is still cosmetic:

* it triggers at startup, delivery, travel, or impact timing points
* it does not decide gameplay success
* missing cosmetic data does not become gameplay authority

## How to use it

1. Set `DeliveryMode` on `UMABSAbilityDefinition`.
2. Set `StartupDuration`, `DeliveryTime`, and `RecoveryDuration`.
3. Fill delivery-specific fields and socket fields.
4. Enable `AoE` or `PeriodicEffect` only when the ability needs them.
5. Fill the relevant presentation groups for that delivery mode.
6. Grant the ability on authority.
7. Activate it by tag.

## Example

* self-heal pulse uses `Direct` with `AoE` and `PeriodicEffect = HOT`
* rifle shot uses `HitTrace` with a small AoE burst on impact
* sword slash uses `Melee` with combo follow-up support
* fireball uses `Projectile` with impact AoE and a burn periodic effect
