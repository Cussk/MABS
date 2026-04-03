# MABS Abilities

## What it is

This document explains the current Phase 7.5 ability model: authored definitions, optional grouped ability sets, granted runtime specs, combo follow-up state, optional AoE, optional periodic effects, timing-driven execution, socket-aware delivery, optional montage playback, and presentation.

## Why it exists

MABS stays readable by keeping authored data separate from live runtime state.

Phase 7.5 keeps the Phase 7 combat model and adds a small authoring convenience: designers can now group multiple `UMABSAbilityDefinition` assets into one `UMABSAbilitySet` and grant them together.

## Ability definitions

`UMABSAbilityDefinition` is the authored single-ability asset in `MABSCore`.

Current authored fields cover:

* identity and activation: `AbilityTag`, `DisplayName`, `ActivationPolicy`
* target intent and gameplay outcome: `TargetType`, `InstantEffectType`, `EffectMagnitude`
* usage rules: `CooldownSeconds`, `CooldownGroupTag`, `ResourceCost`
* timing: `StartupDuration`, `DeliveryTime`, `RecoveryDuration`
* delivery and sockets: `DeliveryMode`, delivery socket fields, offsets, and optional montage fields
* presentation: `StartupPresentation`, `DeliveryPresentation`, and `ImpactPresentation`
* combat breadth: `Combo`, `AoE`, and `PeriodicEffect`

## Ability sets

`UMABSAbilitySet` is the authored grouped-grant asset in `MABSCore`.

It currently contains:

* `AbilityDefinitions`

Use it when several abilities should be granted together, such as:

* starting abilities
* class or archetype bundles
* enemy bundles
* test bundles

Ability sets do not replace individual ability definitions. They only group them for grant authoring.

## Granted abilities

Granting still creates a `FMABSAbilitySpec` on `UMABSAbilityComponent`.

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

Ability sets do not add a second runtime state container. They reuse this same existing grant path.

## Activation flow

Activation in Phase 7.5 still means:

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

* what one ability should do
* how multiple abilities should be grouped for grant authoring
* which delivery mode it uses
* whether it can branch into a combo follow-up
* whether it resolves an AoE shape
* whether it starts a periodic effect
* how timing, sockets, montage playback, and presentation are configured

Runtime state answers:

* which abilities are granted
* whether a granted ability is idle, in startup, active, recovery, or blocked
* what happened on the last activation request
* when its personal cooldown ends
* when startup began
* when delivery is scheduled
* when recovery ends
* whether a combo window is open and which follow-up has been queued
* which periodic effects are currently active on authority

## How to use it

1. Create one or more `UMABSAbilityDefinition` assets.
2. Optionally create a `UMABSAbilitySet`.
3. Fill `AbilityDefinitions` when several abilities should be granted together.
4. Grant either the definition or the set on authority.
5. Activate abilities with `TryActivateAbilityByTag`.
6. Inspect `FMABSAbilitySpec`, active periodic effects, and recent debug events.

## Example

Example grouped player starter:

* `DA_Player_Attack`
* `DA_Player_Dodge`
* `DA_Player_Heal`
* `DA_StartingAbilities_Player.AbilityDefinitions = [Attack, Dodge, Heal]`

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
