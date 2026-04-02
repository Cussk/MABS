# MABS Modules

## What it is

This document explains the current module layout for MABS after Phase 3 cooldown and cost support.

## Why it exists

Cooldowns and costs are common gameplay rules, but they should not collapse authored data, runtime execution, and debug presentation back into one layer. The module split keeps those responsibilities readable and marketplace-friendly.

## Module map

### `MABSCore`

What it is:

* the foundational runtime module

Why it exists:

* it keeps shared authored data and runtime types stable
* it gives higher-level modules one clean dependency for ability definitions, granted specs, and cooldown-group state

How to use it:

* create `UMABSAbilityDefinition` assets
* read `FMABSAbilitySpec`, `FMABSCooldownGroupState`, `FMABSAbilityDebugEvent`, and `FMABSTargetTraceDebugInfo`

### `MABSGameplay`

What it is:

* the primary runtime gameplay module

Why it exists:

* it owns the server-authoritative execution path
* it keeps grant, activation, cooldowns, costs, target resolution, effect application, and replication together

How to use it:

* add `UMABSAbilityComponent` to an actor or character
* grant abilities on authority
* call `TryActivateAbilityByTag`
* use the cooldown query helpers
* optionally implement `IMABSInstantEffectReceiver` and `IMABSCostReceiver` on actors that should receive heals or pay cost

### `MABSDebug`

What it is:

* the runtime-safe debug tooling module

Why it exists:

* it keeps debug consumers out of the core gameplay execution path
* it provides formatting helpers and the lightweight runtime overlay

How to use it:

* format debug events, target traces, and granted ability runtime summaries
* use `AMABSDebugHUD` for a quick inspection layer in PIE

### `MABSEditor`

What it is:

* the editor-only module

Why it exists:

* runtime and server builds should not depend on editor code
* future validation and authoring tools need an isolated home

## Dependency direction

Current dependency flow is:

* `MABSCore` depends on no other MABS modules
* `MABSGameplay` depends on `MABSCore`
* `MABSDebug` depends on `MABSCore` and consumes `MABSGameplay` runtime data for the overlay
* `MABSEditor` may depend on runtime modules

## Host project versus plugin content

The plugin remains the reusable runtime product. The host project still contains the local verification harness.

Host-project-only example items now include:

* `Config/DefaultGameplayTags.ini`
* example content assets
* minimal health and resource state on `AMABSCharacter`
* the default `AMABSGameMode` wiring that opts into `AMABSDebugHUD`
