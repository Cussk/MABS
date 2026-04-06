# MABS Projectiles

## What it is

This document explains the built-in projectile path in MABS after the Phase 12 validation pass on top of the Phase 11 delivery-handler work.

## Why it exists

Projectiles are still the world-space delivery mode for abilities that travel over time.

Phase 11 keeps the existing projectile actor model, but adds `UMABSProjectileDeliveryHandler` so projectile spawn and routing use the same delivery-handler system as the other delivery modes.

## Built-in runtime types

`UMABSProjectileDeliveryHandler` in `MABSGameplay`:

* owns authoritative projectile spawn / routing
* commits the ability after projectile spawn succeeds

`AMABSProjectileBase` in `MABSGameplay`:

* replicates movement
* stores replicated source ability context
* routes impact handling back through `UMABSAbilityComponent`
* applies the authored instant and periodic effects on authoritative impact
* realizes projectile-travel presentation locally from replicated cue data

## Authored fields

Use:

* `TargetType = Actor`
* `DeliveryMode = Projectile`
* optional `DeliveryHandlerClass`
* `ProjectileActorClass`
* `ProjectileSpawnOffset`
* `DeliveryPresentation.Cue`
* `DeliveryPresentation.ProjectileTravel`
* `ImpactPresentation.Cue`

`ProjectileActorClass` must derive from `AMABSProjectileBase`.

Phase 12 also requires `ProjectileActorClass` to be non-abstract.

## Runtime behavior

Projectile abilities still work the same way for gameplay:

* the server spawns the projectile
* successful spawn commits the ability
* the effect applies later on authoritative impact

Phase 11 only changes the spawn side:

* the delivery runtime now resolves `UMABSProjectileDeliveryHandler`
* the built-in projectile handler performs the authoritative spawn
* impact still routes through `AMABSProjectileBase`

## Validation

Phase 12 validates the common projectile authoring mistakes:

* `TargetType` must be `Actor`
* `ProjectileActorClass` must be set
* `ProjectileActorClass` must derive from `AMABSProjectileBase`
* `ProjectileActorClass` must not be abstract
* if a custom handler class is set, it must be loadable, non-abstract, and derive from `UMABSDeliveryHandler`
* a handler derived from a different built-in mode-specific base is rejected as incompatible authoring

## How to extend it

Use:

* a `UMABSProjectileDeliveryHandler` subclass when you want custom projectile spawn / routing behavior
* an `AMABSProjectileBase` subclass when you want custom in-world projectile behavior or impact behavior

## Example

Example custom fireball spawn:

* `DeliveryMode = Projectile`
* `DeliveryHandlerClass = UMyGameCustomProjectileHandler`
* `ProjectileActorClass = BP_MABS_Fireball`
* the custom handler changes spawn / routing rules
* `AMABSProjectileBase` still handles the projectile’s replicated in-world life
