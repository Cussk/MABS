# MABS Quick Start

## What it is

This guide covers the current Phase 8 setup. MABS now supports:

* individual `UMABSAbilityDefinition` assets
* grouped `UMABSAbilitySet` assets
* `Direct`, `HitTrace`, `Melee`, and `Projectile` delivery
* authored startup, delivery, and recovery timing
* optional combo, AoE, and periodic effect data
* startup, delivery, tracer, projectile-travel, and impact presentation
* the Phase 8 runtime debug harness

## Why it exists

The goal is to get from a blank actor or character to a multiplayer-safe ability workflow that:

* validates and grants on authority
* lets designers author one ability at a time
* optionally groups multiple abilities into a starting bundle
* activates by gameplay tag through the normal existing runtime path
* exposes readable runtime inspection for grant, activation, delivery, combo, periodic, and cue flow

## How to use it

### Step 1: Add `UMABSAbilityComponent`

Add the component to the actor or character that owns abilities.

### Step 2: Create one or more `UMABSAbilityDefinition` assets

Set:

* core ability fields
* target and delivery fields
* timing fields
* optional combo, AoE, or periodic effect fields
* optional presentation fields

### Step 3: Optionally create a `UMABSAbilitySet`

Create a data asset of type `UMABSAbilitySet` and fill `AbilityDefinitions` with the abilities that should be granted together.

Use sets for:

* starting player abilities
* archetype loadouts
* enemy ability bundles
* test harness bundles

### Step 4: Grant on authority

Grant either:

* a single ability with `GrantAbility(...)`
* one set with `GrantAbilitySet(...)`
* multiple sets with `GrantAbilitySets(...)`

Grouped granting is still a convenience workflow only. It reuses the normal per-ability grant rules.

### Step 5: Activate by tag

Call `TryActivateAbilityByTag(...)` with the tag of a granted ability.

### Step 6: Optionally enable the runtime harness

If you want in-game inspection:

* set the HUD class to `AMABSDebugHUD`
* leave debug replication enabled on authority
* run `mabs.DebugHarness 1`

## Example setups

Example starting player bundle:

* create `DA_Player_Attack`
* create `DA_Player_Dodge`
* create `DA_Player_Heal`
* create `DA_StartingAbilities_Player` as a `UMABSAbilitySet`
* fill `AbilityDefinitions` with the three ability assets
* call `GrantAbilitySet(DA_StartingAbilities_Player)` on the server at spawn

Example single authored projectile with harness inspection:

* `DeliveryMode = Projectile`
* `ProjectileActorClass = BP_MABS_Fireball`
* `DeliveryPresentation.Cue.VFX = P_FireballCast`
* `DeliveryPresentation.ProjectileTravel.TravelVFX = P_FireballTrail`
* `ImpactPresentation.Cue.VFX = P_FireballImpact`
* set the HUD class to `AMABSDebugHUD`
* use the harness to inspect the targeting / delivery section and recent event history

## Multiplayer note

Ability sets do not add a separate replication model.

The server still grants the underlying abilities one by one through the existing authority path, granted runtime state still lives on `UMABSAbilityComponent`, and the Phase 8 harness reads that authoritative state back on the owning client.
