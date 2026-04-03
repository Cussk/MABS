# MABS Periodic Effects

## What it is

This document explains the basic Phase 7 periodic effect model in MABS.

## Why it exists

Burns, poison, regeneration, and heal-over-time are baseline combat expectations. Phase 7 adds those as a small timer-driven runtime system instead of a larger status-effect graph.

## Authored fields

Use the `PeriodicEffect` group on an ability:

* `bEnabled`
* `EffectType`
* `Duration`
* `TickInterval`
* `TickMagnitude`

Supported periodic types are:

* `DOT`
* `HOT`

## Runtime behavior

On authority, MABS:

1. applies or refreshes a lightweight active periodic effect entry after a successful application moment
2. stores the effect on the source actor's `UMABSAbilityComponent`
3. ticks it using timers
4. expires it cleanly when duration ends

Reapplying the same periodic effect to the same target refreshes duration in Phase 7.

## How to use it

1. Enable `PeriodicEffect`.
2. Pick `DOT` or `HOT`.
3. Set `Duration`, `TickInterval`, and `TickMagnitude`.
4. Optionally pair it with an instant effect on the same ability.
5. Activate the ability on authority.

## Example

Example burn:

* `PeriodicEffect.bEnabled = true`
* `PeriodicEffect.EffectType = DOT`
* `PeriodicEffect.Duration = 4.0`
* `PeriodicEffect.TickInterval = 1.0`
* `PeriodicEffect.TickMagnitude = 6.0`

Example regeneration pulse:

* `PeriodicEffect.bEnabled = true`
* `PeriodicEffect.EffectType = HOT`
* `PeriodicEffect.Duration = 3.0`
* `PeriodicEffect.TickInterval = 0.5`
* `PeriodicEffect.TickMagnitude = 4.0`

## Not included

Phase 7 does not add:

* complex stacking rules
* buff / debuff stat modifiers
* deep status-effect graphs
