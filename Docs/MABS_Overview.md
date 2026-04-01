# MABS Overview

## What MABS is

MABS stands for **Multiplayer Ability System**.

As of Phase 2.5, MABS is a plugin-first, multiplayer-ready, data-driven ability framework for Unreal Engine. It keeps authored data, runtime state, gameplay execution, debug tooling, and editor-only code in separate modules while supporting a small but inspectable end-to-end gameplay result:

* activate an ability
* resolve a target on authority
* apply an instant effect
* inspect the result through structured events, trace snapshots, and a runtime overlay

Current runtime ownership is:

* `MABSCore` for authored data and foundational runtime types
* `MABSGameplay` for granting, activation, target resolution, target validation, instant effects, and replication
* `MABSDebug` for runtime-safe debug helpers and the first HUD overlay
* `MABSEditor` for editor-only extensions

## Why it exists

Most teams need the same ability building blocks:

* reusable data assets for designers
* a runtime owner component for characters and actors
* server-authoritative multiplayer behavior
* enough telemetry to understand request flow, targeting, and failures
* a clean module layout that can grow without mixing editor and runtime code

MABS provides that foundation in a form that is easy to inspect, extend, and ship.

## What Phase 2.5 delivers

Phase 2.5 builds on the Phase 2 effect pipeline and improves usability and runtime visibility.

The current runtime behavior includes:

* granted ability storage on `UMABSAbilityComponent`
* replicated `FMABSAbilitySpec` runtime state
* `TryActivateAbilityByTag` with a client-to-server RPC path
* authority-side validation and request acceptance
* target resolution for `Self` and `Actor`
* configurable actor trace mode with line and sphere support
* centralized actor-target validation
* instant effects for `Damage` and `Heal`
* structured debug events for request, target trace, target result, effect, and commit steps
* a latest-trace snapshot for runtime inspection
* a first lightweight `AMABSDebugHUD` overlay

## Content ownership in the current project

The plugin is the reusable runtime product.

The current host project still contains development harness content such as:

* `Config/DefaultGameplayTags.ini`
* `Content/Data/DA_Test_Fireball.uasset`
* Third Person Blueprint setup used to verify the flow
* minimal example health state on `AMABSCharacter` for self-heal testing
* the default `AMABSGameMode` wiring for the debug HUD overlay

That content is useful for testing and examples, but it is not treated as core plugin runtime content.

## How to use it

1. Enable the local `MABS` plugin.
2. Create a `UMABSAbilityDefinition` asset.
3. Set its `TargetType`, `InstantEffectType`, `EffectMagnitude`, and Phase 2.5 actor-target trace options when needed.
4. Add `UMABSAbilityComponent` to the owning actor.
5. Grant the definition on the server.
6. Call `TryActivateAbilityByTag`.
7. Inspect `OnAbilityDebugEvent`, `GetRecentDebugEvents`, `GetLatestTargetTraceDebugInfo`, or the debug HUD.

## Example

A typical Phase 2.5 actor-target flow is:

1. Create a `MABSAbilityDefinition` asset with tag `Test.Ability.Fireball`.
2. Set `TargetType` to `Actor`.
3. Set `InstantEffectType` to `Damage`.
4. Set `EffectMagnitude` to `20`.
5. Set `ActorTargetTraceMode` to `Sphere` and `TargetTraceRadius` to `50`.
6. Add `UMABSAbilityComponent` to the player character and grant the ability on the server.
7. Call `TryActivateAbilityByTag(Test.Ability.Fireball)`.
8. The authority path runs the target trace, validates the hit actor, stores the latest trace snapshot, applies damage, emits target and effect events, and then returns the ability to idle.
