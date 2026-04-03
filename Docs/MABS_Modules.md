# MABS Modules

## What it is

This document explains the current Phase 6.5 module layout for MABS after lightweight presentation cue routing was added.

## Why it exists

Phase 6.5 adds new shared cue types and routing behavior, but the module split still keeps authored data, runtime execution, and debug display separate and readable.

## Module map

### `MABSCore`

Owns:

* `UMABSAbilityDefinition`
* ability runtime structs and enums
* authored timing fields
* authored socket fields
* authored montage references
* authored presentation structs and fields, including Niagara VFX references
* `EMABSPresentationCueVisibilityPolicy`
* `EMABSPresentationCuePhase`
* `FMABSPresentationCueEvent`
* `FMABSTracerCueEvent`
* `FMABSProjectileTravelCueEvent`
* shared debug structs

### `MABSGameplay`

Owns:

* `UMABSAbilityComponent`
* `AMABSProjectileBase`
* timer-based startup, delivery, and recovery scheduling
* socket transform resolution
* montage request hooks
* authority-side delivery execution
* cue routing helpers for startup, delivery, tracer, travel, and impact
* owner-only, local-only, and relevant-client cue handling
* local cue realization helpers
* pooling-friendly Niagara reuse for repeated world cues and tracers
* projectile travel cue hookup on the replicated projectile actor
* instant effects
* cooldown and cost integration
* replication and cosmetic routing

### `MABSDebug`

Owns:

* runtime-safe debug formatting
* the HUD overlay
* display of timing state, trace state, and recent cue-routing events

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

Runtime presentation still depends on the engine `Niagara` module for authored VFX types and local system realization.
