# MABS Quick Start

## What it is

This guide covers the current Phase 4 setup. MABS now supports the Phase 3 cooldown and cost path plus the first real delivery layer:

* `Direct`
* `HitTrace`
* `Melee`
* `Projectile`

## Why it exists

The goal is to get from a blank actor or character to a verified multiplayer-safe ability that:

* validates on authority
* delivers through the authored mode
* applies an instant effect
* spends cost and starts cooldown only on authority
* explains success or failure through logs, debug events, and the overlay

## How to use it

### Step 1: Build the project with the plugin enabled

Compile the project with the local `MABS` plugin enabled and verify that:

* `MABSCore`
* `MABSGameplay`
* `MABSDebug`
* `MABSEditor`

load without startup errors.

### Step 2: Add the ability component

Add `UMABSAbilityComponent` to the actor or character that should own abilities.

The component owns granted specs, authoritative activation, delivery execution, cooldowns, costs, and debug state.

### Step 3: Create an ability definition asset

Create a `MABSAbilityDefinition` and set the core fields:

* `AbilityTag`
* `DisplayName`
* `ActivationPolicy`
* `TargetType`
* `DeliveryMode`
* `InstantEffectType`
* `EffectMagnitude`
* `CooldownSeconds`
* `CooldownGroupTag`
* `ResourceCost`

Direct abilities still use the older target authoring:

* `TargetTraceDistance`
* `ActorTargetTraceMode`
* `TargetTraceRadius`
* `bRequireValidActorTarget`
* `bIgnoreNonTargetWorldHits`

Phase 4 delivery fields are:

* `HitTraceDistance`
* `HitTraceRadius`
* `MeleeRange`
* `MeleeRadius`
* `MeleeForwardOffset`
* `ProjectileActorClass`
* `ProjectileSpawnOffset`

### Step 4: Implement optional interfaces

Implement:

* `IMABSCostReceiver` if `ResourceCost > 0`
* `IMABSInstantEffectReceiver` on actors that should accept `Heal`

The host-project `AMABSCharacter` still provides a minimal example of both.

### Step 5: Grant an ability on the server

Call `GrantAbility` on the authoritative `UMABSAbilityComponent`.

Each grant creates a `FMABSAbilitySpec` that stores:

* a stable `FMABSAbilityHandle`
* the definition reference
* the copied ability tag
* runtime state
* the last activation result
* the personal cooldown end time

### Step 6: Request activation by tag

Call `TryActivateAbilityByTag`.

Behavior now depends on delivery mode:

* `Direct`: resolves the target immediately and applies the effect immediately
* `HitTrace`: performs an authority-side trace and applies the effect on the first valid hit
* `Melee`: performs an authority-side short-range sweep and applies the effect on the first valid hit
* `Projectile`: spawns an authority-side projectile, then applies the effect later on authoritative impact

### Step 7: Inspect debug output

Useful Phase 4 event names include:

* `DeliveryStarted`
* `DeliveryFailed`
* `HitTraceHit`
* `HitTraceRejected`
* `MeleeHit`
* `MeleeRejected`
* `ProjectileSpawned`
* `ProjectileSpawnFailed`
* `ProjectileImpact`
* `ProjectileImpactRejected`
* `EffectApplied`
* `CostSpent`
* `CooldownStarted`
* `CommitSucceeded`

### Step 8: Use the runtime overlay

`AMABSDebugHUD` still provides the current runtime overlay. It now shows:

* the latest target or delivery trace snapshot
* granted ability summaries, including delivery mode and cooldown remaining
* the recent event list

## Example setups

Example direct self-heal:

1. Set `TargetType = Self`.
2. Set `DeliveryMode = Direct`.
3. Set `InstantEffectType = Heal`.
4. Set `EffectMagnitude = 25`.

Example rifle shot:

1. Set `TargetType = Actor`.
2. Set `DeliveryMode = HitTrace`.
3. Set `HitTraceDistance = 2000`.
4. Set `HitTraceRadius = 0`.
5. Set `InstantEffectType = Damage`.

Example sword slash:

1. Set `TargetType = Actor`.
2. Set `DeliveryMode = Melee`.
3. Set `MeleeRange = 200`.
4. Set `MeleeRadius = 75`.
5. Set `InstantEffectType = Damage`.

Example fireball:

1. Set `TargetType = Actor`.
2. Set `DeliveryMode = Projectile`.
3. Set `ProjectileActorClass` to a class derived from `AMABSProjectileBase`.
4. Set `ProjectileSpawnOffset`.
5. Set `InstantEffectType = Damage`.

## Validation checklist

Singleplayer:

* direct self-heal still works
* hit-trace abilities resolve a valid target
* melee abilities resolve a valid nearby target
* projectile abilities spawn and impact correctly
* cooldown and cost still work with each delivery mode

Listen server:

* host can use every delivery mode
* remote clients still route activation through authority
* projectile spawn and impact stay authoritative
* the owning client receives authoritative delivery events

Dedicated server:

* delivery logic remains server-authoritative
* projectile spawn and impact remain server-authoritative
* remote clients cannot fake delivery results
