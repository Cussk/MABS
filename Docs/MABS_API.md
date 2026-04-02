# MABS API

## What it is

This document lists the current Phase 3 public surface for MABS and shows which module owns authored data, runtime state, cooldown data, cost integration, execution, and debug helpers.

## Why it exists

Phase 3 keeps the multiplayer-safe activation flow from earlier phases, then adds cooldown execution, cooldown groups, and resource cost spending. Users need a clear map of the new authored fields, runtime structs, interfaces, and query helpers.

## Module ownership

### `MABSCore`

Owns foundational runtime data:

* `EMABSAbilityActivationResult`
* `EMABSAbilityRuntimeState`
* `EMABSTargetType`
* `EMABSTargetTraceMode`
* `EMABSAbilityActivationPolicy`
* `EMABSInstantEffectType`
* `FMABSAbilityHandle`
* `FMABSAbilitySpec`
* `FMABSCooldownGroupState`
* `FMABSAbilityDebugEvent`
* `FMABSTargetTraceDebugInfo`
* `UMABSAbilityDefinition`
* `LogMABSAbilitySystem`

### `MABSGameplay`

Owns gameplay execution:

* `UMABSAbilityComponent`
* `IMABSInstantEffectReceiver`
* `IMABSCostReceiver`

### `MABSDebug`

Owns runtime-safe presentation helpers:

* `UMABSDebugBlueprintLibrary`
* `AMABSDebugHUD`

## Enums

### `EMABSAbilityActivationResult`

Describes the result of an activation request or the last known result stored on a granted ability spec.

Current values:

* `None`
* `Success`
* `RequestSentToServer`
* `InvalidAbility`
* `NotGranted`
* `AlreadyActive`
* `Blocked`
* `OnCooldown`
* `InsufficientResources`
* `AuthorityRejected`
* `TargetResolutionFailed`
* `EffectApplicationFailed`

### `EMABSAbilityRuntimeState`

Represents the live runtime state of a granted ability entry.

Current values:

* `None`
* `Idle`
* `Active`
* `Blocked`

### `EMABSTargetType`

Represents the authored target intent for an ability definition.

Current supported values:

* `Self`
* `Actor`

### `EMABSTargetTraceMode`

Represents the authored trace shape for `Actor` targeting.

Current values:

* `Line`
* `Sphere`

### `EMABSAbilityActivationPolicy`

Represents the authored activation intent for an ability definition.

Current values:

* `OnInputTriggered`
* `Passive`

### `EMABSInstantEffectType`

Represents the authored instant effect intent for an ability definition.

Current values:

* `None`
* `Damage`
* `Heal`

## Structs

### `FMABSAbilityHandle`

A lightweight runtime identifier assigned when an ability is granted.

### `FMABSAbilitySpec`

The runtime representation of one granted ability.

Current fields:

* `Handle`
* `AbilityDefinition`
* `AbilityTag`
* `RuntimeState`
* `LastActivationResult`
* `CooldownEndTime`

### `FMABSCooldownGroupState`

Represents one shared cooldown-group runtime entry on the owning component.

Current fields:

* `CooldownGroupTag`
* `CooldownEndTime`

### `FMABSAbilityDebugEvent`

A structured debug payload emitted by the ability component. It records the event name, owner, tag, handle, runtime state, activation result, timestamp, and plain-language message.

### `FMABSTargetTraceDebugInfo`

A structured snapshot of the latest actor-target trace for the owning actor.

## Classes

### `UMABSAbilityDefinition`

A `UDataAsset` that stores authored ability data.

Current authored fields:

* `AbilityTag`
* `DisplayName`
* `ActivationPolicy`
* `TargetType`
* `InstantEffectType`
* `EffectMagnitude`
* `TargetTraceDistance`
* `ActorTargetTraceMode`
* `TargetTraceRadius`
* `bRequireValidActorTarget`
* `bIgnoreNonTargetWorldHits`
* `bDrawTargetTraceDebug`
* `TargetTraceDebugDuration`
* `CooldownSeconds`
* `CooldownGroupTag`
* `ResourceCost`

### `UMABSAbilityComponent`

A `UActorComponent` that owns granted ability specs, routes activation requests, validates cooldown and cost rules, resolves targets, applies instant effects, emits debug events, and stores shared cooldown-group state.

Current public functions:

* `GrantAbility(UMABSAbilityDefinition* AbilityDefinition)`
* `SetAbilityBlockedByTag(FGameplayTag AbilityTag, bool bBlocked)`
* `TryActivateAbilityByTag(FGameplayTag AbilityTag)`
* `GetGrantedAbilities()`
* `FindGrantedAbilitySpecByTag(FGameplayTag AbilityTag, FMABSAbilitySpec& OutAbilitySpec)`
* `GetCooldownRemainingByTag(FGameplayTag AbilityTag)`
* `IsAbilityOnCooldown(FGameplayTag AbilityTag)`
* `GetCooldownGroupRemaining(FGameplayTag CooldownGroupTag)`
* `IsCooldownGroupActive(FGameplayTag CooldownGroupTag)`
* `GetRecentDebugEvents()`
* `GetLatestTargetTraceDebugInfo()`

Current replicated state:

* `GrantedAbilities`
* `CooldownGroupStates`

Important new debug event names:

* `CooldownRejected`
* `CooldownStarted`
* `CostValidated`
* `CostRejected`
* `CostSpent`

### `IMABSInstantEffectReceiver`

A minimal optional extension hook for receiving MABS heal effects.

Current public function:

* `ApplyMABSHeal(float HealAmount, AActor* SourceActor, UMABSAbilityDefinition* SourceAbility)`

### `IMABSCostReceiver`

A minimal optional extension hook for validating and spending resource cost.

Current public functions:

* `CanAffordMABSCost(float Cost, UMABSAbilityDefinition* SourceAbility)`
* `SpendMABSCost(float Cost, UMABSAbilityDefinition* SourceAbility)`

### `UMABSDebugBlueprintLibrary`

A runtime-safe helper library for debug consumers.

Current public functions:

* `FormatAbilityDebugEvent(const FMABSAbilityDebugEvent& DebugEvent)`
* `FormatTargetTraceDebugInfo(const FMABSTargetTraceDebugInfo& DebugInfo)`
* `FormatAbilitySpecRuntimeSummary(const FMABSAbilitySpec& AbilitySpec, float CurrentWorldTimeSeconds)`
* `GetAbilityDebugEventColor(const FMABSAbilityDebugEvent& DebugEvent)`
* `GetTargetTraceDebugColor(const FMABSTargetTraceDebugInfo& DebugInfo)`
* `GetAbilitySpecRuntimeColor(const FMABSAbilitySpec& AbilitySpec, float CurrentWorldTimeSeconds)`

### `AMABSDebugHUD`

A lightweight runtime HUD overlay that reads the local owner’s `UMABSAbilityComponent` and displays:

* the latest target trace snapshot
* granted ability runtime summaries
* a recent debug event list

## How to use it

1. Enable the `MABS` plugin.
2. Create a `UMABSAbilityDefinition` asset with a valid `AbilityTag`.
3. Set `TargetType`, `InstantEffectType`, `EffectMagnitude`, and the Phase 3 cooldown and cost options.
4. Add `UMABSAbilityComponent` to the owning actor or character.
5. Implement `IMABSCostReceiver` on the owner if `ResourceCost > 0`.
6. Grant the definition on the authoritative owner with `GrantAbility`.
7. Call `TryActivateAbilityByTag` from input or gameplay code.
8. Inspect cooldown query helpers, recent debug events, and the overlay.

## Example

Example C++ flow:

```cpp
if (AbilityComponent != nullptr && HasAuthority())
{
	AbilityComponent->GrantAbility(SelfHealDefinition);
}

const EMABSAbilityActivationResult Result =
	AbilityComponent->TryActivateAbilityByTag(SelfHealTag);

const float CooldownRemaining =
	AbilityComponent->GetCooldownRemainingByTag(SelfHealTag);
```

In standalone or on the listen server host, a valid granted ability can now be denied by cooldown or cost before target/effect execution. After a successful commit, the authority path spends cost, starts cooldown, updates replicated runtime state, and emits the matching debug events. On a remote client, the local call still returns `RequestSentToServer`, then the owning client receives the authoritative denial or success information from the server.
