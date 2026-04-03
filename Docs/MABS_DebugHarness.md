# MABS Debug Harness

## What it is

The MABS debug harness is the Phase 9.5 runtime inspection panel built around `AMABSDebugHUD`.

It is a gameplay-facing inspection tool, not an Unreal profiling replacement.

## Why it exists

By Phase 9.5, MABS now includes:

* grouped granting
* timing
* multiple delivery modes
* combos
* AoE
* periodic effects
* presentation and cue routing

That feature set needs a single readable runtime view.

## What it shows

The harness currently shows:

* owner and replication status
* granted abilities
* latest activation result
* latest enabled failure reason
* latest targeting and delivery snapshot
* current cooldown groups
* combo state
* active periodic effects
* recent presentation and cue activity
* recent event history filtered by category

## Which subsystem owns it

`MABSDebug` owns:

* the HUD panel
* section layout
* category toggles
* formatting
* event filtering

`MABSGameplay` owns the query helpers and debug runtime data that feed it.

Phase 9.5 keeps display concerns in `MABSDebug`, while debug read-model assembly now lives in `MABSAbilityRuntime_Debug.cpp`.

## What state it uses

The harness reads existing runtime state from `UMABSAbilityComponent`:

* `GrantedAbilities`
* `CooldownGroupStates`
* `RecentDebugEvents`
* `LatestTargetTraceDebugInfo`
* `ActivePeriodicEffects`

It also uses the summary accessors:

* `GetGrantedAbilityDebugSummaries()`
* `GetCooldownGroupDebugSummaries()`
* `GetComboDebugSummary()`
* `GetPeriodicEffectDebugSummaries()`

## How to enable it

1. Set your HUD class to `AMABSDebugHUD` or a subclass.
2. Make sure debug replication is enabled on authority.
3. Use the master console variable:
   * `mabs.DebugHarness 1`
4. Optionally narrow the display:
   * `mabs.DebugHarness.General`
   * `mabs.DebugHarness.Activation`
   * `mabs.DebugHarness.Targeting`
   * `mabs.DebugHarness.Delivery`
   * `mabs.DebugHarness.CostCooldown`
   * `mabs.DebugHarness.Combo`
   * `mabs.DebugHarness.Periodic`
   * `mabs.DebugHarness.Presentation`

## Example

Example local test flow:

1. grant a starter ability set on begin play
2. activate a rifle shot
3. activate a sword combo chain
4. activate a projectile ability with DOT
5. read:
   * the granted ability entries
   * the targeting / delivery section
   * the combo section
   * the periodic section
   * the recent event history

## What it does not try to solve

The harness does not add:

* editor visualizers
* a remote cross-player debugger
* profiling or significance analysis
* replay analysis
* a new telemetry ingestion system

It is a runtime gameplay inspection layer.
