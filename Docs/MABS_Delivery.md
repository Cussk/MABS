# MABS Delivery

## What it is

This document explains the Phase 13 delivery layer in MABS.

The delivery runtime itself is unchanged from Phase 12:

* built-in direct, hit trace, melee, and projectile handlers
* optional authored `DeliveryHandlerClass`
* authority-owned target resolution, effect application, commit, and recovery

Phase 13 adds a better proof surface for that runtime:

* the sample demo HUD reads delivery summaries and recent delivery events
* the sample quickstart walks users through delivery modes in a fixed order
* the project sample still includes `UMABSExampleKnockbackHitTraceDeliveryHandler` as the custom-handler proof slice

## Why it exists

By Phase 12 the delivery layer was correct and extensible, but a new user still had to infer too much from logs and asset panels.

Phase 13 changes that by making delivery behavior more legible in the sample scene:

* the hotbar exposes ready/cooldown/cost state
* the target panel exposes trace and hit feedback
* the feature callout panel explains which delivery feature is currently being shown

## Built-in delivery handlers

MABS still exposes:

* `UMABSDirectDeliveryHandler`
* `UMABSHitTraceDeliveryHandler`
* `UMABSMeleeDeliveryHandler`
* `UMABSProjectileDeliveryHandler`

If an ability leaves `DeliveryHandlerClass` empty, MABS resolves the correct built-in handler from `DeliveryMode`.

## Sample proof path

Phase 13 is intentionally opinionated about how delivery should be demonstrated in the sample scene:

1. direct self-heal
2. built-in hit trace shot
3. built-in melee swing
4. combo follow-up
5. projectile with AoE and periodic behavior
6. custom knockback hit trace handler

That order makes it obvious which parts are built in and where extensibility begins.

## Runtime behavior

On authority, MABS still:

1. resolves the authored or built-in delivery handler
2. builds a transient `FMABSDeliveryExecutionContext`
3. runs the handler
4. reads back `FMABSDeliveryExecutionResult`
5. applies effects, presentation, cost, cooldown, and recovery through `UMABSAbilityComponent`

Handlers still do not own authoritative gameplay state or replicated state.

## Commit rules

Commit behavior is unchanged:

`Direct`, `HitTrace`, and `Melee`:

* commit after delivery succeeds and standard gameplay flow reaches cost/cooldown finalization

`Projectile`:

* commit after authoritative projectile spawn succeeds
* apply gameplay results later on authoritative impact

## Custom-handler sample

The sample project still ships `UMABSExampleKnockbackHitTraceDeliveryHandler`.

That example proves:

* the normal built-in hit trace still resolves targets
* a custom handler can modify the delivery result
* authoritative knockback still lives inside the normal MABS delivery flow
* the sample HUD can call out that behavior as a distinct station in the sample scene

## How to extend it

Use the smallest handler base that matches the authored mode:

* derive from `UMABSDirectDeliveryHandler` for direct customization
* derive from `UMABSHitTraceDeliveryHandler` for ranged instant-shot customization
* derive from `UMABSMeleeDeliveryHandler` for close-range customization
* derive from `UMABSProjectileDeliveryHandler` for projectile customization

Use Phase 13 sample content to demonstrate the custom behavior clearly:

* add a station label
* add a hotbar entry in `UMABSDemoDisplayConfig`
* give the feature callout text a plain-language summary

## How to use it

1. Author the normal delivery mode and related fields on `UMABSAbilityDefinition`.
2. Leave `DeliveryHandlerClass` empty for the built-in handler.
3. Assign a custom handler class only when one ability needs custom delivery behavior.
4. Run asset validation before runtime.
5. In the sample scene, wire that ability tag into `UMABSDemoDisplayConfig` so the HUD can label it clearly.
6. Verify the result in standalone, listen server, and dedicated server tests.

## Example

Example custom knockback lane:

* `DeliveryMode = HitTrace`
* `DeliveryHandlerClass = UMABSExampleKnockbackHitTraceDeliveryHandler`
* the sample station label says `Custom Delivery Handler`
* the demo HUD feature summary says `Custom knockback hit trace handler`
* runtime delivery events still come from `UMABSAbilityComponent`

## Not included

Phase 13 does not add:

* a new delivery runtime subsystem
* handler-owned replicated state
* handler-owned timers
* prediction
* a second AoE framework
