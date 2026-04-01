# MABS Quick Start

## What it is

This guide covers the current Phase 2.5 setup. MABS now supports the Phase 2 ability flow plus actor-targeting polish, richer target trace events, in-world trace debug drawing, and a first runtime debug overlay.

## Why it exists

The goal of this guide is to get a new user from a blank actor or character to a verified multiplayer-safe ability that:

* resolves a target on authority
* applies an instant effect
* explains the result through logs, structured events, and on-screen runtime visibility

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

The component lives in `MABSGameplay`. It owns granted ability specs, authoritative activation, target resolution, effect application, and runtime debug state.

### Step 3: Create an ability definition asset

Create a `MABSAbilityDefinition` data asset and set the usual gameplay fields:

* `AbilityTag`
* `DisplayName`
* `ActivationPolicy`
* `TargetType`
* `InstantEffectType`
* `EffectMagnitude`

For `Actor` targeting, Phase 2.5 also adds these authored options:

* `TargetTraceDistance`
* `ActorTargetTraceMode`
* `TargetTraceRadius`
* `bRequireValidActorTarget`
* `bIgnoreNonTargetWorldHits`
* `bDrawTargetTraceDebug`
* `TargetTraceDebugDuration`

Recommended Phase 2.5 defaults for moment-to-moment testing:

* `ActorTargetTraceMode = Sphere`
* `TargetTraceRadius = 50`
* `bRequireValidActorTarget = true`
* `bIgnoreNonTargetWorldHits = true`

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

* standalone or server authority: validates, resolves target, applies effect, and commits directly
* remote client: emits `RequestStarted`, sends an RPC, and returns `RequestSentToServer`

When the server finishes execution, the owning client receives the authoritative debug events and the latest authoritative target trace snapshot.

### Step 6: Inspect debug output

Inspect:

* `OnAbilityDebugEvent`
* `GetRecentDebugEvents()`
* `GetLatestTargetTraceDebugInfo()`
* `LogMABSAbilitySystem`
* `UMABSDebugBlueprintLibrary::FormatAbilityDebugEvent`
* `UMABSDebugBlueprintLibrary::FormatTargetTraceDebugInfo`

Useful Phase 2.5 event names:

* `RequestStarted`
* `RequestSentToServer`
* `RequestAccepted`
* `TargetTraceStarted`
* `TargetTraceHit`
* `TargetTraceRejected`
* `TargetResolved`
* `TargetResolutionFailed`
* `EffectApplied`
* `EffectApplicationFailed`
* `CommitSucceeded`

### Step 7: Use the runtime overlay

The default Third Person host harness now uses `AMABSDebugHUD` through `AMABSGameMode`.

What it shows:

* the latest actor-target trace snapshot for the local owner
* a recent event list
* color-coded success, in-progress, and failure text

You can toggle it at runtime by calling `ToggleOverlayEnabled` or `SetOverlayEnabled` on the active HUD.

## Example

Example self-heal setup:

1. Add `UMABSAbilityComponent`.
2. Create `DA_Test_SelfHeal`.
3. Set `AbilityTag` to `Test.Ability.SelfHeal`.
4. Set `TargetType` to `Self`.
5. Set `InstantEffectType` to `Heal`.
6. Set `EffectMagnitude` to `25`.
7. On authoritative `BeginPlay`, call `GrantAbility(DA_Test_SelfHeal)`.
8. Bind input to call `TryActivateAbilityByTag(Test.Ability.SelfHeal)`.
9. Verify `TargetResolved`, `EffectApplied`, and `CommitSucceeded`.

Example actor-damage setup:

1. Create `DA_Test_Fireball`.
2. Set `AbilityTag` to `Test.Ability.Fireball`.
3. Set `TargetType` to `Actor`.
4. Set `InstantEffectType` to `Damage`.
5. Set `EffectMagnitude` to `20`.
6. Set `TargetTraceDistance` to `1500`.
7. Set `ActorTargetTraceMode` to `Sphere`.
8. Set `TargetTraceRadius` to `50`.
9. Enable `bIgnoreNonTargetWorldHits` and `bDrawTargetTraceDebug`.
10. Grant it on the server and call `TryActivateAbilityByTag(Test.Ability.Fireball)` while facing a target actor.
11. Verify that the overlay shows the latest trace result and the world shows the debug line and hit marker.

## Validation checklist

Singleplayer:

* project compiles
* the plugin loads
* `UMABSAbilityComponent` can be added to an actor
* `MABSAbilityDefinition` assets can be created
* self-heal resolves the owner and applies successfully
* actor damage resolves a valid nearby character more reliably than the old Phase 2 line trace
* actor damage rejects invalid world hits cleanly
* the trace debug draw appears when enabled
* the overlay shows recent target and effect events

Listen server:

* the host can grant and activate self-heal and damage abilities
* a remote client still returns `RequestSentToServer` locally
* the server resolves targets and applies effects authoritatively
* the owning client receives authoritative target/effect events and the latest trace snapshot

Dedicated server:

* `MABSServer.Target.cs` is included for dedicated server builds
* no editor-only plugin module is required by runtime code
* remote clients send activation requests through the server RPC path
* the runtime overlay remains client-safe because it reads local replicated/debug state rather than editor-only systems
