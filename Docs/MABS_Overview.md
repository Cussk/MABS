# MABS Overview

## What MABS is

MABS stands for **Multiplayer Ability System**.

As of Phase 2, MABS is a plugin-first, multiplayer-ready, data-driven ability framework for Unreal Engine. It keeps authored data, runtime state, gameplay execution, debug tooling, and editor-only code in separate modules while supporting a minimal end-to-end gameplay result:

* activate an ability
* resolve a simple target
* apply an instant effect on authority

Current runtime ownership is:

* `MABSCore` for authored data and foundational runtime types
* `MABSGameplay` for granting, activation, target resolution, instant effects, validation, and replication
* `MABSDebug` for runtime-safe debug helpers
* `MABSEditor` for editor-only extensions

## Why it exists

Most teams need the same ability building blocks:

* reusable data assets for designers
* a runtime owner component for characters and actors
* server-authoritative multiplayer behavior
* enough telemetry to understand request flow and failures
* a clean module layout that can grow without mixing editor and runtime code

MABS provides that foundation in a form that is easy to inspect, extend, and ship.

## What Phase 2 delivers

Phase 2 builds on the Phase 01 and 1.5 flow and adds the first real gameplay outcome layer.

The current runtime behavior includes:

* granted ability storage on `UMABSAbilityComponent`
* replicated `FMABSAbilitySpec` runtime state
* `TryActivateAbilityByTag` with a client-to-server RPC path
* authority-side validation and request acceptance
* target resolution for `Self` and `Actor`
* instant effects for `Damage` and `Heal`
* structured debug events for request, target, effect, and commit steps

## Content ownership in the current project

The plugin is the reusable runtime product.

The current host project still contains development harness content such as:

* `Config/DefaultGameplayTags.ini`
* `Content/Data/DA_Test_Fireball.uasset`
* Third Person Blueprint setup used to verify the flow
* minimal example health state on `AMABSCharacter` for self-heal testing

That content is useful for testing and examples, but it is not treated as core plugin runtime content.

## How to use it

1. Enable the local `MABS` plugin.
2. Create a `UMABSAbilityDefinition` asset.
3. Set its `TargetType`, `InstantEffectType`, `EffectMagnitude`, and optional `TargetTraceDistance`.
4. Add `UMABSAbilityComponent` to the owning actor.
5. Grant the definition on the server.
6. Call `TryActivateAbilityByTag`.
7. Inspect `OnAbilityDebugEvent`, `GetRecentDebugEvents`, or `UMABSDebugBlueprintLibrary::FormatAbilityDebugEvent`.

## Example

A typical Phase 2 flow is:

1. Create a `MABSAbilityDefinition` asset with tag `Test.Ability.SelfHeal`.
2. Set `TargetType` to `Self`.
3. Set `InstantEffectType` to `Heal`.
4. Set `EffectMagnitude` to `25`.
5. Add `UMABSAbilityComponent` to the player character and grant the ability on the server.
6. Call `TryActivateAbilityByTag(Test.Ability.SelfHeal)`.
7. The authority path resolves the owner as the target, applies the heal, emits `TargetResolved`, `EffectApplied`, and `CommitSucceeded`, then returns the ability to idle.
