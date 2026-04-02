# MABS Melee

## What it is

`Melee` is the short-range sweep delivery mode in MABS.

## Why it exists

Many combat abilities need a sword arc or short-range strike path that resolves on authority without a projectile.

## Authored fields

Use:

* `TargetType = Actor`
* `DeliveryMode = Melee`
* `MeleeRange`
* `MeleeRadius`
* `MeleeForwardOffset`

## Runtime behavior

On authority, MABS:

1. starts delivery
2. uses the authored melee socket or fallback origin as the sweep start location
3. uses the owning actor's current facing as the sweep direction
4. falls back to controller or eye-view yaw only if actor forward is unavailable
5. validates the first resolved actor
6. applies the authored instant effect
7. spends cost and starts cooldown on successful commit

## Debug events

Use:

* `DeliveryStarted`
* `MeleeHit`
* `MeleeRejected`
* `DeliveryFailed`

## Example

Example sword slash:

* `TargetType = Actor`
* `DeliveryMode = Melee`
* `MeleeRange = 200`
* `MeleeRadius = 75`
* `MeleeForwardOffset = 50`
* `InstantEffectType = Damage`
