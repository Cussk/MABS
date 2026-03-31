# MABS API

## Purpose of this document

This document lists the current Phase 00 public surface for MABS and explains which type owns which responsibility.

## Enums

### `EMABSAbilityActivationResult`

Describes the outcome of an activation request. Phase 00 uses it for both activation rejections and the current `NotImplemented` stub result.

### `EMABSAbilityRuntimeState`

Represents the runtime state of a granted ability entry.

### `EMABSTargetType`

Represents the authored target intent for an ability definition.

## Structs

### `FMABSAbilityHandle`

A lightweight runtime identifier assigned when an ability is granted.

### `FMABSAbilitySpec`

The runtime representation of a granted ability. It stores the handle, definition reference, and runtime state.

### `FMABSAbilityDebugEvent`

A structured debug payload emitted by the ability component. It records event name, owner, tag, handle, runtime state, result, and a plain-language message.

## Classes

### `UMABSAbilityDefinition`

A `UDataAsset` that stores authored ability data.

Current authored fields:

* `AbilityTag`
* `TargetType`
* `CooldownSeconds`
* `ResourceCost`

### `UMABSAbilityComponent`

A `UActorComponent` that owns runtime ability state for an actor.

Current public functions:

* `GrantAbility`
* `TryActivateAbilityByTag`
* `GetGrantedAbilities`
* `GetRecentDebugEvents`

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
