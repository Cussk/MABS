# MABS Debugging

## What it is

This document explains the current Phase 4 debugging tools for MABS.

## Why it exists

Delivery adds more ways for an activation to fail. Good multiplayer behavior is only useful if users can tell whether the request failed during validation, delivery, effect application, projectile spawn, or projectile impact.

## What is available

Phase 4 debugging includes:

* structured `FMABSAbilityDebugEvent` output
* the latest `FMABSTargetTraceDebugInfo` snapshot for direct traces, hit traces, and melee sweeps
* granted ability summaries with cooldown remaining and delivery mode
* cooldown query helpers
* optional trace and sweep debug drawing
* `UMABSDebugBlueprintLibrary`
* `AMABSDebugHUD`

## Important event names

Delivery events:

* `DeliveryStarted`
* `DeliveryFailed`
* `HitTraceHit`
* `HitTraceRejected`
* `MeleeHit`
* `MeleeRejected`
* `ProjectileSpawned`
* `ProjectileSpawnFailed`
* `ProjectileImpact`
* `ProjectileImpactRejected`

Commit events:

* `EffectApplied`
* `EffectApplicationFailed`
* `CostSpent`
* `CooldownStarted`
* `CommitSucceeded`

## How to use it

### Structured events

Read:

* `OnAbilityDebugEvent`
* `GetRecentDebugEvents()`
* `LogMABSAbilitySystem`

### Latest trace or sweep snapshot

Use `GetLatestTargetTraceDebugInfo()` on `UMABSAbilityComponent`.

That snapshot now covers:

* direct actor targeting traces
* hit-trace delivery
* melee delivery

### Runtime overlay

`AMABSDebugHUD` still provides the built-in runtime overlay.

It now shows:

* the latest trace or sweep snapshot
* granted abilities with delivery mode and cooldown state
* the recent event list

### Blueprint formatting helpers

`UMABSDebugBlueprintLibrary` formats:

* debug events
* trace and sweep snapshots
* granted ability runtime summaries

## Example workflow

Example projectile debugging flow:

1. Create a damage ability with `DeliveryMode = Projectile`.
2. Set `ProjectileActorClass`.
3. Grant it on authority.
4. Activate it from a client.
5. Verify:
   * `DeliveryStarted`
   * `ProjectileSpawned`
   * `CostSpent`
   * `CooldownStarted`
   * `CommitSucceeded`
6. Let the projectile hit a valid actor.
7. Verify:
   * `EffectApplied`
   * `ProjectileImpact`

## Test checklist

Singleplayer:

* each delivery mode produces readable success events
* delivery failures produce readable rejection reasons
* projectile spawn failures are visible

Listen server:

* the host sees authoritative delivery events immediately
* remote clients receive authoritative delivery and impact events

Dedicated server:

* runtime debug helpers remain runtime-safe
* no editor-only dependency leaks into server builds
