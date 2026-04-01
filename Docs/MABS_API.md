# MABS API

## What it is

This document lists the current Phase 1.5 public surface for MABS and shows which module owns authored data, runtime state, execution, and debug helpers.

## Why it exists

Phase 1.5 keeps the existing Phase 01 gameplay slice, but it moves that slice into a plugin-first architecture. Users still need a clear view of what type to use and which module now owns it.

## Module ownership

### `MABSCore`

Owns foundational runtime data:

* `EMABSAbilityActivationResult`
* `EMABSAbilityRuntimeState`
* `EMABSTargetType`
* `EMABSAbilityActivationPolicy`
* `FMABSAbilityHandle`
* `FMABSAbilitySpec`
* `FMABSAbilityDebugEvent`
* `UMABSAbilityDefinition`
* `LogMABSAbilitySystem`

### `MABSGameplay`

Owns gameplay execution:

* `UMABSAbilityComponent`

### `MABSDebug`

Owns runtime-safe debug helpers:

* `UMABSDebugBlueprintLibrary`

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

### `UMABSDebugBlueprintLibrary`

A runtime-safe helper library for debug consumers.

Current public functions:

* `FormatAbilityDebugEvent(const FMABSAbilityDebugEvent& DebugEvent)`

## How to use it

1. Enable the `MABS` plugin.
2. Create a `UMABSAbilityDefinition` asset with a valid `AbilityTag`.
3. Add `UMABSAbilityComponent` to the owning actor or character.
4. Grant the definition on the authoritative owner with `GrantAbility`.
5. Call `TryActivateAbilityByTag` from input or gameplay code.
6. Inspect `OnAbilityDebugEvent`, `GetRecentDebugEvents`, or formatted debug strings for the outcome.

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
