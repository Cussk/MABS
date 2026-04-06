# MABS Validation

## What it is

Phase 12 adds editor-side validation for `UMABSAbilityDefinition` authoring and documents the v1 regression matrix for built-in and custom delivery flows.

## Why it exists

MABS now has enough delivery and effect breadth that bad content needs to fail before gameplay testing wastes time.

Phase 12 keeps runtime authority and state ownership the same, but adds clearer authoring guardrails.

## Which subsystem owns it

* `MABSEditor` owns `UMABSAbilityDefinitionValidator`
* `MABSCore` still owns `UMABSAbilityDefinition`
* `MABSGameplay` still owns authoritative runtime execution on `UMABSAbilityComponent`

## What state it uses

The validator does not add new gameplay state.

It only reads authored data already stored on `UMABSAbilityDefinition`, such as:

* `DeliveryMode`
* `DeliveryHandlerClass`
* `TargetType`
* direct target-trace fields
* hit-trace fields
* melee fields
* `ProjectileActorClass`
* `Combo`
* `AoE`
* `PeriodicEffect`

Runtime gameplay state still lives on `UMABSAbilityComponent`:

* `GrantedAbilities`
* `CooldownGroupStates`
* `ActivePeriodicEffects`
* `ActiveAbilityExecutionContexts`
* `RecentDebugEvents`
* `LatestTargetTraceDebugInfo`

## What gets validated

### General ability authoring

* `AbilityTag` must be valid
* an ability must author an instant or periodic gameplay effect
* instant effects require `EffectMagnitude > 0`
* enabled `PeriodicEffect` data must have valid duration, interval, and magnitude
* enabled `AoE` data must author a valid shape size
* combo follow-ups are only valid on `Melee` abilities and must author a real combo window
* `TargetType = Location` remains invalid in v1

### Delivery handler authoring

* `DeliveryHandlerClass` must load successfully
* `DeliveryHandlerClass` must derive from `UMABSDeliveryHandler`
* `DeliveryHandlerClass` must not be abstract
* obvious mode/base mismatches are rejected

Examples:

* a `UMABSMeleeDeliveryHandler` subclass on a `Projectile` ability
* a `UMABSProjectileDeliveryHandler` subclass on a `HitTrace` ability

An empty `DeliveryHandlerClass` is still valid and falls back to the built-in handler for the authored `DeliveryMode`.

### Delivery-mode authoring

`Direct`

* `TargetType` must be `Self` or `Actor`
* direct actor targeting requires positive `TargetTraceDistance`
* direct sphere targeting requires positive `TargetTraceRadius`

`HitTrace`

* `TargetType` must be `Actor`
* `HitTraceDistance` must be greater than zero

`Melee`

* `TargetType` must be `Actor`
* `MeleeRange` must be greater than zero
* `MeleeRadius` must be greater than zero

`Projectile`

* `TargetType` must be `Actor`
* `ProjectileActorClass` must be set
* `ProjectileActorClass` must derive from `AMABSProjectileBase`
* `ProjectileActorClass` must not be abstract

## What custom handlers are allowed to customize

* authoritative delivery logic
* target filtering and final resolved targets
* impact data and delivery-side rules
* extra delivery-side outcomes such as knockback

## What custom handlers are not expected to own

* replicated runtime state
* cooldown state
* active execution timers
* periodic-effect state
* authority transfer or prediction

## Example failures

* `Ability 'DA_Test_Fireball' uses Projectile delivery but ProjectileActorClass is unset.`
* `Ability 'DA_Test_RifleShot' sets DeliveryHandlerClass '/Game/...', but that class could not be loaded.`
* `Ability 'DA_Test_SwordSwing' uses Melee delivery but DeliveryHandlerClass 'BP_CustomProjectileHandler_C' derives from UMABSProjectileDeliveryHandler.`

## How to use it

1. Author or edit a `UMABSAbilityDefinition`.
2. Run asset validation in the editor.
3. Fix invalid handler or delivery authoring before gameplay testing.
4. Use the runtime debug harness only after the asset validates cleanly.

## V1 regression matrix

### Singleplayer

Primary test assets:

* `DA_Test_SelfHeal` for direct / self
* `DA_Test_RifleShot` for built-in hit trace
* `DA_Test_SwordSwing`, `DA_Test_SwordSwing1`, and `DA_Test_SwordSwing2` for melee and combo flow
* `DA_Test_Fireball` for projectile, AoE, and periodic flow
* `UMABSExampleKnockbackHitTraceDeliveryHandler` for custom C++ delivery coverage

Verify:

* built-in direct
* built-in hit trace
* built-in melee
* built-in projectile
* cooldown start, reject, and expire
* cost validate, reject, and spend
* periodic apply, tick, refresh, and expire
* combo queue and follow-up
* AoE gather and apply
* debug harness enabled
* debug harness disabled
* empty `DeliveryHandlerClass` uses built-in fallback
* one Blueprint melee or hit-trace handler derived from the correct built-in handler base

### Listen server

Verify:

* host can grant and activate all default delivery modes
* remote client sees authoritative gameplay results
* custom handler gameplay outcomes happen only on authority
* projectile delivery still routes through `AMABSProjectileBase`
* owner-facing debug data remains correct when enabled

### Dedicated server

Verify:

* server owns gameplay truth for default and custom handler paths
* clients see expected gameplay and presentation results
* projectile impact still routes through `AMABSProjectileBase`
* gameplay still works with the debug harness disabled
* empty `DeliveryHandlerClass` still falls back to the built-in handler
