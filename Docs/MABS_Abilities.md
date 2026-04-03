# MABS Abilities

## What it is

This document explains the current Phase 7 ability model: authored definitions, granted runtime specs, combo follow-up state, optional AoE, optional periodic effects, timing-driven execution, socket-aware delivery, optional montage playback, and presentation.

## Why it exists

MABS stays readable by keeping authored data separate from live runtime state.

Phase 7 extends the Phase 6.5 model without turning it into a large effect-spec framework. Combos, AoE, and periodic effects are grouped authored data on the ability definition, while runtime timing, combo queueing, and active periodic entries stay on the ability component side.

## Ability definitions

`UMABSAbilityDefinition` is the authored asset in `MABSCore`.

Current authored fields cover:

* identity and activation: `AbilityTag`, `DisplayName`, `ActivationPolicy`
* target intent and gameplay outcome: `TargetType`, `InstantEffectType`, `EffectMagnitude`
* usage rules: `CooldownSeconds`, `CooldownGroupTag`, `ResourceCost`
* timing: `StartupDuration`, `DeliveryTime`, `RecoveryDuration`
* delivery and sockets: `DeliveryMode`, delivery socket fields, offsets, and optional montage fields
* presentation: `StartupPresentation`, `DeliveryPresentation`, and `ImpactPresentation`
* combat breadth: `Combo`, `AoE`, and `PeriodicEffect`

Current delivery modes are:

* `Direct`
* `HitTrace`
* `Melee`
* `Projectile`

## Presentation authoring

Presentation still uses grouped data:

* `StartupPresentation.Cue`
* `DeliveryPresentation.Cue`
* `DeliveryPresentation.HitTraceTracer`
* `DeliveryPresentation.ProjectileTravel`
* `ImpactPresentation.Cue`

Phase 7 does not change the cue model. Combo, AoE, and periodic effects plug into the same gameplay timing and debug flow.

## Granted abilities

Granting creates a `FMABSAbilitySpec` on `UMABSAbilityComponent`.

Each spec stores:

* the definition reference
* a copied gameplay tag
* a stable handle
* the current runtime state
* the last activation result
* the personal cooldown end time
* the activation start timestamp
* the scheduled delivery timestamp
* the recovery end timestamp
* the combo window start time
* the combo window end time
* the queued combo follow-up tag, when one has been buffered

## Activation flow

Activation in Phase 7 means:

1. code or input calls `TryActivateAbilityByTag`
2. the request reaches authority
3. combo follow-up requests may be queued against an already active melee ability
4. authority validates grant, cooldown, cost, and authored gameplay effect data
5. the granted spec enters `Startup`
6. startup presentation triggers if authored
7. the component optionally requests montage playback
8. delivery executes at the authored delivery time
9. delivery presentation triggers at that same timing point
10. the server resolves the primary impact context
11. optional AoE gathers the final actor set
12. instant effects apply immediately to affected targets
13. optional periodic effects start or refresh on affected targets
14. impact presentation triggers after successful application
15. authority spends cost, starts cooldown, and enters `Recovery`
16. queued combo follow-ups start automatically when the current ability completes recovery

## Runtime state versus authored data

Authored data answers:

* what the ability should do
* which delivery mode it uses
* whether it can branch into a combo follow-up
* whether it resolves an AoE shape
* whether it starts a periodic effect
* how timing, sockets, montage playback, and presentation are configured

Runtime state answers:

* whether the ability is granted
* whether it is idle, in startup, active, recovery, or blocked
* what happened on the last activation request
* when its personal cooldown ends
* when startup began
* when delivery is scheduled
* when recovery ends
* whether a combo window is open and which follow-up has been queued
* which periodic effects are currently active on authority

## How to use it

1. Create a `UMABSAbilityDefinition`.
2. Set `TargetType`, `DeliveryMode`, timing, cost, and cooldown data.
3. Configure instant effect, periodic effect, or both.
4. Fill `Combo` when authoring a melee follow-up chain.
5. Fill `AoE` when the ability should affect more than one actor.
6. Fill the presentation groups that matter for that delivery mode.
7. Grant the ability on authority.
8. Activate it with `TryActivateAbilityByTag`.
9. Inspect `FMABSAbilitySpec`, active periodic effects, recent debug events, and the overlay.

## Example

Example three-hit sword opener:

* `DeliveryMode = Melee`
* `Combo.NextComboAbilityTag = Ability.Combat.Attack2`
* `Combo.ComboWindowStart = 0.18`
* `Combo.ComboWindowEnd = 0.42`
* `Combo.bBufferComboInput = true`

Example explosive fireball:

* `DeliveryMode = Projectile`
* `AoE.bEnabled = true`
* `AoE.Shape = Sphere`
* `AoE.Radius = 250`
* `PeriodicEffect.bEnabled = true`
* `PeriodicEffect.EffectType = DOT`
* `PeriodicEffect.Duration = 4.0`
* `PeriodicEffect.TickInterval = 1.0`
* `PeriodicEffect.TickMagnitude = 6.0`
