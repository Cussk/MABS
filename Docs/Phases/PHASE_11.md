# PHASE_11 - Custom Delivery Handlers

## What shipped

Phase 11 adds a public delivery-handler extensibility layer to MABS.

The implemented Phase 11 result includes:

* `FMABSDeliveryExecutionContext`
* `FMABSDeliveryExecutionResult`
* `UMABSDeliveryHandler`
* built-in `UMABSDirectDeliveryHandler`
* built-in `UMABSHitTraceDeliveryHandler`
* built-in `UMABSMeleeDeliveryHandler`
* built-in `UMABSProjectileDeliveryHandler`
* authored `DeliveryHandlerClass` support on `UMABSAbilityDefinition`
* delivery-runtime orchestration through handler resolution and execution
* one C++ example custom handler: `UMABSExampleKnockbackHitTraceDeliveryHandler`

## Why it matters

Before Phase 11, projectile delivery already had a project-facing extension seam through `AMABSProjectileBase`, but hit trace and melee did not.

Phase 11 fixes that by letting project code or Blueprint subclass a delivery handler instead of editing `MABSAbilityRuntime_Delivery.cpp`.

## What changed

### `MABSCore`

* `UMABSAbilityDefinition` now stores an optional authored `DeliveryHandlerClass`
* existing authored delivery mode data stays intact

### `MABSGameplay`

* added public delivery handler context/result structs
* added public base and built-in delivery handler classes
* `MABSAbilityRuntime_Delivery.cpp` now resolves a handler and consumes its result
* built-in direct, hit trace, melee, and projectile behavior now run through handler classes

### Sample project

* added `UMABSExampleKnockbackHitTraceDeliveryHandler`

## State model

Phase 11 does not move authoritative runtime state off `UMABSAbilityComponent`.

Gameplay state still lives on:

* `GrantedAbilities`
* `CooldownGroupStates`
* `ActivePeriodicEffects`
* `ActiveAbilityExecutionContexts`
* `RecentDebugEvents`
* `LatestTargetTraceDebugInfo`

Handlers only use:

* transient `FMABSDeliveryExecutionContext`
* transient `FMABSDeliveryExecutionResult`

## Implemented runtime path

Delivery flow now means:

1. authority reaches delivery time for an ability
2. the delivery runtime resolves the authored custom handler or the built-in default handler
3. the handler performs authoritative delivery work
4. the handler returns resolved targets and impact data
5. `UMABSAbilityComponent` continues the normal effect, impact presentation, cooldown, cost, and recovery flow

Projectile impact still routes through `AMABSProjectileBase` back into `UMABSAbilityComponent`.

## Blueprint support

Blueprint subclasses of the built-in hit trace and melee handlers can override `ModifyDeliveryResult(...)` to:

* add knockback or pull logic
* trim or reorder targets
* keep the normal built-in trace / sweep behavior

## Documentation updated

Phase 11 updates or adds:

* `Docs/MABS_Abilities.md`
* `Docs/MABS_API.md`
* `Docs/MABS_Architecture.md`
* `Docs/MABS_Delivery.md`
* `Docs/MABS_HitTrace.md`
* `Docs/MABS_Melee.md`
* `Docs/MABS_Modules.md`
* `Docs/MABS_Overview.md`
* `Docs/MABS_Projectiles.md`
* `Docs/Phases/PHASE_11.md`

## Validation

Code validation completed with:

* `Build.bat MABSEditor Win64 Development C:\Users\cussp\OneDrive\Documents\UnrealProjects\MABS\MABS.uproject -WaitMutex -NoHotReloadFromIDE`

Recommended runtime validation:

* singleplayer default direct / hit trace / melee / projectile regression
* custom C++ hit trace handler using `UMABSExampleKnockbackHitTraceDeliveryHandler`
* Blueprint subclass of `UMABSMeleeDeliveryHandler` or `UMABSHitTraceDeliveryHandler`
* listen-server authority validation for custom handler outcomes
* dedicated-server authority validation for custom handler outcomes
