# MABS HitTrace

## What it is

`HitTrace` is the instant ranged delivery mode in MABS.

## Why it exists

Many abilities need a practical gun, beam, or instant-shot path without spawning a projectile.

## Authored fields

Use:

* `TargetType = Actor`
* `DeliveryMode = HitTrace`
* `HitTraceDistance`
* `HitTraceRadius`

`HitTraceRadius = 0` behaves like a line trace. A positive radius behaves like a sphere trace.

## Runtime behavior

On authority, MABS:

1. starts delivery
2. traces from the authoritative aim direction
3. validates the first resolved actor
4. applies the authored instant effect
5. spends cost and starts cooldown on successful commit

## Debug events

Use:

* `DeliveryStarted`
* `HitTraceHit`
* `HitTraceRejected`
* `DeliveryFailed`

## Example

Example rifle shot:

* `TargetType = Actor`
* `DeliveryMode = HitTrace`
* `HitTraceDistance = 2000`
* `HitTraceRadius = 0`
* `InstantEffectType = Damage`
