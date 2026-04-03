# MABS Debugging

## What it is

This document explains the current Phase 7 debugging tools for MABS.

## Why it exists

Phase 7 adds combo queueing, AoE target gathering, and timer-driven periodic effects. Those systems are practical only if the runtime can explain when a combo window opened, which actors an AoE accepted, and when a periodic effect was applied, ticked, refreshed, or expired.

## What is available

Phase 7 debugging includes:

* structured `FMABSAbilityDebugEvent` output
* the latest `FMABSTargetTraceDebugInfo` snapshot for target traces, hit traces, and melee sweeps
* granted ability summaries with timing, combo queue, and cooldown state
* the current authority-side active periodic effect list through `GetActivePeriodicEffects()`
* an owning-client debug replication toggle on `UMABSAbilityComponent`
* optional trace and sweep debug drawing
* `UMABSDebugBlueprintLibrary`
* `AMABSDebugHUD`

## Important event names

Timing and combo events:

* `StartupStarted`
* `DeliveryScheduled`
* `DeliveryTriggered`
* `RecoveryStarted`
* `RecoveryCompleted`
* `ComboWindowStarted`
* `ComboWindowEnded`
* `ComboQueued`
* `ComboRejected`

AoE and gameplay result events:

* `AoEResolved`
* `AoETargetRejected`
* `EffectApplied`
* `EffectApplicationFailed`

Periodic events:

* `PeriodicEffectApplied`
* `PeriodicEffectRefreshed`
* `PeriodicEffectTick`
* `PeriodicEffectExpired`

Presentation and socket events from earlier phases still remain:

* `PresentationCueRouted`
* `PresentationCueSkipped`
* `PresentationCueRealized`
* `TracerCueRouted`
* `TracerCueRealized`
* `ProjectileTravelPresentationTriggered`
* `SocketResolved`
* `SocketFallbackUsed`
* `PresentationSocketFallbackUsed`

## How to use it

Read:

* `OnAbilityDebugEvent`
* `GetRecentDebugEvents()`
* `GetLatestTargetTraceDebugInfo()`
* `GetActivePeriodicEffects()`
* `LogMABSAbilitySystem`

For Phase 7 problems, look for this order:

1. combo window events if the ability should chain
2. delivery and target resolution events
3. `AoEResolved` and any `AoETargetRejected` events
4. `EffectApplied` or `EffectApplicationFailed`
5. periodic apply / refresh / tick / expire events when periodic data is enabled

## Example

Example burning combo slash flow:

* `StartupStarted`
* `ComboWindowStarted`
* `DeliveryTriggered`
* `MeleeHit`
* `AoEResolved`
* `EffectApplied`
* `PeriodicEffectApplied`
* `ComboQueued`
* `ComboWindowEnded`
* `RecoveryCompleted`
* `PeriodicEffectTick`
