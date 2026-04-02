# MABS Delivery

## What it is

This document explains the Phase 4 delivery layer in MABS.

## Why it exists

Earlier phases only supported direct target resolution and instant effect application. Phase 4 adds the common delivery patterns users expect from a practical ability system.

## Current delivery modes

### `Direct`

Keeps the older behavior:

* resolve the target immediately
* apply the instant effect immediately

### `HitTrace`

Uses an authority-side line trace or sphere trace to resolve one valid target.

### `Melee`

Uses an authority-side short-range sweep to resolve one valid nearby target.

### `Projectile`

Spawns an authority-side projectile actor. The projectile later applies the authored effect on authoritative impact.

## Commit rules

`Direct`, `HitTrace`, and `Melee`:

* commit after delivery succeeds and the effect applies

`Projectile`:

* commit after authoritative projectile spawn succeeds
* apply the effect later on impact

## How to use it

1. Set `DeliveryMode` on `UMABSAbilityDefinition`.
2. Fill in the delivery-specific fields for that mode.
3. Grant the ability on authority.
4. Activate it by tag.
5. Inspect delivery-specific debug events.

## Example

* self-heal uses `Direct`
* rifle shot uses `HitTrace`
* sword slash uses `Melee`
* fireball uses `Projectile`

## Not included

Phase 4 delivery does not include:

* AoE zones
* location placement
* chain projectiles
* homing projectiles
* multi-hit melee
