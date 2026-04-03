# MABS Debugging

## What it is

This document explains the current Phase 6 debugging tools for MABS.

## Why it exists

Phase 6 adds startup, delivery, tracer, projectile-travel, and impact presentation. Multiplayer-safe cosmetics are only useful if users can tell exactly when a cue triggered, which fallback path was used, and whether gameplay succeeded even when cosmetics were optional.

## What is available

Phase 6 debugging includes:

* structured `FMABSAbilityDebugEvent` output
* the latest `FMABSTargetTraceDebugInfo` snapshot for direct traces, hit traces, and melee sweeps
* granted ability summaries with runtime timing state and cooldown remaining
* an owning-client debug replication toggle on `UMABSAbilityComponent`
* cooldown query helpers
* optional trace and sweep debug drawing
* `UMABSDebugBlueprintLibrary`
* `AMABSDebugHUD`

## Important event names

Timing and presentation events:

* `StartupStarted`
* `StartupPresentationTriggered`
* `DeliveryScheduled`
* `DeliveryTriggered`
* `DeliveryPresentationTriggered`
* `ImpactPresentationTriggered`
* `TracerSpawned`
* `TracerSpawnFailed`
* `ProjectileTravelPresentationTriggered`
* `RecoveryStarted`
* `RecoveryCompleted`

Socket and fallback events:

* `SocketResolved`
* `SocketFallbackUsed`
* `PresentationSocketFallbackUsed`

Delivery and impact events:

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

## How to use it

Read:

* `OnAbilityDebugEvent`
* `GetRecentDebugEvents()`
* `GetLatestTargetTraceDebugInfo()`
* `LogMABSAbilitySystem`

Example rifle-shot flow:

* `StartupStarted`
* `StartupPresentationTriggered`
* `DeliveryScheduled`
* `DeliveryTriggered`
* `DeliveryPresentationTriggered`
* `TracerSpawned`
* `HitTraceHit`
* `EffectApplied`
* `ImpactPresentationTriggered`
* `RecoveryStarted`
* `RecoveryCompleted`
