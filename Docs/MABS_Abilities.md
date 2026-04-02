# MABS Abilities

## What it is

This document explains the current Phase 4 ability model: authored definitions, granted runtime specs, activation flow, and how delivery now fits between validation and effect application.

## Why it exists

MABS stays readable by keeping authored data separate from live runtime state. Phase 4 keeps that split intact while adding the first real delivery layer.

## Ability definitions

`UMABSAbilityDefinition` is the authored asset in `MABSCore`.

Phase 4 authored fields now cover four areas:

* identity and activation: `AbilityTag`, `DisplayName`, `ActivationPolicy`
* target intent and effect: `TargetType`, `InstantEffectType`, `EffectMagnitude`
* usage rules: `CooldownSeconds`, `CooldownGroupTag`, `ResourceCost`
* delivery: `DeliveryMode` plus delivery-specific fields

Current delivery modes are:

* `Direct`
* `HitTrace`
* `Melee`
* `Projectile`

## Granted abilities

Granting still creates a `FMABSAbilitySpec` on `UMABSAbilityComponent`.

Each spec stores:

* the definition reference
* a copied gameplay tag
* a stable handle
* the current runtime state
* the last activation result
* the personal cooldown end time

## Activation flow

Activation in Phase 4 means:

1. code or input calls `TryActivateAbilityByTag`
2. the request reaches authority
3. the component validates grant, cooldown, and cost rules
4. the component emits `DeliveryStarted`
5. the component executes the authored delivery mode
6. `Direct`, `HitTrace`, and `Melee` apply the instant effect immediately on success
7. `Projectile` commits on successful spawn
8. authority spends cost and starts cooldown on successful commit
9. projectile impact later applies the authored instant effect on authority
10. the component emits readable debug events for the full path

## Direct versus delivery

Target intent and delivery are now separate concepts.

Examples:

* a self-heal uses `TargetType = Self` and `DeliveryMode = Direct`
* a rifle shot uses `TargetType = Actor` and `DeliveryMode = HitTrace`
* a sword slash uses `TargetType = Actor` and `DeliveryMode = Melee`
* a fireball uses `TargetType = Actor` and `DeliveryMode = Projectile`

`Direct` keeps the older target-resolution path. The other delivery modes resolve the final actor through delivery execution.

## Runtime state versus authored data

Authored data answers:

* what the ability should do
* which delivery mode it uses
* how its effect, cost, and cooldown are configured

Runtime state answers:

* whether the ability is granted
* whether it is idle, active, or blocked
* what happened on the last activation request
* when its personal cooldown ends
* which cooldown groups are active on the owner

Projectile flight itself is transient runtime state on the spawned projectile actor, not on the data asset.

## How to use it

1. Create a `UMABSAbilityDefinition`.
2. Set `TargetType` and `DeliveryMode`.
3. Set `InstantEffectType` and `EffectMagnitude`.
4. Configure cooldown and cost if needed.
5. Fill in the delivery-specific fields for hit trace, melee, or projectile.
6. Grant the ability on authority.
7. Activate it with `TryActivateAbilityByTag`.
8. Inspect `FMABSAbilitySpec`, recent debug events, and the overlay.

## Not included in this phase

Phase 4 still does not include:

* AoE zones
* location abilities
* prediction
* projectile homing
* chain or bounce projectiles
* multi-hit melee systems
* duration-based effects
* status effects
* animation systems
