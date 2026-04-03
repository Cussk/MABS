# MABS Abilities

## What it is

This document explains the current Phase 6 ability model: authored definitions, granted runtime specs, timing-driven execution, socket-aware delivery, optional montage playback, and the first built-in presentation layer.

## Why it exists

MABS stays readable by keeping authored data separate from live runtime state.

Phase 6 keeps that split intact while extending authored abilities with grouped presentation data instead of scattering cosmetic fields across gameplay code.

## Ability definitions

`UMABSAbilityDefinition` is the authored asset in `MABSCore`.

Current authored fields cover:

* identity and activation: `AbilityTag`, `DisplayName`, `ActivationPolicy`
* target intent and effect: `TargetType`, `InstantEffectType`, `EffectMagnitude`
* usage rules: `CooldownSeconds`, `CooldownGroupTag`, `ResourceCost`
* timing: `StartupDuration`, `DeliveryTime`, `RecoveryDuration`
* delivery and sockets: `DeliveryMode`, delivery socket fields, offsets, and optional montage fields
* presentation: `StartupPresentation`, `DeliveryPresentation`, and `ImpactPresentation`

Current delivery modes are:

* `Direct`
* `HitTrace`
* `Melee`
* `Projectile`

## Presentation authoring

Phase 6 adds grouped presentation data:

* `StartupPresentation.Cue` for cast or windup presentation
* `DeliveryPresentation.Cue` for muzzle, spawn, or release presentation
* `DeliveryPresentation.HitTraceTracer` for hit-trace travel presentation
* `DeliveryPresentation.ProjectileTravel` for projectile-attached travel presentation
* `ImpactPresentation.Cue` for successful hit or effect presentation

Each shared cue can author:

* `VFX`
* `SFX`
* `CameraShake`
* `SocketName`
* `LocationOffset`
* `RotationOffset`

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

Phase 6 does not add cosmetic runtime state to the spec. Presentation stays transient and is triggered from the existing execution flow.

## Activation flow

Activation in Phase 6 means:

1. code or input calls `TryActivateAbilityByTag`
2. the request reaches authority
3. the component validates grant, cooldown, and cost rules
4. the granted spec enters `Startup`
5. startup presentation triggers if authored
6. the component optionally requests montage playback
7. delivery executes at the authored delivery time
8. delivery presentation triggers at that same timing point
9. `HitTrace` may also spawn a tracer from the resolved trace path
10. `Direct`, `HitTrace`, and `Melee` apply the instant effect on successful delivery
11. impact presentation triggers after successful effect application
12. `Projectile` commits on successful spawn, then the replicated projectile handles travel presentation locally and impact presentation later on hit
13. authority spends cost and starts cooldown on successful commit
14. the granted spec enters `Recovery` and later returns to `Idle`

## Runtime state versus authored data

Authored data answers:

* what the ability should do
* which delivery mode it uses
* how timing is configured
* which sockets or offsets delivery and presentation should use
* whether montage playback should be requested
* which startup, delivery, tracer, travel, and impact assets should play

Runtime state answers:

* whether the ability is granted
* whether it is idle, in startup, active, in recovery, or blocked
* what happened on the last activation request
* when its personal cooldown ends
* when startup began
* when delivery is scheduled
* when recovery ends
* which cooldown groups are active on the owner

Projectile travel presentation remains transient state on the spawned projectile actor, not on the data asset or the ability spec.

## How to use it

1. Create a `UMABSAbilityDefinition`.
2. Set `TargetType`, `DeliveryMode`, `InstantEffectType`, and `EffectMagnitude`.
3. Configure cooldown, cost, timing, and delivery sockets.
4. Fill the presentation groups that matter for that delivery mode.
5. Grant the ability on authority.
6. Activate it with `TryActivateAbilityByTag`.
7. Inspect `FMABSAbilitySpec`, recent debug events, and the overlay.

## Example

Example rifle shot:

* `DeliveryMode = HitTrace`
* `HitTraceOriginSocketName = Muzzle`
* `DeliveryPresentation.Cue.VFX = P_RifleMuzzle`
* `DeliveryPresentation.HitTraceTracer.TracerVFX = P_RifleTracer`
* `ImpactPresentation.Cue.VFX = P_BulletImpact`
