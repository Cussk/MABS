# MABS Modules

## What it is

This document explains the current module layout for MABS after Phase 5 timing, sockets, and montage support.

## Why it exists

Phase 5 adds authored timing, socket fields, runtime scheduling, and optional montage hooks. The module split keeps those responsibilities readable and marketplace-friendly instead of collapsing them into one large gameplay class.

## Module map

### `MABSCore`

Owns:

* `UMABSAbilityDefinition`
* `EMABSDeliveryMode`
* ability runtime structs and enums
* authored timing fields
* authored socket fields
* authored montage references
* shared debug structs

### `MABSGameplay`

Owns:

* `UMABSAbilityComponent`
* timer-based startup, delivery, and recovery scheduling
* socket transform resolution
* montage request hooks
* authority-side delivery execution
* direct target resolution
* hit-trace and melee traces
* projectile spawn and impact handling
* instant effects
* cooldown and cost integration
* replication

### `MABSDebug`

Owns:

* runtime-safe debug formatting
* the HUD overlay
* display of timing state, recent delivery results, and the latest trace snapshot

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

## Why timing and projectile data still live cleanly

`ProjectileActorClass`, socket names, timing values, and optional montage references are authored on `UMABSAbilityDefinition` in `MABSCore`, but the actual scheduling, projectile runtime, and playback requests stay in `MABSGameplay`.

That keeps:

* authored ability data in the core module
* runtime scheduling and projectile execution logic in gameplay

## Host project versus plugin

The plugin remains the reusable runtime product. The host project still provides the local harness content, sample actors, and example game mode wiring.
