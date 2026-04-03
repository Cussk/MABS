# MABS Debugging

## What it is

This document explains the current Phase 7.5 debugging tools for MABS.

## Why it exists

Phase 7.5 adds grouped granting through ability sets. That workflow is only practical if the runtime can explain which set was processed, how many entries were granted, and which entries were skipped or rejected.

## What is available

Phase 7.5 debugging includes:

* structured `FMABSAbilityDebugEvent` output
* the latest `FMABSTargetTraceDebugInfo` snapshot for target traces, hit traces, and melee sweeps
* granted ability summaries with timing, combo queue, and cooldown state
* the current authority-side active periodic effect list through `GetActivePeriodicEffects()`
* an owning-client debug replication toggle on `UMABSAbilityComponent`
* optional trace and sweep debug drawing
* `UMABSDebugBlueprintLibrary`
* `AMABSDebugHUD`

## Important event names

Granting and grouped granting:

* `AbilityGranted`
* `AbilityGrantRejected`
* `AbilitySetGranted`
* `AbilitySetGrantSkipped`
* `AbilitySetGrantFailed`

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

For Phase 7.5 grouped grant problems, look for this order:

1. `AbilitySetGranted`, `AbilitySetGrantSkipped`, or `AbilitySetGrantFailed`
2. any underlying `AbilityGranted` or `AbilityGrantRejected` entries
3. activation, delivery, AoE, and periodic events after the grant succeeds

## Example

Example starting bundle flow:

* `AbilityGranted`
* `AbilityGranted`
* `AbilitySetGrantSkipped`
* `AbilitySetGranted`

That means two valid abilities were granted, one null entry was skipped, and the set finished successfully overall.
