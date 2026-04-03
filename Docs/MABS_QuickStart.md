# MABS Quick Start

## What it is

This guide covers the current Phase 6 setup. MABS now supports:

* `Direct`, `HitTrace`, `Melee`, and `Projectile` delivery
* authored `StartupDuration`, `DeliveryTime`, and `RecoveryDuration`
* socket-first delivery origins
* optional activation montage requests
* authored startup, delivery, tracer, projectile-travel, and impact presentation

## Why it exists

The goal is to get from a blank actor or character to a multiplayer-safe ability that:

* validates on authority
* enters `Startup`
* delivers at an authored time
* plays cosmetics at startup, delivery, and impact
* enters `Recovery`
* spends cost and starts cooldown only after successful delivery
* explains timing, socket, tracer, and impact behavior through debug events

## How to use it

### Step 1: Add `UMABSAbilityComponent`

Add the component to the actor or character that owns abilities.

### Step 2: Create a `UMABSAbilityDefinition`

Set:

* core ability fields
* timing fields
* delivery fields
* socket fields
* optional montage fields

### Step 3: Fill presentation data

Startup:

* `StartupPresentation.Cue`

Delivery:

* `DeliveryPresentation.Cue`
* `DeliveryPresentation.HitTraceTracer`
* `DeliveryPresentation.ProjectileTravel`

Impact:

* `ImpactPresentation.Cue`

### Step 4: Grant and activate

Grant the ability on authority and call `TryActivateAbilityByTag`.

Phase 6 flow is:

1. authority validates the request
2. startup begins
3. startup presentation triggers if authored
4. delivery executes at the authored delivery time
5. delivery presentation triggers at that same time
6. hit trace may spawn a tracer, and projectiles may start travel presentation
7. impact presentation triggers after successful effect application or projectile impact
8. cost and cooldown finalize only after successful gameplay commit
9. the ability enters `Recovery` and later returns to `Idle`

## Example setups

Example self-heal:

* `DeliveryMode = Direct`
* `StartupPresentation.Cue.SFX = Heal_Cast`
* `ImpactPresentation.Cue.VFX = P_HealBurst`

Example rifle shot:

* `DeliveryMode = HitTrace`
* `HitTraceOriginSocketName = Muzzle`
* `DeliveryPresentation.Cue.VFX = P_RifleMuzzle`
* `DeliveryPresentation.HitTraceTracer.TracerVFX = P_RifleTracer`
* `ImpactPresentation.Cue.VFX = P_BulletImpact`

Example fireball:

* `DeliveryMode = Projectile`
* `ProjectileActorClass = BP_MABS_Fireball`
* `DeliveryPresentation.Cue.VFX = P_FireballCast`
* `DeliveryPresentation.ProjectileTravel.TravelVFX = P_FireballTrail`
* `ImpactPresentation.Cue.VFX = P_FireballImpact`
