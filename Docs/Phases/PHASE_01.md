# Phase 01 - Ability Granting and Activation Flow

## What it is

Phase 01 is the first real gameplay slice for MABS. It adds granted ability storage, activation requests by gameplay tag, server-authoritative validation and commit, and user-facing documentation for the new flow.

## Why it exists

Phase 00 established the architecture. Phase 01 proves that architecture can now drive a real multiplayer-safe path from data asset to activation request.

## Added in this phase

### Source

* `UMABSAbilityDefinition` now carries a display name and activation policy placeholder
* `FMABSAbilitySpec` now stores ability tag, runtime state, and last activation result
* `UMABSAbilityComponent` now grants abilities, blocks and unblocks runtime state, routes client activation requests through a server RPC path, and mirrors authoritative debug events back to the owning client
* granted ability specs now replicate

### Behavior

* ability grants reject null, invalid, and duplicate definitions
* `TryActivateAbilityByTag` succeeds directly on authority
* remote clients send activation requests to the server
* the server accepts, rejects, or commits the request
* successful commits mark the ability active briefly, then reset to idle
* structured debug events record request start, send, accept, reject, and commit

## How to use it

1. Add `UMABSAbilityComponent` to the owning actor.
2. Create a `UMABSAbilityDefinition` with a valid ability tag.
3. Grant the ability on the server with `GrantAbility`.
4. Call `TryActivateAbilityByTag`.
5. Inspect debug events or the replicated granted ability specs.

## Example

Example Phase 01 flow:

1. Create `DA_TestFireball` with tag `Test.Ability.Fireball`.
2. Add `UMABSAbilityComponent` to the player character.
3. On server `BeginPlay`, call `GrantAbility(DA_TestFireball)`.
4. On input, call `TryActivateAbilityByTag(Test.Ability.Fireball)`.
5. In standalone, expect `RequestAccepted` and `CommitSucceeded`.
6. In multiplayer, expect remote clients to see `RequestSentToServer` before receiving the server result.

## Not included in this phase

* cooldown execution
* resource costs
* targeting logic
* gameplay effects
* prediction
* advanced replicated runtime inspection UI

## Test checklist

Singleplayer:

* grant a valid ability
* activate a granted ability
* verify invalid and ungranted tags fail cleanly
* verify debug events describe the result

Listen server:

* host activation succeeds locally on authority
* remote client activation routes through the RPC path
* remote client receives the authoritative result

Dedicated server:

* dedicated server target remains present
* remote client requests are processed on the server
* there are no client-only activation shortcuts
