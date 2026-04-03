# MABS Modules

## What it is

This document explains the current module layout for MABS after Phase 6 presentation support.

## Why it exists

Phase 6 adds authored presentation structs and runtime cosmetic routing, but the module split still keeps authored data, runtime execution, and debug display separate and readable.

## Module map

### `MABSCore`

Owns:

* `UMABSAbilityDefinition`
* ability runtime structs and enums
* authored timing fields
* authored socket fields
* authored montage references
* authored presentation structs and fields
* shared debug structs

### `MABSGameplay`

Owns:

* `UMABSAbilityComponent`
* `AMABSProjectileBase`
* timer-based startup, delivery, and recovery scheduling
* socket transform resolution
* montage request hooks
* authority-side delivery execution
* startup, delivery, tracer, and impact presentation triggers
* projectile travel presentation hookup
* instant effects
* cooldown and cost integration
* replication and multicast cosmetic routing

### `MABSDebug`

Owns:

* runtime-safe debug formatting
* the HUD overlay
* display of timing state, presentation events, recent delivery results, and the latest trace snapshot

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
