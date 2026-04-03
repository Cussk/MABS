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
* `StartupPresentation.Cue`
* `DeliveryPresentation.Cue`
* `ImpactPresentation.Cue`

## Runtime behavior

On authority, MABS:

1. enters delivery at the authored delivery time
2. triggers delivery presentation at the melee origin
3. uses the authored melee socket or fallback origin as the sweep start location
4. uses the owning actor's current facing as the sweep direction
5. validates the first resolved actor
6. applies the authored instant effect
7. triggers impact presentation on successful effect application
8. spends cost and starts cooldown on successful commit

## Debug events

Use:

* `DeliveryPresentationTriggered`
* `MeleeHit`
* `MeleeRejected`
* `ImpactPresentationTriggered`

## Example

Example sword slash:

* `MeleeOriginSocketName = weapon_tip`
* `StartupPresentation.Cue.SFX = Sword_Windup`
* `DeliveryPresentation.Cue.SFX = Sword_Swing`
* `ImpactPresentation.Cue.VFX = P_SwordImpact`
