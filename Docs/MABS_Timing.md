# MABS Timing

## What it is

This document explains the Phase 5 timing model for ability execution.

## Why it exists

Phase 4 delivery worked, but everything still felt immediate. Phase 5 adds explicit timing windows so abilities can line up with authored combat pacing.

## Authored fields

`UMABSAbilityDefinition` now exposes:

* `StartupDuration`
* `DeliveryTime`
* `RecoveryDuration`

`RecoveryDuration` is authored as the total desired ability length from activation start.

## How timing works

When activation is accepted on authority:

1. the granted spec enters `Startup`
2. delivery is scheduled using the authored delivery delay
3. delivery executes when that timer fires
4. successful delivery enters `Recovery`
5. recovery returns the granted spec to `Idle`

`DeliveryTime` is treated as time since activation start.
`RecoveryDuration` is also treated as time since activation start, but it marks when the full ability should be done.

If both `StartupDuration` and `DeliveryTime` are authored, MABS uses the later of the two values so delivery does not fire before startup completes.

The practical rule is:

* delivery fires at `max(StartupDuration, DeliveryTime)`
* the ability ends at `max(delivery time actually used, RecoveryDuration)`
* the runtime recovery window is whatever time remains between delivery and the authored total end time

## Runtime state

Granted specs now move through:

* `Idle`
* `Startup`
* `Active`
* `Recovery`
* `Blocked`

Runtime timestamps are stored on `FMABSAbilitySpec`:

* `ActivationStartTime`
* `ScheduledDeliveryTime`
* `RecoveryEndTime`

## Example

Example authored timing:

* `StartupDuration = 0.15`
* `DeliveryTime = 0.2`
* `RecoveryDuration = 0.5`

Result:

* activation enters `Startup`
* delivery fires at 0.20 seconds after activation
* the full ability ends at 0.50 seconds after activation
* runtime recovery lasts 0.30 seconds after delivery

Example cumulative authoring:

* `StartupDuration = 0.5`
* `DeliveryTime = 0.5`
* `RecoveryDuration = 1.6`

Result:

* delivery fires at 0.5 seconds
* the ability ends at 1.6 seconds total
* runtime recovery lasts 1.1 seconds

## Test checklist

Singleplayer:

* startup, delivery, and recovery state transitions appear in order
* zero recovery still emits readable state events

Listen server:

* remote client requests still schedule on authority
* the owning client sees authoritative timing events

Dedicated server:

* timer-based delivery remains authoritative
* no tick dependency is required
