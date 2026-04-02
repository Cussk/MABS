# MABS Abilities

## What it is

This document explains the current Phase 5 ability model: authored definitions, granted runtime specs, activation timing, socket-aware delivery, and optional montage requests.

## Why it exists

MABS stays readable by keeping authored data separate from live runtime state. Phase 5 keeps that split intact while making runtime timing explicit and inspectable.

## Ability definitions

`UMABSAbilityDefinition` is the authored asset in `MABSCore`.

Phase 5 authored fields cover five areas:

* identity and activation: `AbilityTag`, `DisplayName`, `ActivationPolicy`
* target intent and effect: `TargetType`, `InstantEffectType`, `EffectMagnitude`
* usage rules: `CooldownSeconds`, `CooldownGroupTag`, `ResourceCost`
* timing: `StartupDuration`, `DeliveryTime`, `RecoveryDuration`
* delivery and presentation: `DeliveryMode`, socket fields, and optional montage fields

Current delivery modes are:

* `Direct`
* `HitTrace`
* `Melee`
* `Projectile`

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

## Activation flow

Activation in Phase 5 means:

1. code or input calls `TryActivateAbilityByTag`
2. the request reaches authority
3. the component validates grant, cooldown, and cost rules
4. the granted spec enters `Startup`
5. the component optionally requests montage playback
6. delivery executes at the authored delivery time
7. `Direct`, `HitTrace`, and `Melee` apply the instant effect on successful delivery
8. `Projectile` commits on successful spawn
9. authority spends cost and starts cooldown on successful commit
10. the granted spec enters `Recovery` and later returns to `Idle`
11. projectile impact later applies the authored instant effect on authority

## Direct versus delivery

Target intent and delivery are separate concepts.

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
* how timing is configured
* which sockets or offsets delivery should use
* whether an activation montage should be requested

Timing authoring uses:

* `StartupDuration` as startup length from activation start
* `DeliveryTime` as the intended delivery moment from activation start
* `RecoveryDuration` as the total intended ability length from activation start

Runtime state answers:

* whether the ability is granted
* whether it is idle, in startup, active, in recovery, or blocked
* what happened on the last activation request
* when its personal cooldown ends
* when startup began
* when delivery is scheduled
* when recovery ends
* which cooldown groups are active on the owner

Projectile flight itself is transient runtime state on the spawned projectile actor, not on the data asset.

## How to use it

1. Create a `UMABSAbilityDefinition`.
2. Set `TargetType` and `DeliveryMode`.
3. Set `InstantEffectType` and `EffectMagnitude`.
4. Configure cooldown and cost if needed.
5. Fill in the delivery-specific fields.
6. Fill in timing, socket, and optional montage fields.
7. Grant the ability on authority.
8. Activate it with `TryActivateAbilityByTag`.
9. Inspect `FMABSAbilitySpec`, recent debug events, and the overlay.

## Not included in this phase

Phase 5 still does not include:

* AoE zones
* location abilities
* prediction
* projectile homing
* chain or bounce projectiles
* multi-hit melee systems
* duration-based effects
* status effects
* notify-driven execution as the primary path
