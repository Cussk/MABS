# PHASE_12 - Validation and Regression Hardening

## What shipped

Phase 12 adds an editor validation surface for authored abilities and documents the v1 regression matrix.

The implemented Phase 12 result includes:

* `UMABSAbilityDefinitionValidator` in `MABSEditor`
* editor validation for invalid `DeliveryHandlerClass` setups
* editor validation for common direct / hit trace / melee / projectile authoring mistakes
* editor validation for invalid projectile class authoring
* runtime backstops for abstract projectile classes and obvious handler-mode mismatches
* `Docs/MABS_Validation.md`

## Why it matters

Phase 11 made delivery extensible. Phase 12 makes that extensibility safer to hand to another project by catching obvious bad content earlier and more clearly.

## What changed

### `MABSEditor`

* added `UMABSAbilityDefinitionValidator`
* validates authored delivery-handler and delivery-mode content

### `MABSGameplay`

* rejects abstract projectile classes during activation
* rejects obvious authored handler-mode mismatches during delivery-handler resolution

### State model

Phase 12 does not move authoritative runtime state off `UMABSAbilityComponent`.

Gameplay state still lives on:

* `GrantedAbilities`
* `CooldownGroupStates`
* `ActivePeriodicEffects`
* `ActiveAbilityExecutionContexts`
* `RecentDebugEvents`
* `LatestTargetTraceDebugInfo`

The validator only reads authored `UMABSAbilityDefinition` fields. It does not create runtime gameplay state.

## Documented regression matrix

Phase 12 documents the v1 regression matrix in `Docs/MABS_Validation.md`.

That matrix covers:

* direct / hit trace / melee / projectile
* cooldowns
* costs
* periodic effects
* combo flow
* AoE
* debug harness enabled and disabled
* custom C++ handler coverage with `UMABSExampleKnockbackHitTraceDeliveryHandler`
* Blueprint custom handler coverage
* singleplayer, listen server, and dedicated server expectations

## Documentation updated

Phase 12 updates or adds:

* `Docs/MABS_Abilities.md`
* `Docs/MABS_API.md`
* `Docs/MABS_Delivery.md`
* `Docs/MABS_HitTrace.md`
* `Docs/MABS_Melee.md`
* `Docs/MABS_Overview.md`
* `Docs/MABS_Projectiles.md`
* `Docs/MABS_Validation.md`
* `Docs/Phases/PHASE_12.md`

## Validation command

Use:

* `Build.bat MABSEditor Win64 Development C:\Users\cussp\OneDrive\Documents\UnrealProjects\MABS\MABS.uproject -WaitMutex -NoHotReloadFromIDE`

Recommended runtime validation:

* singleplayer regression using `DA_Test_SelfHeal`, `DA_Test_RifleShot`, `DA_Test_SwordSwing`, `DA_Test_SwordSwing1`, `DA_Test_SwordSwing2`, and `DA_Test_Fireball`
* custom C++ hit trace handler using `UMABSExampleKnockbackHitTraceDeliveryHandler`
* one Blueprint subclass of `UMABSMeleeDeliveryHandler` or `UMABSHitTraceDeliveryHandler`
* listen-server authority verification for custom handler outcomes
* dedicated-server authority verification for custom handler outcomes
