# Phase 3 - Costs and Cooldowns

## What it is

Phase 3 adds the first real ability-usage restrictions to MABS.

This phase adds:

* per-ability cooldown execution
* shared cooldown-group support
* simple authoritative resource cost validation
* authoritative cost spending
* explicit cooldown and cost denial reasons
* runtime query helpers and updated debug visibility

## Why it exists

Earlier phases proved that MABS could grant abilities, resolve targets, apply effects, and expose good runtime debug visibility. Phase 3 makes those abilities behave more like real gameplay actions by answering two common questions:

* is this ability still on cooldown?
* can the owner afford to use it?

## Added in this phase

### Core data

Added:

* `OnCooldown` and `InsufficientResources` activation results
* `CooldownGroupTag` on `UMABSAbilityDefinition`
* `CooldownEndTime` on `FMABSAbilitySpec`
* `FMABSCooldownGroupState`

### Gameplay execution

Added or changed:

* `IMABSCostReceiver`
* cooldown validation before target/effect execution
* cost affordability validation before target/effect execution
* authoritative cost spending after successful effect application
* authoritative cooldown start on successful commit
* cooldown-group runtime tracking on `UMABSAbilityComponent`
* cooldown query helpers on `UMABSAbilityComponent`

### Debugging

Added or changed:

* `CooldownRejected`
* `CooldownStarted`
* `CostValidated`
* `CostRejected`
* `CostSpent`
* HUD ability-status summaries that show cooldown remaining

### Host project harness

Added:

* example resource pool support on `AMABSCharacter`
* example cooldown-group tags in `DefaultGameplayTags.ini`

## How to use it

1. Create or update an ability definition.
2. Set `CooldownSeconds`.
3. Optionally set `CooldownGroupTag`.
4. Set `ResourceCost` if the ability should spend a resource.
5. Implement `IMABSCostReceiver` on the owning actor when using cost.
6. Activate the ability and inspect:
   * cooldown query helpers
   * recent debug events
   * the runtime overlay

## Example

Example self-heal verification flow:

1. Set `CooldownSeconds = 5`.
2. Set `CooldownGroupTag = Test.CooldownGroup.Support`.
3. Set `ResourceCost = 20`.
4. Activate the ability once.
5. Verify the effect succeeds, cost is spent, and cooldown starts.
6. Activate it again immediately.
7. Verify `CooldownRejected`.
8. Wait for cooldown expiry, reduce resource below `20`, and activate again.
9. Verify `CostRejected`.

## Not included in this phase

Phase 3 still does not add:

* a full attribute/stat system
* resource regeneration
* cooldown reduction stats
* charge systems
* complex formulas
* UI beyond the existing debug surfaces

## Test checklist

Singleplayer:

* self-heal starts cooldown after success
* self-heal cannot be reactivated until cooldown expires
* fireball starts cooldown after success
* cooldown denial produces readable debug output
* cost denial produces readable debug output
* cost is not spent on failed target resolution
* cooldown does not start on failed target resolution

Listen server:

* host cooldown behavior is correct
* host cost spending is authoritative and correct
* remote client receives correct cooldown/cost denial results
* remote client cannot bypass cooldown or cost checks

Dedicated server:

* cooldown and cost validation happen only on the server
* owning remote client receives authoritative result and debug info
* no editor-only dependency leaks into runtime or server build

## Deliverables

Phase 3 delivers:

1. Per-ability cooldown support
2. Cooldown-group support
3. Simple authoritative resource cost support
4. Cost/cooldown debug events and readable denial reasons
5. Updated user-facing documentation
