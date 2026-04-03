# MABS Ability Sets

## What it is

`UMABSAbilitySet` is a lightweight data asset that groups multiple `UMABSAbilityDefinition` references so they can be granted together.

## Why it exists

By Phase 7.5, MABS already has enough ability breadth that users should not have to grant every starting or themed ability one by one in code or Blueprint.

Ability sets solve common grouped-grant cases such as:

* starting player abilities
* class or archetype bundles
* enemy bundles
* boss phase bundles
* test harness bundles

## What it contains

`UMABSAbilitySet` currently exposes:

* `AbilityDefinitions`

This phase intentionally keeps the asset small. It does not add nested sets, grant conditions, progression rules, or revoke-by-set bookkeeping.

## Runtime behavior

When `GrantAbilitySet(...)` runs on `UMABSAbilityComponent`:

1. authority validates the set asset
2. the component iterates `AbilityDefinitions`
3. null entries are skipped safely
4. each valid ability is granted through the existing `GrantAbility(...)` path
5. the same duplicate policy as individual grant still applies
6. set-level debug events summarize what happened

`GrantAbilitySets(...)` simply loops through multiple set assets and reuses `GrantAbilitySet(...)`.

## What state it uses

Ability sets do not create new replicated runtime state.

The existing state remains:

* `GrantedAbilities` on `UMABSAbilityComponent`
* `FMABSAbilitySpec` for per-ability runtime data
* normal debug event history on the component

## How to use it

1. Create several `UMABSAbilityDefinition` assets.
2. Create a data asset of type `UMABSAbilitySet`.
3. Fill `AbilityDefinitions` with the ability assets that belong together.
4. On authority, call `GrantAbilitySet(...)` or `GrantAbilitySets(...)`.
5. Activate the granted abilities normally by tag.

## Example

Example starting player bundle:

* create `DA_Player_Attack`
* create `DA_Player_Dodge`
* create `DA_Player_Heal`
* create `DA_StartingAbilities_Player`
* fill `AbilityDefinitions = [DA_Player_Attack, DA_Player_Dodge, DA_Player_Heal]`
* call `GrantAbilitySet(DA_StartingAbilities_Player)` on the server

## Not included in this phase

Phase 7.5 does not add:

* loadout swapping systems
* equipment systems
* progression unlock logic
* nested ability sets
* set-specific duplicate rules
* set-specific replication tracking

Ability sets are a grouped granting convenience feature, not a loadout framework.
