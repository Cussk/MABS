# MABS Quick Start

## What it is

This guide covers the current Phase 1.5 setup. At this stage, MABS is organized as a plugin with separate core, gameplay, debug, and editor modules while keeping the Phase 01 grant-and-activate flow unchanged.

## Why it exists

The goal of this guide is to get a new user from a blank actor or character to a verified multiplayer-safe activation request in PIE without needing to read the runtime code first.

## How to use it

### Step 1: Build the project with the plugin enabled

Compile the project with the local `MABS` plugin enabled.

Verify that the editor loads these modules without startup errors:

* `MABSCore`
* `MABSGameplay`
* `MABSDebug`
* `MABSEditor`

### Step 2: Add the ability component

Add `UMABSAbilityComponent` to the actor or character that should own abilities.

The component lives in `MABSGameplay`. The owning actor should still be the authoritative source for grants and activation, which usually means the server-owned pawn or character.

### Step 3: Create an ability definition asset

Create a `MABSAbilityDefinition` data asset and set:

* `AbilityTag`
* `DisplayName`
* `ActivationPolicy`
* `TargetType`
* `CooldownSeconds`
* `ResourceCost`

The definition type lives in `MABSCore`. In Phase 1.5, cooldown and cost are still placeholders. Granting and activation flow are the focus.

### Step 4: Grant an ability on the server

Call `GrantAbility` on the component with the definition asset.

This creates a runtime `FMABSAbilitySpec` with:

* a stable `FMABSAbilityHandle`
* the granted definition reference
* a copied `AbilityTag`
* runtime state
* the last activation result

Granting must run on authority. Invalid definitions and duplicate grants are rejected and logged.

### Step 5: Request activation by gameplay tag

Call `TryActivateAbilityByTag` with the definition tag.

Behavior depends on where the call starts:

* standalone or server authority: validates and commits directly
* remote client: emits `RequestStarted`, sends an RPC, and returns `RequestSentToServer`

When the server finishes validation, the owning client receives the authoritative accept or reject debug events.

### Step 6: Inspect debug output

Inspect:

* `OnAbilityDebugEvent`
* `GetRecentDebugEvents()`
* `LogMABSAbilitySystem`
* `UMABSDebugBlueprintLibrary::FormatAbilityDebugEvent`

Useful event names:

* `RequestStarted`
* `RequestSentToServer`
* `RequestAccepted`
* `RequestRejected`
* `CommitSucceeded`

## Example

Example setup for the current Third Person host project harness:

1. Add `UMABSAbilityComponent`.
2. Expose a `MABSAbilityDefinition` asset reference named `FireballAbility`.
3. On authoritative `BeginPlay`, call `GrantAbility(FireballAbility)`.
4. Bind `OnAbilityDebugEvent` to print the event name, tag, and result.
5. Bind an input action to call `TryActivateAbilityByTag` with `Test.Ability.Fireball`.
6. In multiplayer PIE, verify that remote clients see `RequestSentToServer` locally and then receive the authoritative result from the server.

## Validation checklist

Singleplayer:

* project compiles
* the plugin loads
* `UMABSAbilityComponent` can be added to an actor
* `MABSAbilityDefinition` assets can be created
* `GrantAbility` creates a runtime spec
* `TryActivateAbilityByTag` succeeds for a granted ability
* invalid or ungranted tags fail cleanly

Listen server:

* the host can grant and activate a granted ability
* a remote client returns `RequestSentToServer` locally
* the server logs `RequestAccepted` or `RequestRejected`
* the owning client receives the authoritative debug result

Dedicated server:

* `MABSServer.Target.cs` is included for dedicated server builds
* no editor-only plugin module is required by runtime code
* remote clients send activation requests through the server RPC path
* actual server build verification still requires a source-built Unreal Engine because Epic's installed binary distribution does not support server target compilation
