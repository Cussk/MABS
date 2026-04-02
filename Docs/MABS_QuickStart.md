# MABS Quick Start

## What it is

This guide covers the current Phase 5 setup. MABS now supports:

* `Direct`, `HitTrace`, `Melee`, and `Projectile` delivery
* authored `StartupDuration`, `DeliveryTime`, and `RecoveryDuration`
* socket-first delivery origins
* optional activation montage requests

## Why it exists

The goal is to get from a blank actor or character to a multiplayer-safe ability that:

* validates on authority
* enters `Startup`
* delivers at an authored time
* enters `Recovery`
* spends cost and starts cooldown only after successful delivery
* explains timing, socket, and delivery behavior through debug events

## How to use it

### Step 1: Add `UMABSAbilityComponent`

Add the component to the actor or character that owns abilities.

The component owns granted specs, authoritative execution, timing state, cooldowns, costs, and debug data.

### Step 2: Create a `UMABSAbilityDefinition`

Set the core fields:

* `AbilityTag`
* `DisplayName`
* `TargetType`
* `DeliveryMode`
* `InstantEffectType`
* `EffectMagnitude`
* `CooldownSeconds`
* `CooldownGroupTag`
* `ResourceCost`

Set the Phase 5 timing fields:

* `StartupDuration`
* `DeliveryTime`
* `RecoveryDuration`

`RecoveryDuration` should be authored as the total skill time from activation start, not just the tail after delivery.

Set delivery fields for the chosen mode:

* `HitTraceDistance`, `HitTraceRadius`
* `MeleeRange`, `MeleeRadius`, `MeleeForwardOffset`
* `ProjectileActorClass`

Optional socket-first fields:

* `DeliveryOriginSocketName`
* `HitTraceOriginSocketName`, `HitTraceOriginOffset`
* `MeleeOriginSocketName`, `MeleeOriginOffset`
* `ProjectileSpawnSocketName`, `ProjectileSpawnOffset`

Optional montage fields:

* `ActivationMontage`
* `MontagePlayRate`

### Step 3: Implement optional interfaces

Implement:

* `IMABSCostReceiver` if `ResourceCost > 0`
* `IMABSInstantEffectReceiver` on actors that should accept `Heal`

### Step 4: Grant the ability on authority

Call `GrantAbility` on the authoritative `UMABSAbilityComponent`.

Each `FMABSAbilitySpec` stores:

* a stable `FMABSAbilityHandle`
* the definition reference and copied tag
* `RuntimeState`
* `LastActivationResult`
* `CooldownEndTime`
* `ActivationStartTime`
* `ScheduledDeliveryTime`
* `RecoveryEndTime`

### Step 5: Activate by tag

Call `TryActivateAbilityByTag`.

Phase 5 flow is:

1. authority validates the request
2. the ability enters `Startup`
3. the component optionally requests montage playback
4. delivery executes at the authored delivery time
5. cost and cooldown finalize only after successful delivery
6. the ability enters `Recovery`
7. the ability returns to `Idle`

### Step 6: Inspect the runtime overlay and logs

Useful Phase 5 events include:

* `StartupStarted`
* `DeliveryScheduled`
* `DeliveryTriggered`
* `SocketResolved`
* `SocketFallbackUsed`
* `MontagePlayRequested`
* `MontagePlayFailed`
* `RecoveryStarted`
* `RecoveryCompleted`

## Example setups

Example self-heal:

1. Set `TargetType = Self`.
2. Set `DeliveryMode = Direct`.
3. Set `StartupDuration = 0.15`.
4. Set `DeliveryTime = 0.15`.
5. Set `RecoveryDuration = 0.35`.
6. Set `InstantEffectType = Heal`.

Example rifle shot:

1. Set `TargetType = Actor`.
2. Set `DeliveryMode = HitTrace`.
3. Set `HitTraceDistance = 2000`.
4. Set `HitTraceOriginSocketName = Muzzle`.
5. Set `DeliveryTime = 0.08`.

Example sword slash:

1. Set `TargetType = Actor`.
2. Set `DeliveryMode = Melee`.
3. Set `MeleeOriginSocketName = weapon_tip`.
4. Set `DeliveryTime = 0.2`.
5. Set `MeleeRange = 200`.
6. Set `RecoveryDuration = 0.45`.

Example fireball:

1. Set `TargetType = Actor`.
2. Set `DeliveryMode = Projectile`.
3. Set `ProjectileActorClass` to a class derived from `AMABSProjectileBase`.
4. Set `ProjectileSpawnSocketName = hand_r`.
5. Optionally set `ActivationMontage`.

## Validation checklist

Singleplayer:

* direct self-heal respects startup and recovery
* hit trace, melee, and projectile delivery fire at the expected delivery time
* valid sockets are used
* fallback origins work when a socket is missing
* cooldown and cost do not finalize before successful delivery

Listen server:

* remote clients still route activation through authority
* timing remains authoritative
* the owning client receives timing and socket debug events

Dedicated server:

* timed delivery remains authoritative
* projectile spawn timing remains authoritative
* no editor-only dependency leaks into runtime code
