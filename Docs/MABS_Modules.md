# MABS Modules

## What it is

This document explains the current Phase 7.5 module layout for MABS after ability-set authoring and grouped grant support were added.

## Why it exists

Phase 7.5 adds new grouped authoring and grouped grant flow, but the module split still keeps authored data, runtime execution, and debug display separate and readable.

## Module map

### `MABSCore`

Owns:

* `UMABSAbilityDefinition`
* `UMABSAbilitySet`
* ability runtime structs and enums
* authored timing fields
* authored socket fields
* authored montage references
* authored presentation structs and fields
* authored combo data
* authored AoE data
* authored periodic effect data
* `FMABSAbilitySpec`
* `FMABSActivePeriodicEffect`
* shared debug structs

### `MABSGameplay`

Owns:

* `UMABSAbilityComponent`
* `AMABSProjectileBase`
* single-ability granting
* grouped set granting
* timer-based startup, delivery, recovery, combo, and periodic scheduling
* authority-side combo queueing
* authority-side AoE resolution
* authority-side instant and periodic effect application
* periodic reapply refresh behavior
* socket transform resolution
* montage request hooks
* authority-side delivery execution
* cue routing helpers for startup, delivery, tracer, travel, and impact
* owner-only, local-only, and relevant-client cue handling
* local cue realization helpers
* cooldown and cost integration
* replication and cosmetic routing

### `MABSDebug`

Owns:

* runtime-safe debug formatting
* the HUD overlay
* display of grant, grouped-grant, timing state, combo queue state, trace state, and recent gameplay events

### `MABSEditor`

Owns:

* editor-only extensions
* future validation and authoring helpers

## Dependency direction

Current dependency flow is:

* `MABSCore` depends on no other MABS modules
* `MABSGameplay` depends on `MABSCore`
* `MABSDebug` depends on `MABSCore` and consumes `MABSGameplay` runtime data
* `MABSEditor` may depend on runtime modules
