# MABS Quick Start

## What it is

This guide covers the current Phase 2 setup. At this stage, MABS is organized as a plugin with separate core, gameplay, debug, and editor modules while supporting a minimal instant gameplay result after activation.

## Why it exists

The goal of this guide is to get a new user from a blank actor or character to a verified multiplayer-safe ability that resolves a target and applies a simple instant effect in PIE.

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

The component lives in `MABSGameplay`. The owning actor should still be the authoritative source for grants, targeting, and effect application.

### Step 3: Create an ability definition asset

Create a `MABSAbilityDefinition` data asset and set:

* `AbilityTag`
* `DisplayName`
* `ActivationPolicy`
* `TargetType`
* `InstantEffectType`
* `EffectMagnitude`
* `TargetTraceDistance`

`TargetTraceDistance` is only used for simple `Actor` targeting. Cooldown and resource cost still remain placeholders in Phase 2.

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

When the server finishes execution, the owning client receives the authoritative debug events.

### Step 6: Inspect debug output

Inspect:

* `OnAbilityDebugEvent`
* `GetRecentDebugEvents()`
* `LogMABSAbilitySystem`
* `UMABSDebugBlueprintLibrary::FormatAbilityDebugEvent`

Useful event names in Phase 2:

* `RequestStarted`
* `RequestSentToServer`
* `RequestAccepted`
* `TargetResolved`
* `TargetResolutionFailed`
* `EffectApplied`
* `EffectApplicationFailed`
* `CommitSucceeded`

## Example

Example self-heal setup for the current Third Person host project harness:

1. Add `UMABSAbilityComponent`.
2. Create a `MABSAbilityDefinition` asset named `DA_Test_SelfHeal`.
3. Set `AbilityTag` to `Test.Ability.SelfHeal`.
4. Set `TargetType` to `Self`.
5. Set `InstantEffectType` to `Heal`.
6. Set `EffectMagnitude` to `25`.
7. On authoritative `BeginPlay`, call `GrantAbility(DA_Test_SelfHeal)`.
8. Bind an input action to call `TryActivateAbilityByTag(Test.Ability.SelfHeal)`.
9. Verify that the server resolves the owner, applies the heal through the example receiver, and emits `EffectApplied` and `CommitSucceeded`.

Example actor-damage setup:

1. Create a `MABSAbilityDefinition` asset named `DA_Test_Fireball`.
2. Set `AbilityTag` to `Test.Ability.Fireball`.
3. Set `TargetType` to `Actor`.
4. Set `InstantEffectType` to `Damage`.
5. Set `EffectMagnitude` to `20`.
6. Set `TargetTraceDistance` to a readable test distance such as `1500`.
7. Grant it on the server and call `TryActivateAbilityByTag(Test.Ability.Fireball)` while facing a target actor.

## Validation checklist

Singleplayer:

* project compiles
* the plugin loads
* `UMABSAbilityComponent` can be added to an actor
* `MABSAbilityDefinition` assets can be created
* self-heal resolves the owner and applies successfully
* actor damage succeeds when a valid actor is traced
* actor damage fails cleanly when no target actor is found

Listen server:

* the host can grant and activate self-heal and damage abilities
* a remote client still returns `RequestSentToServer` locally
* the server resolves targets and applies effects authoritatively
* the owning client receives the authoritative target/effect debug events

Dedicated server:

* `MABSServer.Target.cs` is included for dedicated server builds
* no editor-only plugin module is required by runtime code
* remote clients send activation requests through the server RPC path
* actual server build verification still requires a source-built Unreal Engine because Epic's installed binary distribution does not support server target compilation
