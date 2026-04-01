# MABS Overview

## What MABS is

MABS stands for **Multiplayer Ability System**.

As of Phase 1.5, MABS is a plugin-first, multiplayer-ready, data-driven ability framework for Unreal Engine. It keeps authored data, runtime state, gameplay execution, debug tooling, and editor-only code in separate modules so the runtime path stays clean and marketplace-oriented.

Current runtime ownership is:

* `MABSCore` for authored data and foundational runtime types
* `MABSGameplay` for granting, activation, validation, commit, and replication
* `MABSDebug` for runtime-safe debug helpers
* `MABSEditor` for editor-only extensions

## Why it exists

Most teams need the same ability building blocks:

* reusable data assets for designers
* a runtime owner component for characters and actors
* server-authoritative multiplayer behavior
* enough telemetry to understand request flow and failures
* a module layout that can grow without mixing editor and runtime code

MABS provides that foundation in a form that is easy to inspect, extend, and ship.

## What Phase 1.5 delivers

Phase 1.5 keeps the existing Phase 01 behavior and reorganizes it into plugin modules.

The preserved runtime behavior includes:

* granted ability storage on `UMABSAbilityComponent`
* replicated `FMABSAbilitySpec` runtime state
* `TryActivateAbilityByTag` with a client-to-server RPC path
* authority-side validation and lightweight commit
* structured debug events for request start, send, accept, reject, and commit

What changed in this phase is ownership and packaging:

* the ability types and definitions moved into `MABSCore`
* the component moved into `MABSGameplay`
* runtime-safe debug helpers now live in `MABSDebug`
* editor-only extension space now lives in `MABSEditor`
* Unreal redirects preserve existing asset references after the move

## Content ownership in the current project

The plugin is now the reusable runtime product.

The current host project still contains development harness content such as:

* `Config/DefaultGameplayTags.ini`
* `Content/Data/DA_Test_Fireball.uasset`
* Third Person Blueprint setup used to verify the flow

That content is useful for testing and examples, but it is not treated as core plugin runtime content in Phase 1.5.

## How to use it

1. Enable the local `MABS` plugin.
2. Create a `UMABSAbilityDefinition` asset.
3. Add `UMABSAbilityComponent` to the owning actor.
4. Grant the definition on the server.
5. Call `TryActivateAbilityByTag`.
6. Inspect `OnAbilityDebugEvent`, `GetRecentDebugEvents`, or `UMABSDebugBlueprintLibrary::FormatAbilityDebugEvent`.

## Example

A typical Phase 1.5 flow is:

1. Create a `MABSAbilityDefinition` asset with tag `Test.Ability.Fireball`.
2. Add `UMABSAbilityComponent` to the player character.
3. Grant the definition on the server during setup.
4. Bind input to `TryActivateAbilityByTag(Test.Ability.Fireball)`.
5. In standalone or as the listen server host, the server accepts and commits immediately.
6. As a remote client, the request is sent to the server and the owning client receives structured acceptance or rejection events.
