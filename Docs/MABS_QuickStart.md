# MABS Quick Start

## What it is

This guide covers the current Phase 01 setup. At this stage, MABS supports real ability granting, activation requests by gameplay tag, server-authoritative approval, and structured debug output for the full request path.

## Why it exists

The goal of this guide is to get a new user from a blank actor or character to a verified activation request in PIE without needing to read the runtime code first.

## How to use it

### Step 1: Build the project

Compile the `MABS` module so the ability types are available in the editor.

Verify that the project opens without module load errors.

### Step 2: Add the ability component

Add `UMABSAbilityComponent` to the actor or character that should own abilities.

The owning actor should be the authoritative source for grants and activation, which usually means the server-owned pawn or character.

### Step 3: Create an ability definition asset

Create a `MABSAbilityDefinition` data asset and set:

* `AbilityTag`
* `DisplayName`
* `ActivationPolicy`
* `TargetType`
* `CooldownSeconds`
* `ResourceCost`

In Phase 01, cooldown and cost are still placeholders. Granting and activation flow are the focus.

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

Useful Phase 01 event names:

* `RequestStarted`
* `RequestSentToServer`
* `RequestAccepted`
* `RequestRejected`
* `CommitSucceeded`

## Example

Example setup for the Third Person template character:

1. Add `MABSAbilityComponent`.
2. Expose a `MABSAbilityDefinition` asset reference named `FireballAbility`.
3. On authoritative `BeginPlay`, call `GrantAbility(FireballAbility)`.
4. Bind `OnAbilityDebugEvent` to print the event name, tag, and result.
5. Bind an input action to call `TryActivateAbilityByTag` with `Ability.Test.Fireball`.
6. In multiplayer PIE, verify that remote clients see `RequestSentToServer` locally and then receive the authoritative result from the server.

## Validation checklist

Singleplayer:

* project compiles
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
* remote clients send activation requests through the server RPC path
* actual server build verification requires a source-built Unreal Engine because Epic's installed binary distribution does not support server target compilation
