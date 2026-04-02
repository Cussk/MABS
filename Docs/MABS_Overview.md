# MABS Overview

## What MABS is

MABS stands for **Multiplayer Ability System**.

As of Phase 3, MABS is a plugin-first, multiplayer-ready, data-driven ability framework for Unreal Engine. It keeps authored data, runtime state, gameplay execution, debug tooling, and editor-only code in separate modules while supporting a small but inspectable end-to-end gameplay result:

* activate an ability
* validate cooldown and cost on authority
* resolve a target
* apply an instant effect
* spend cost and start cooldown on authority
* inspect the result through structured events and the runtime overlay

Current runtime ownership is:

* `MABSCore` for authored data and foundational runtime types
* `MABSGameplay` for granting, activation, cooldowns, costs, target resolution, instant effects, and replication
* `MABSDebug` for runtime-safe debug helpers and the HUD overlay
* `MABSEditor` for editor-only extensions

## Why it exists

Most teams need the same ability building blocks:

* reusable data assets for designers
* a runtime owner component for characters and actors
* server-authoritative multiplayer behavior
* enough telemetry to understand request flow, denial reasons, and failures
* a clean module layout that can grow without mixing editor and runtime code

MABS provides that foundation in a form that is easy to inspect, extend, and ship.

## What Phase 3 delivers

Phase 3 builds on the targeting/debug work from Phase 2.5 and adds the first real usage restrictions.

The current runtime behavior includes:

* granted ability storage on `UMABSAbilityComponent`
* replicated `FMABSAbilitySpec` runtime state
* replicated shared cooldown-group state
* `TryActivateAbilityByTag` with a client-to-server RPC path
* authority-side cooldown and cost validation
* target resolution for `Self` and `Actor`
* instant effects for `Damage` and `Heal`
* authoritative cost spending through `IMABSCostReceiver`
* authoritative cooldown start after successful commit
* structured debug events for request, cost, cooldown, target, effect, and commit steps

## Content ownership in the current project

The plugin is the reusable runtime product.

The current host project still contains development harness content such as:

* `Config/DefaultGameplayTags.ini`
* `Content/Data/DA_Test_Fireball.uasset`
* Third Person Blueprint setup used to verify the flow
* minimal example health and resource state on `AMABSCharacter`
* the default `AMABSGameMode` wiring for the debug HUD overlay

## How to use it

1. Enable the local `MABS` plugin.
2. Create a `UMABSAbilityDefinition` asset.
3. Set its targeting, effect, cooldown, and cost fields.
4. Add `UMABSAbilityComponent` to the owning actor.
5. Implement `IMABSCostReceiver` if the ability spends cost.
6. Grant the definition on the server.
7. Call `TryActivateAbilityByTag`.
8. Inspect the debug events, cooldown query helpers, and the debug HUD.
