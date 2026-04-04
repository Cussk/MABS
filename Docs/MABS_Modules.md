# MABS Modules

## What it is

This document explains the Phase 11 module layout for MABS.

Phase 11 keeps the existing module ownership and runtime split, then adds a public delivery-handler seam inside `MABSGameplay` while preserving state ownership on `UMABSAbilityComponent`.

## Module map

### `MABSCore`

Owns:

* `UMABSAbilityDefinition`
* `UMABSAbilitySet`
* authored delivery-handler class reference on `UMABSAbilityDefinition`
* ability runtime structs and enums
* authored timing fields
* authored socket fields
* authored montage references
* authored presentation structs and fields
* authored combo data
* authored AoE data
* authored periodic effect data
* `FMABSAbilitySpec`
* `FMABSActivePeriodicEffect`
* shared debug event types
* shared debug event categories
* shared harness summary structs

### `MABSGameplay`

Owns:

* `UMABSAbilityComponent`
* `AMABSProjectileBase`
* `FMABSDeliveryExecutionContext`
* `FMABSDeliveryExecutionResult`
* `UMABSDeliveryHandler`
* built-in direct / hit trace / melee / projectile delivery handlers
* granting runtime
* activation and combo runtime
* delivery and targeting runtime
* effects, cooldown, cost, and periodic runtime
* presentation routing runtime
* debug summary and event assembly runtime
* replicated gameplay state

### `MABSDebug`

Owns:

* runtime-safe debug formatting
* the runtime harness HUD
* category toggles
* recent-event presentation
* harness section layout and filtering

### `MABSEditor`

Owns:

* editor-only extensions
* future validation and authoring helpers

## `MABSGameplay` internal split

Phase 11 keeps `UMABSAbilityComponent` as the public facade, and the private runtime now lives in:

* `MABSAbilityRuntime_EventNames.h`
* `MABSAbilityRuntime_EventNames.cpp`
* `MABSAbilityRuntime_Common.h`
* `MABSAbilityRuntime_Common.cpp`
* `MABSAbilityRuntime_Core.cpp`
* `MABSAbilityRuntime_Granting.cpp`
* `MABSAbilityRuntime_Activation.cpp`
* `MABSAbilityRuntime_Delivery.cpp`
* `MABSAbilityRuntime_Effects.cpp`
* `MABSAbilityRuntime_Presentation.cpp`
* `MABSAbilityRuntime_Debug.cpp`
* `Private/Delivery/MABSDeliveryHandler.cpp`
* `Private/Delivery/MABSDirectDeliveryHandler.cpp`
* `Private/Delivery/MABSHitTraceDeliveryHandler.cpp`
* `Private/Delivery/MABSMeleeDeliveryHandler.cpp`
* `Private/Delivery/MABSProjectileDeliveryHandler.cpp`

Phase 11 also adds the public delivery-facing headers:

* `Public/Types/MABSDeliveryHandlerTypes.h`
* `Public/Delivery/MABSDeliveryHandler.h`
* `Public/Delivery/MABSDirectDeliveryHandler.h`
* `Public/Delivery/MABSHitTraceDeliveryHandler.h`
* `Public/Delivery/MABSMeleeDeliveryHandler.h`
* `Public/Delivery/MABSProjectileDeliveryHandler.h`

## Ownership split

Phase 10 intentionally keeps responsibilities like this:

* `MABSGameplay` owns gameplay truth, delivery handlers, and runtime read models
* `MABSDebug` decides how that runtime data is displayed

Inside `MABSGameplay`, helper ownership now follows the same rule:

* event-name constants are shared private runtime data
* label and formatting helpers are shared only when multiple runtime units use them
* debug categorization stays in debug ownership
* delivery trace/query helpers stay in delivery ownership
* built-in delivery handler implementations stay in `MABSGameplay`
* presentation cue/tracer helpers stay in presentation ownership

That means `UMABSAbilityComponent` still exposes compact query helpers such as:

* `GetGrantedAbilityDebugSummaries()`
* `GetCooldownGroupDebugSummaries()`
* `GetComboDebugSummary()`
* `GetPeriodicEffectDebugSummaries()`

The harness UI itself stays in `AMABSDebugHUD`.

## Dependency direction

Current dependency flow is:

* `MABSCore` depends on no other MABS modules
* `MABSGameplay` depends on `MABSCore`
* `MABSDebug` depends on `MABSCore` and consumes `MABSGameplay` runtime data
* `MABSEditor` may depend on runtime modules
