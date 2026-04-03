# PHASE_08 - Runtime Debug Harness

## What shipped

Phase 8 adds a real runtime debug harness for MABS.

The implemented Phase 8 feature set is:

* a sectioned `AMABSDebugHUD` runtime harness
* a master harness toggle
* per-category visibility toggles
* categorized `FMABSAbilityDebugEvent` entries
* compact harness summary structs in `MABSCore`
* gameplay query helpers for harness summaries
* owner-visible periodic-effect summary replication for debug inspection
* updated runtime debugging documentation

## Why it matters

Earlier phases already had useful logs and a lightweight overlay, but the system had outgrown that.

Phase 8 makes MABS easier to inspect during real gameplay testing:

* users can see granted abilities and current runtime state
* activation failures are easier to read
* targeting and delivery outcomes are visible in one place
* combo and periodic state no longer require digging through logs
* cue and presentation routing stays readable

## What changed

### `MABSCore`

* added `EMABSDebugEventCategory`
* expanded `FMABSAbilityDebugEvent` with `Category`
* added:
  * `FMABSGrantedAbilityDebugSummary`
  * `FMABSCooldownGroupDebugSummary`
  * `FMABSComboDebugSummary`
  * `FMABSPeriodicEffectDebugSummary`

### `MABSGameplay`

* `UMABSAbilityComponent` now exposes:
  * `GetGrantedAbilityDebugSummaries()`
  * `GetCooldownGroupDebugSummaries()`
  * `GetComboDebugSummary()`
  * `GetPeriodicEffectDebugSummaries()`
* active periodic effect summaries now replicate owner-only for harness inspection when debug replication is enabled
* debug events are now categorized when they are emitted

### `MABSDebug`

* replaced the thin overlay with a full sectioned harness in `AMABSDebugHUD`
* added harness category toggles
* added compact formatting helpers in `UMABSDebugBlueprintLibrary`
* recent event history now displays category-aware filtered output

## Implemented runtime path

Harness flow:

* gameplay still emits the normal structured debug events
* gameplay still owns current runtime state
* gameplay now exposes compact summary views for the harness
* `AMABSDebugHUD` reads those summaries and recent events locally
* the owning client can inspect their own authoritative debug state during multiplayer testing

## State model

Phase 8 does not add a second gameplay state system.

The harness reads existing runtime state:

* `GrantedAbilities`
* `CooldownGroupStates`
* `RecentDebugEvents`
* `LatestTargetTraceDebugInfo`
* `ActivePeriodicEffects`

The new summary structs are read models for display, not replacement gameplay containers.

## Runtime enable and category controls

Phase 8 adds:

* `mabs.DebugHarness`
* `mabs.DebugHarness.General`
* `mabs.DebugHarness.Activation`
* `mabs.DebugHarness.Targeting`
* `mabs.DebugHarness.Delivery`
* `mabs.DebugHarness.CostCooldown`
* `mabs.DebugHarness.Combo`
* `mabs.DebugHarness.Periodic`
* `mabs.DebugHarness.Presentation`

## Documentation updated

Phase 8 updates or adds:

* `Docs/MABS_API.md`
* `Docs/MABS_Debugging.md`
* `Docs/MABS_DebugHarness.md`
* `Docs/MABS_Modules.md`
* `Docs/MABS_Multiplayer.md`
* `Docs/MABS_Overview.md`
* `Docs/MABS_QuickStart.md`
* `Docs/Phases/PHASE_08.md`

## Verification

Recommended runtime validation:

* singleplayer:
  * toggle the harness on and off
  * read granted abilities, cooldown groups, combo state, and periodic entries
* listen server:
  * inspect local host state
  * inspect remote owning-client authoritative results
* dedicated server:
  * inspect owning-client event history, latest trace state, and periodic summaries
  * verify gameplay still works when the harness is disabled
