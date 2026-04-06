# MABS Melee

## What it is

`Melee` is the short-range sweep delivery mode in MABS.

In Phase 11, the built-in melee behavior now lives in `UMABSMeleeDeliveryHandler`.

## Why it exists

Many combat abilities need a sword arc or short-range strike path that resolves on authority without a projectile.

Phase 11 also gives melee the same extension story that projectiles already had.

## Authored fields

Use:

* `TargetType = Actor`
* `DeliveryMode = Melee`
* optional `DeliveryHandlerClass`
* `MeleeRange`
* `MeleeRadius`
* `MeleeForwardOffset`
* `StartupPresentation.Cue`
* `DeliveryPresentation.Cue`
* `ImpactPresentation.Cue`

## Runtime behavior

On authority, the built-in melee handler:

1. enters delivery at the authored delivery time
2. triggers delivery presentation at the melee origin
3. uses the authored melee socket or fallback origin
4. sweeps forward using the owner-facing direction
5. resolves the primary hit actor
6. expands to AoE targets when authored
7. returns the final target set to the normal MABS effect / presentation / commit flow

Melee whiffs still commit into recovery so combo windows and queued follow-ups behave the same way they did before Phase 11.

## Validation

Phase 12 validates the common melee authoring mistakes:

* `TargetType` must be `Actor`
* `MeleeRange` must be greater than zero
* `MeleeRadius` must be greater than zero
* if a custom handler class is set, it must be loadable, non-abstract, and derive from `UMABSDeliveryHandler`
* a handler derived from a different built-in mode-specific base is rejected as incompatible authoring

## How to extend it

Create a class deriving from `UMABSMeleeDeliveryHandler` when one ability needs custom close-range delivery behavior.

Good uses:

* stagger or launch logic
* pull or suction effects
* extra target prioritization
* melee-specific filtering

Blueprint subclasses can override `ModifyDeliveryResult(...)` and keep the normal built-in sweep logic.

## Example

Example launcher strike:

* `DeliveryMode = Melee`
* `DeliveryHandlerClass = BP_LauncherStrikeHandler`
* built-in melee still resolves the hit
* the Blueprint handler adds launch movement to the resolved targets
