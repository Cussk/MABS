# MABS Montages

## What it is

This document explains the optional montage hook added in Phase 5.

## Why it exists

Abilities often need animation alignment, but Phase 5 keeps timing data-driven instead of requiring animation notifies.

## Authored fields

Optional montage fields on `UMABSAbilityDefinition`:

* `ActivationMontage`
* `MontagePlayRate`

## Runtime behavior

If `ActivationMontage` is set:

1. the ability enters `Startup`
2. `UMABSAbilityComponent` requests montage playback
3. delivery timing still follows the authored timing data

Montage playback does not replace the timer-based timing model in this phase.

If montage playback cannot be requested cleanly on the local instance, MABS emits `MontagePlayFailed` but does not hard-fail the ability.

## Example

Example sword slash:

* `StartupDuration = 0.1`
* `DeliveryTime = 0.12`
* `RecoveryDuration = 0.4`
* `ActivationMontage = Slash_Montage`

The montage starts during startup, the melee delivery still fires from the authored timer, and the ability still returns to `Idle` when the authored total duration is reached.

## Not included

Phase 5 does not add:

* notify-driven execution as the primary path
* montage branching systems
* combo chains

## Test checklist

Singleplayer:

* authored timing still works with and without a montage
* montage request failures do not cancel the ability

Listen server:

* host and remote client abilities still follow authority timing

Dedicated server:

* montage hooks do not introduce editor-only or client-only runtime dependencies
