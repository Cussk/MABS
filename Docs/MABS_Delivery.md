# MABS Delivery

## What it is

This document explains the Phase 12 delivery layer in MABS.

Phase 11 moved built-in delivery behavior behind public delivery handler classes. Phase 12 keeps that design and adds validation for invalid handler and delivery authoring.

## Why it exists

Before Phase 11, projectile delivery already had an obvious extension seam through `AMABSProjectileBase`, but hit trace and melee did not.

Phase 11 fixes that by adding:

* a shared `UMABSDeliveryHandler` base class
* built-in direct / hit trace / melee / projectile handlers
* an authored handler-class reference on `UMABSAbilityDefinition`

## Built-in delivery handlers

MABS now exposes:

* `UMABSDirectDeliveryHandler`
* `UMABSHitTraceDeliveryHandler`
* `UMABSMeleeDeliveryHandler`
* `UMABSProjectileDeliveryHandler`

If an ability does not assign a custom handler, MABS resolves the correct built-in handler from `DeliveryMode`.

## Authoring changes

`UMABSAbilityDefinition` still uses the normal authored delivery data:

* `DeliveryMode`
* delivery socket names and offsets
* hit trace / melee / projectile authored fields
* `AoE`
* `PeriodicEffect`
* presentation groups

Phase 11 adds one optional authored class reference:

* `DeliveryHandlerClass`

Leave it empty to use the built-in handler. Set it only when one ability needs custom delivery behavior.

## Phase 12 validation

Phase 12 validates common delivery authoring mistakes before runtime:

* `DeliveryHandlerClass` must load successfully
* `DeliveryHandlerClass` must derive from `UMABSDeliveryHandler`
* `DeliveryHandlerClass` must not be abstract
* obvious handler-mode mismatches such as melee handlers on projectile abilities are rejected
* built-in delivery requirements such as trace distance, melee range, and `ProjectileActorClass` are validated from the authored `DeliveryMode`

## Runtime behavior

On authority, MABS now:

1. resolves the authored custom handler or the built-in default handler
2. builds a transient `FMABSDeliveryExecutionContext`
3. runs the handler
4. reads back a transient `FMABSDeliveryExecutionResult`
5. continues normal effect application, impact presentation, cost, cooldown, and recovery flow on `UMABSAbilityComponent`

Handlers do not own authoritative gameplay state or replicated state.

## Commit rules

The commit rules stay the same:

`Direct`, `HitTrace`, and `Melee`:

* commit after delivery succeeds and standard gameplay flow reaches cost/cooldown finalization

`Projectile`:

* commits after authoritative projectile spawn succeeds
* applies instant and periodic gameplay outcomes later on authoritative impact

## How to extend it

Use the smallest handler base that matches the authored delivery mode:

* derive from `UMABSHitTraceDeliveryHandler` for custom ranged instant-shot logic
* derive from `UMABSMeleeDeliveryHandler` for custom close-range strike logic
* derive from `UMABSProjectileDeliveryHandler` for custom projectile spawn / routing logic
* derive from `UMABSDirectDeliveryHandler` for direct-target customization

Built-in hit trace and melee handlers expose a Blueprint/C++ hook through `ModifyDeliveryResult(...)`, so a subclass can:

* trim or reorder targets
* add knockback or pull logic
* add extra delivery-side rules
* keep normal MABS effects, presentation, cooldown, and cost flow

## AoE note

AoE is still part of delivery and targeting in Phase 11.

This phase does not add a separate AoE handler framework.

## How to use it

1. Author the normal delivery mode and delivery data on `UMABSAbilityDefinition`.
2. Leave `DeliveryHandlerClass` empty for default behavior.
3. Create a C++ or Blueprint subclass of the appropriate built-in delivery handler when one ability needs custom delivery logic.
4. Assign that handler class to `DeliveryHandlerClass`.
5. Run asset validation and fix any invalid handler or delivery errors.
6. Grant and activate the ability on authority as usual.

## Example

Example custom rifle shot:

* `DeliveryMode = HitTrace`
* `DeliveryHandlerClass = UMABSExampleKnockbackHitTraceDeliveryHandler`
* built-in hit trace still resolves the target
* the custom handler adds a knockback impulse to each resolved target
* normal MABS effects, impact presentation, cooldown, and recovery still run

## Not included

Phase 11 does not add:

* a separate AoE handler framework
* client-side prediction
* handler-owned replicated state
* handler-owned timers
* a new projectile actor model
