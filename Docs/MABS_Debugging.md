# MABS Debugging

## What it is

This document explains the Phase 8 runtime debugging model for MABS.

Phase 8 turns the old lightweight overlay into a real runtime harness that is useful during standalone, listen-server, and dedicated-server client testing.

## Why it exists

By Phase 8, MABS already has enough runtime breadth that raw logs alone are not enough.

Users need to inspect:

* granted abilities and their current state
* latest activation success or failure
* targeting and delivery results
* current cooldown groups and recent cost failures
* combo state
* active periodic effects
* recent presentation and cue routing activity

## What is available

Phase 8 debugging now includes:

* structured `FMABSAbilityDebugEvent` output with `Category`
* the latest `FMABSTargetTraceDebugInfo` snapshot
* `FMABSGrantedAbilityDebugSummary`
* `FMABSCooldownGroupDebugSummary`
* `FMABSComboDebugSummary`
* `FMABSPeriodicEffectDebugSummary`
* `GetGrantedAbilityDebugSummaries()`
* `GetCooldownGroupDebugSummaries()`
* `GetComboDebugSummary()`
* `GetPeriodicEffectDebugSummaries()`
* owner-routed recent event history
* owner-routed target trace snapshots
* owner-only periodic summary replication when debug replication is enabled
* category-aware formatting helpers in `UMABSDebugBlueprintLibrary`
* the full `AMABSDebugHUD` runtime harness

## Event categories

Phase 8 keeps the existing event system, but each event now carries a lightweight category:

* `General`
* `Activation`
* `Targeting`
* `Delivery`
* `CostCooldown`
* `Combo`
* `Periodic`
* `Presentation`

This makes it practical to filter the runtime harness without inventing a second telemetry system.

## How to enable it

1. Set your HUD class to `AMABSDebugHUD` or a subclass of it.
2. Leave debug replication enabled on `UMABSAbilityComponent`, or call `SetDebugReplicationEnabled(true)` on authority.
3. Use the master toggle:
   * `mabs.DebugHarness 1`
   * `mabs.DebugHarness 0`
4. Narrow the panel with category toggles:
   * `mabs.DebugHarness.General`
   * `mabs.DebugHarness.Activation`
   * `mabs.DebugHarness.Targeting`
   * `mabs.DebugHarness.Delivery`
   * `mabs.DebugHarness.CostCooldown`
   * `mabs.DebugHarness.Combo`
   * `mabs.DebugHarness.Periodic`
   * `mabs.DebugHarness.Presentation`

## What the harness shows

When enabled, `AMABSDebugHUD` shows:

* status and replication context
* granted ability summaries
* latest activation result and latest enabled failure
* latest targeting and delivery context
* current cooldown groups
* current combo state
* active periodic effect summaries
* recent presentation and cue activity
* recent event history filtered by the enabled categories

## Multiplayer note

The harness is local-owner inspection first.

That means:

* authority still decides gameplay truth
* the owning client receives recent debug events and target trace snapshots when debug replication is enabled
* active periodic effect summaries are replicated owner-only for harness inspection
* disabling the harness does not change gameplay behavior

## Example

Example dedicated-server client workflow:

1. grant a starter ability set on the server
2. set the HUD class to `AMABSDebugHUD`
3. enable `mabs.DebugHarness 1`
4. fire a projectile ability, then apply a DOT
5. read:
   * the granted ability entry
   * the latest targeting / delivery line
   * the recent failure line if activation was denied
   * the periodic effect entry with next-tick and remaining-time data
