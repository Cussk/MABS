# MABS Overview

## What MABS is

MABS is a plugin-first, multiplayer-ready, data-driven ability framework for Unreal Engine.

As of Phase 13, it now has two equally important surfaces:

* the runtime framework in the `MABS` plugin
* a sample-first proof slice in the project module that lets a user open one map and understand the framework quickly

The runtime path supports:

* granting authored abilities and ability sets
* server-authoritative activation
* cooldowns, costs, startup, delivery, and recovery timing
* direct, hit trace, melee, and projectile delivery
* custom delivery handlers in C++ or Blueprint
* combos, AoE, and periodic effects
* cue-driven presentation and structured debug events
* editor validation for common invalid authoring

The sample path now adds:

* a readable `AMABSDemoHUD`
* sample-only authored display data through `UMABSDemoDisplayConfig`
* sample character vitals that replicate cleanly to the owning client for HUD readability

## Why it exists

The framework is useful only if people can both build with it and understand it quickly.

The plugin layers solve the runtime problem. Phase 13 adds the onboarding layer so a new user can open the sample scene, trigger a few abilities, and see cooldowns, hit results, combo timing, periodic effects, and custom delivery behavior without living in raw logs first.

## Current module ownership

* `MABSCore` owns authored ability data, ability-set data, shared runtime structs, and shared debug summary types.
* `MABSGameplay` owns granting, activation, delivery, delivery handlers, effects, combos, AoE, periodic timers, presentation routing, replication, and the authoritative runtime state on `UMABSAbilityComponent`.
* `MABSDebug` owns the technical runtime harness HUD and formatting helpers.
* `MABSEditor` owns authoring validation.
* The project `MABS` module owns the Phase 13 sample-facing layer: `AMABSDemoHUD`, `UMABSDemoDisplayConfig`, the sample controller toggles, the sample custom knockback handler example, and the sample character vitals used by the readable overlay.

## Current ability flow

1. A server-owned actor is granted one or more `UMABSAbilityDefinition` assets directly or through a `UMABSAbilitySet`.
2. Input or gameplay code calls `TryActivateAbilityByTag`.
3. Authority validates grant, cooldown, cost, and authored gameplay effect data.
4. Authority enters `Startup`, schedules delivery, and may open a combo window later.
5. Authority resolves the authored custom handler or the built-in handler for the selected delivery mode.
6. Delivery resolves a primary target and may expand into AoE targets.
7. Authority applies instant effects and optionally starts or refreshes periodic effects.
8. Authority commits cost, starts cooldown, enters recovery, and may auto-trigger a queued combo follow-up.
9. Authority emits structured debug events and updates debug summaries on `UMABSAbilityComponent`.
10. The Phase 13 demo HUD reads those existing summaries and events on the owning client to present a simpler sample-facing view.

## How to use it

1. Start with the sample scene, not a blank actor.
2. Use `AMABSGameMode` or a derived sample game mode so the HUD defaults to `AMABSDemoHUD`.
3. Add `UMABSAbilityComponent` to the sample pawn or character.
4. Author one or more `UMABSAbilityDefinition` assets.
5. Optionally group them with `UMABSAbilitySet`.
6. If you want readable sample labels and instructions, create a `UMABSDemoDisplayConfig` asset and assign it on a Blueprint subclass of `AMABSDemoHUD`.
7. Grant abilities on authority and activate them by tag.
8. Use `F1` for help, `F2` for the technical debug harness, and `F3` for validation notes.

## Phase 13 onboarding note

Phase 13 does not add a new gameplay execution subsystem.

It adds a better first-use path:

* the framework still runs through `UMABSAbilityComponent`
* the demo HUD reads existing runtime/debug state
* the sample-only display config adds labels and instructions without changing gameplay authority
