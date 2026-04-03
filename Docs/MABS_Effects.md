# MABS Effects

## What it is

This document explains the current Phase 7 gameplay effect model in MABS.

## Why it exists

Activation is only useful if it leads to a gameplay outcome. Phase 7 keeps instant effects small and readable, then adds a lightweight timer-driven periodic layer for common burn and regen style abilities.

## Supported gameplay effects

Current support is:

* instant `Damage`
* instant `Heal`
* periodic `DOT`
* periodic `HOT`

## How effects apply now

`Direct`, `HitTrace`, and `Melee`:

* resolve a primary impact context
* optionally expand to AoE
* apply instant effects immediately on authority
* start or refresh periodic effects on authority

`Projectile`:

* commits on authoritative spawn
* applies instant effects and periodic effects later on authoritative impact

## Damage

Damage still uses Unreal's generic damage path through `UGameplayStatics::ApplyDamage`.

## Heal

Heal still uses `IMABSInstantEffectReceiver`.

Targets that should accept heal must implement:

* `ApplyMABSHeal(...)`

## Periodic runtime

Periodic effects:

* live on the authoritative `UMABSAbilityComponent`
* use timers instead of tick-based actor logic
* apply their magnitude every authored `TickInterval`
* expire cleanly when `Duration` ends

## Reapply rule

Phase 7 uses one simple reapply rule:

* reapplying the same periodic effect to the same target refreshes duration

It does not add complex stacking math in this phase.

## Failure behavior

Gameplay effect application still fails cleanly if:

* no valid target could be resolved
* the target does not support the authored effect type
* a heal target rejects the heal
* damage application returns zero accepted damage

## Example

* fireball explosion deals instant damage and starts a `DOT`
* healing pulse resolves an AoE and starts a `HOT`
