# MABS Quick Start

## What it is

This guide covers the current Phase 3 setup. MABS now supports the Phase 2.5 targeting/debug flow plus real cooldown execution, cooldown groups, and simple authoritative resource costs.

## Why it exists

The goal of this guide is to get a new user from a blank actor or character to a verified multiplayer-safe ability that:

* resolves a target on authority
* applies an instant effect
* spends cost only on authority
* starts cooldown only on authority
* explains success or denial through logs, structured events, and the runtime overlay

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

The component lives in `MABSGameplay`. It owns granted ability specs, shared cooldown-group state, authoritative activation, target resolution, effect application, and runtime debug state.

### Step 3: Create an ability definition asset

Create a `MABSAbilityDefinition` data asset and set the usual gameplay fields:

* `AbilityTag`
* `DisplayName`
* `ActivationPolicy`
* `TargetType`
* `InstantEffectType`
* `EffectMagnitude`

For Phase 3 usage restrictions, also configure:

* `CooldownSeconds`
* `CooldownGroupTag`
* `ResourceCost`

For `Actor` targeting, you can still configure:

* `TargetTraceDistance`
* `ActorTargetTraceMode`
* `TargetTraceRadius`
* `bRequireValidActorTarget`
* `bIgnoreNonTargetWorldHits`
* `bDrawTargetTraceDebug`
* `TargetTraceDebugDuration`

### Step 4: Implement cost spending if needed

If `ResourceCost > 0`, implement `IMABSCostReceiver` on the owning actor.

The current host-project `AMABSCharacter` already provides a minimal example resource pool through:

* `CanAffordMABSCost`
* `SpendMABSCost`

### Step 5: Grant an ability on the server

Call `GrantAbility` on the component with the definition asset.

This creates a runtime `FMABSAbilitySpec` with:

* a stable `FMABSAbilityHandle`
* the granted definition reference
* a copied `AbilityTag`
* runtime state
* the last activation result
* the cooldown end time

### Step 6: Request activation by gameplay tag

Call `TryActivateAbilityByTag` with the definition tag.

Behavior depends on where the call starts:

* standalone or server authority: validates cooldown/cost, resolves target, applies effect, spends cost, starts cooldown, and commits directly
* remote client: emits `RequestStarted`, sends an RPC, and returns `RequestSentToServer`

### Step 7: Inspect debug output

Inspect:

* `OnAbilityDebugEvent`
* `GetRecentDebugEvents()`
* `GetLatestTargetTraceDebugInfo()`
* `GetCooldownRemainingByTag(...)`
* `GetCooldownGroupRemaining(...)`
* `UMABSDebugBlueprintLibrary`

Useful Phase 3 event names:

* `CooldownRejected`
* `CooldownStarted`
* `CostValidated`
* `CostRejected`
* `CostSpent`
* `TargetResolved`
* `EffectApplied`
* `CommitSucceeded`

### Step 8: Use the runtime overlay

The default Third Person host harness still uses `AMABSDebugHUD` through `AMABSGameMode`.

What it shows:

* the latest actor-target trace snapshot for the local owner
* granted ability summaries with cooldown remaining
* a recent event list

## Example

Example self-heal setup:

1. Add `UMABSAbilityComponent`.
2. Create `DA_Test_SelfHeal`.
3. Set `AbilityTag` to `Test.Ability.SelfHeal`.
4. Set `TargetType` to `Self`.
5. Set `InstantEffectType` to `Heal`.
6. Set `EffectMagnitude` to `25`.
7. Set `CooldownSeconds` to `5`.
8. Set `CooldownGroupTag` to `Test.CooldownGroup.Support`.
9. Set `ResourceCost` to `20`.
10. On authoritative `BeginPlay`, call `GrantAbility(DA_Test_SelfHeal)`.
11. Bind input to call `TryActivateAbilityByTag(Test.Ability.SelfHeal)`.
12. Verify the first activation succeeds, the second immediate activation is denied by cooldown, and later activation is denied if the example resource pool is too low.

Example actor-damage setup:

1. Create `DA_Test_Fireball`.
2. Set `AbilityTag` to `Test.Ability.Fireball`.
3. Set `TargetType` to `Actor`.
4. Set `InstantEffectType` to `Damage`.
5. Set `EffectMagnitude` to `20`.
6. Set `CooldownSeconds` to `3`.
7. Set `CooldownGroupTag` to `Test.CooldownGroup.Offense`.
8. Set `ResourceCost` to `15`.
9. Set `ActorTargetTraceMode` to `Sphere` and `TargetTraceRadius` to `50`.
10. Grant it on the server and call `TryActivateAbilityByTag(Test.Ability.Fireball)` while facing a target actor.

## Validation checklist

Singleplayer:

* self-heal starts cooldown after success
* self-heal cannot be reactivated until cooldown expires
* fireball starts cooldown after success
* cooldown denial produces readable debug output
* cost denial produces readable debug output
* cost is not spent on failed target resolution
* cooldown does not start on failed target resolution

Listen server:

* the host sees cooldown and cost behavior directly on authority
* a remote client still returns `RequestSentToServer` locally
* the server resolves targets, spends cost, and starts cooldown authoritatively
* the owning client receives authoritative cooldown/cost events

Dedicated server:

* `MABSServer.Target.cs` is included for dedicated server builds
* no editor-only plugin module is required by runtime code
* remote clients send activation requests through the server RPC path
* authoritative denial and success reasons still reach the owning client
