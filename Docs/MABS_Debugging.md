# MABS Debugging

## What it is

This document explains the current Phase 5 debugging tools for MABS.

## Why it exists

Phase 5 adds timing windows, socket resolution, and optional montage requests. Good multiplayer behavior is only useful if users can tell whether an activation failed during validation, startup, delivery scheduling, socket resolution, effect application, projectile spawn, or projectile impact.

## What is available

Phase 5 debugging includes:

* structured `FMABSAbilityDebugEvent` output
* the latest `FMABSTargetTraceDebugInfo` snapshot for direct traces, hit traces, and melee sweeps
* granted ability summaries with runtime timing state and cooldown remaining
* an owning-client debug replication toggle on `UMABSAbilityComponent`
* cooldown query helpers
* optional trace and sweep debug drawing
* `UMABSDebugBlueprintLibrary`
* `AMABSDebugHUD`

## Important event names

Timing and socket events:

* `StartupStarted`
* `DeliveryScheduled`
* `DeliveryTriggered`
* `RecoveryStarted`
* `RecoveryCompleted`
* `SocketResolved`
* `SocketFallbackUsed`
* `MontagePlayRequested`
* `MontagePlayFailed`

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

### Owning-client debug replication toggle

Use `SetDebugReplicationEnabled(...)` on `UMABSAbilityComponent` when you want to stop authoritative debug events and target-trace snapshots from RPCing back to the owning client.

This is useful when:

* profiling listen-server or multiplayer projectile performance
* keeping authoritative gameplay active while reducing debug RPC traffic

When disabled:

* server-side debug events still record locally
* local HUD and log inspection on the authority side still work
* owning clients stop receiving debug event and trace RPC updates

### Latest trace or sweep snapshot

Use `GetLatestTargetTraceDebugInfo()` on `UMABSAbilityComponent`.

That snapshot covers:

* direct actor targeting traces
* hit-trace delivery
* melee delivery

### Runtime overlay

`AMABSDebugHUD` provides the built-in runtime overlay.

It shows:

* the latest target or delivery trace snapshot
* granted abilities with runtime state, timing, and cooldown state
* the recent event list

### Blueprint formatting helpers

`UMABSDebugBlueprintLibrary` formats:

* debug events
* trace and sweep snapshots
* granted ability runtime summaries

## Example workflow

Example rifle-shot debugging flow:

1. Create a damage ability with `DeliveryMode = HitTrace`.
2. Set `HitTraceOriginSocketName = Muzzle`.
3. Set `StartupDuration = 0.1`, `DeliveryTime = 0.1`, and `RecoveryDuration = 0.3`.
4. Grant it on authority.
5. Activate it from a client.
6. Verify:
   * `StartupStarted`
   * `DeliveryScheduled`
   * `SocketResolved` or `SocketFallbackUsed`
   * `DeliveryTriggered`
   * `HitTraceHit`
   * `EffectApplied`
   * `RecoveryStarted`
   * `RecoveryCompleted`

## Test checklist

Singleplayer:

* each delivery mode produces readable timing and delivery events
* socket fallback behavior is visible
* montage request failures are visible when relevant

Listen server:

* the host sees authoritative timing and socket events immediately
* remote clients receive authoritative timing, delivery, and impact events

Dedicated server:

* runtime debug helpers remain runtime-safe
* no editor-only dependency leaks into server builds
