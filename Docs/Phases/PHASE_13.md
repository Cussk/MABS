# PHASE_13 - Sample Scene, Demo HUD, and Quickstart Proof Slice

## What shipped

Phase 13 adds the first sample-facing onboarding layer for MABS.

The implemented C++ result includes:

* `AMABSDemoHUD` in the project `MABS` module
* `UMABSDemoDisplayConfig` for sample-only hotbar/help/validation authoring
* `AMABSGameMode` now defaulting to `AMABSDemoHUD`
* `AMABSPlayerController` bindings for `F1`, `F2`, and `F3`
* replicated example health/resource state on `AMABSCharacter`
* a Blueprint-callable `RestoreExampleVitals()` helper on `AMABSCharacter`
* quickstart and user-facing doc updates that treat the sample map as the first onboarding path

## Why it matters

By Phase 12 the framework was technically solid, but the first-use experience still leaned too hard on logs and docs.

Phase 13 makes the framework easier to understand quickly by adding a readable sample overlay without moving gameplay authority out of the real runtime.

## What changed

### Project sample layer

The project `MABS` module now owns the readable sample HUD and its sample-only display config:

* `AMABSDemoHUD`
* `UMABSDemoDisplayConfig`

That keeps onboarding text, hotbar labels, and station summaries out of the core plugin runtime.

### Sample controller flow

`AMABSPlayerController` now binds:

* `F1` to toggle demo help
* `F2` to toggle the technical debug harness
* `F3` to toggle validation notes

### Sample vitals

`AMABSCharacter` now replicates its example health/resource values so the sample HUD reads correct numbers on the owning client in listen server and dedicated server play.

## Which subsystem owns it

Ownership is intentionally split:

* `MABSGameplay` still owns authoritative ability state and execution
* `MABSDebug` still owns the technical debug harness
* the project `MABS` module owns the readable sample HUD and sample-only display config

Phase 13 does not add a second gameplay execution subsystem.

## What state it uses

The Phase 13 demo HUD reads existing runtime state from `UMABSAbilityComponent`:

* granted ability debug summaries
* combo summary
* latest target trace debug info
* periodic effect summaries
* recent debug events

It also reads sample vitals from `AMABSCharacter`:

* current example health
* max example health
* current example resource
* max example resource

`UMABSDemoDisplayConfig` is sample display data only. It does not create authoritative gameplay state.

## Editor work still expected

This phase still needs in-editor/sample-content work to be fully presentable:

* sample map layout
* station labels
* sample ability assets
* sample ability set
* target dummies, bars, reactions, and reset flow
* a Blueprint subclass of `AMABSDemoHUD` with an assigned `UMABSDemoDisplayConfig`

The C++ work here is the support layer for that authoring.

## How to use it

1. Create a sample HUD Blueprint from `AMABSDemoHUD`.
2. Create a `UMABSDemoDisplayConfig` asset and assign it on that HUD Blueprint.
3. Use `AMABSGameMode` or a derived game mode that references the HUD Blueprint.
4. Make sure the sample pawn has `UMABSAbilityComponent`.
5. Grant the sample abilities on authority.
6. Play the sample map and use `F1`, `F2`, and `F3` as needed.

## Example

Example Phase 13 sample HUD config:

* `OverlayTitle = "MABS Sample Scene"`
* one hotbar entry per sample ability tag
* help entries for station order and controls
* validation entries for multiplayer smoke tests

Example multiplayer-safe sample character behavior:

* authority applies damage, heal, and cost changes
* `CurrentExampleHealth` and `CurrentExampleResource` replicate
* the owning client demo HUD reads the replicated values directly

## Documentation updated

Phase 13 updates:

* `Docs/MABS_Overview.md`
* `Docs/MABS_Abilities.md`
* `Docs/MABS_Delivery.md`
* `Docs/MABS_QuickStart.md`
* `Docs/Phases/PHASE_13.md`

## Verification

Recommended verification for this phase:

* standalone: confirm the demo HUD shows hotbar state, target readout, status, and feature callouts
* listen server: confirm health/resource, cooldowns, combo state, and periodic readouts stay correct on the owning client
* dedicated server: confirm the readable overlay still reflects replicated ability state and sample vitals
* sample map pass: confirm `F1`, `F2`, and `F3` work and the technical harness can still be shown on top of the demo HUD
