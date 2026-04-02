# MABS Modules

## What it is

This document explains the current module layout for MABS after Phase 4 delivery support.

## Why it exists

Delivery adds common combat behavior, but it should not collapse authored data, runtime execution, projectile runtime, and debug presentation into one layer. The module split keeps those responsibilities readable and marketplace-friendly.

## Module map

### `MABSCore`

Owns:

* `UMABSAbilityDefinition`
* `EMABSDeliveryMode`
* ability runtime structs and enums
* shared debug structs

### `MABSGameplay`

Owns:

* `UMABSAbilityComponent`
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
* display of recent delivery results and the latest trace snapshot

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

## Why projectile data still lives cleanly

`ProjectileActorClass` is authored on `UMABSAbilityDefinition` in `MABSCore`, but the actual projectile actor implementation stays in `MABSGameplay`.

That keeps:

* authored ability data in the core module
* projectile execution logic in gameplay

## Host project versus plugin

The plugin remains the reusable runtime product. The host project still provides the local harness content, sample actors, and example game mode wiring.
