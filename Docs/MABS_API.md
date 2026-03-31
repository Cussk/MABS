# MABS API

## What it is

This document lists the current Phase 01 public surface for MABS and shows which type owns authored data, runtime state, execution, and debug output.

## Why it exists

Phase 01 is the first end-to-end gameplay slice. A user can now grant an authored ability, request activation by gameplay tag, route client requests to the server, and inspect structured success or failure results.

## Enums

### `EMABSAbilityActivationResult`

Describes the current result of an activation request or the last known result stored on a granted ability spec.

Current values:

* `None`
* `Success`
* `RequestSentToServer`
* `InvalidAbility`
* `NotGranted`
* `AlreadyActive`
* `Blocked`
* `AuthorityRejected`

### `EMABSAbilityRuntimeState`

Represents the live runtime state of a granted ability entry.

Current values:

* `None`
* `Idle`
* `Active`
* `Blocked`

### `EMABSTargetType`

Represents the authored target intent for an ability definition.

### `EMABSAbilityActivationPolicy`

Represents the authored activation intent for an ability definition.

Current values:

* `OnInputTriggered`
* `Passive`

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

### `FMABSAbilityDebugEvent`

A structured debug payload emitted by the ability component. It records the event name, owner, tag, handle, runtime state, activation result, timestamp, and plain-language message.

## Classes

### `UMABSAbilityDefinition`

A `UDataAsset` that stores authored ability data.

Current authored fields:

* `AbilityTag`
* `DisplayName`
* `ActivationPolicy`
* `TargetType`
* `CooldownSeconds`
* `ResourceCost`

### `UMABSAbilityComponent`

A `UActorComponent` that owns granted ability specs, routes activation requests, and emits debug events.

Current public functions:

* `GrantAbility(UMABSAbilityDefinition* AbilityDefinition)`
* `SetAbilityBlockedByTag(FGameplayTag AbilityTag, bool bBlocked)`
* `TryActivateAbilityByTag(FGameplayTag AbilityTag)`
* `GetGrantedAbilities()`
* `FindGrantedAbilitySpecByTag(FGameplayTag AbilityTag, FMABSAbilitySpec& OutAbilitySpec)`
* `GetRecentDebugEvents()`

Current replicated state:

* `GrantedAbilities`

Current public events:

* `OnAbilityDebugEvent`

## Responsibility split

Data:

* `UMABSAbilityDefinition`

Runtime state:

* `FMABSAbilityHandle`
* `FMABSAbilitySpec`

Execution:

* `UMABSAbilityComponent`

Observability:

* `FMABSAbilityDebugEvent`
* `LogMABSAbilitySystem`

## How to use it

1. Create a `UMABSAbilityDefinition` asset with a valid `AbilityTag`.
2. Add `UMABSAbilityComponent` to the owning actor or character.
3. Grant the definition on the authoritative owner with `GrantAbility`.
4. Call `TryActivateAbilityByTag` from input or gameplay code.
5. Inspect `OnAbilityDebugEvent`, `GetRecentDebugEvents`, or the replicated `FMABSAbilitySpec` state for the outcome.

## Example

Example C++ flow:

```cpp
if (AbilityComponent != nullptr && HasAuthority())
{
	AbilityComponent->GrantAbility(FireballDefinition);
}

const EMABSAbilityActivationResult Result =
	AbilityComponent->TryActivateAbilityByTag(FireballTag);
```

In standalone or on the listen server host, a valid granted ability returns `Success`. On a remote client, the local call returns `RequestSentToServer`, then the owning client receives `RequestAccepted`, `RequestRejected`, and `CommitSucceeded` debug events from the server-authoritative path.
