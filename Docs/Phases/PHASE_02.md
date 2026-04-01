# Phase 2 - Targeting and Instant Effects

## What it is

Phase 2 adds the first real gameplay outcome layer to MABS.

This phase keeps the existing multiplayer-safe grant and activation flow and extends it with:

* simple targeting resolution
* simple instant effects
* authority-side effect application
* documentation for how targeting and effects work

## Why it exists

An ability framework is not useful for long unless activation leads to an actual gameplay outcome.

Phase 2 proves that:

* ability data can describe target intent
* authority can resolve a simple target
* authority can apply a simple instant effect
* debug output can explain what happened

## Added in this phase

### Core data

* added `EMABSInstantEffectType`
* expanded `EMABSAbilityActivationResult` with target/effect failure results
* expanded `UMABSAbilityDefinition` with:
  * `InstantEffectType`
  * `EffectMagnitude`
  * `TargetTraceDistance`

### Gameplay execution

* added target resolution for `Self` and `Actor`
* added instant damage application through Unreal's generic damage path
* added optional heal delivery through `IMABSInstantEffectReceiver`
* extended `UMABSAbilityComponent` to emit target/effect debug events and return clean failure results

### Host project harness

* added `Test.Ability.SelfHeal` to the gameplay tag config
* added minimal example health/heal receiver support on `AMABSCharacter`

## How to use it

1. Enable the local `MABS` plugin.
2. Create a `UMABSAbilityDefinition`.
3. Set `TargetType`, `InstantEffectType`, `EffectMagnitude`, and optional `TargetTraceDistance`.
4. Add `UMABSAbilityComponent` to the owning actor.
5. Grant the ability on authority.
6. Call `TryActivateAbilityByTag`.
7. Inspect target/effect debug events.

## Example

Example self-heal flow:

1. Create `DA_Test_SelfHeal` with tag `Test.Ability.SelfHeal`.
2. Set `TargetType` to `Self`.
3. Set `InstantEffectType` to `Heal`.
4. Set `EffectMagnitude` to `25`.
5. Grant it on the player character on the server.
6. Call `TryActivateAbilityByTag(Test.Ability.SelfHeal)`.
7. Expect `RequestAccepted`, `TargetResolved`, `EffectApplied`, and `CommitSucceeded`.

Example actor-damage flow:

1. Create `DA_Test_Fireball` with tag `Test.Ability.Fireball`.
2. Set `TargetType` to `Actor`.
3. Set `InstantEffectType` to `Damage`.
4. Set `EffectMagnitude` to `20`.
5. Set `TargetTraceDistance` to `1500`.
6. Face a target actor and activate the ability.

## Not included in this phase

Phase 2 still does not add:

* cooldown execution
* resource cost payment
* AoE targeting
* location targeting
* team filtering
* advanced target filters
* gameplay effect durations
* stacking
* DOT/HOT
* prediction
* projectiles
* a full health or attribute framework

## Test checklist

Singleplayer:

* self-heal resolves the owner and applies successfully
* actor-targeted damage succeeds when a valid actor is traced
* actor-targeted damage fails cleanly when no actor is found
* debug events clearly show target and effect steps

Listen server:

* host self-heal succeeds
* host damage ability succeeds
* remote client self-heal succeeds through authority
* remote client damage ability succeeds through authority
* remote client receives authoritative debug results

Dedicated server:

* dedicated server target remains present
* remote client requests still resolve targets on the server
* effect application remains server-authoritative
* no editor-only dependency leaks into runtime/server build
