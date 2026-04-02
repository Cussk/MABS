# MABS Multiplayer

## What it is

This document explains how Phase 3 handles multiplayer authority for activation, cooldowns, costs, target resolution, instant effects, and runtime debug visibility.

## Why it exists

Cooldowns and costs are easy to get wrong if clients are allowed to decide them. MABS keeps the gameplay decision, cost spending, and cooldown start on the server while still giving the owning client enough visibility to understand why activation succeeded or failed.

## Server authority

The server owns gameplay approval and gameplay results.

That means:

* grants should be made on authority
* cooldown denial is decided on authority
* cost affordability and cost spending are decided on authority
* target resolution is decided on authority
* damage and heal application happen on authority

## Client behavior

When a remote client calls `TryActivateAbilityByTag`:

1. the local component records `RequestStarted`
2. the local component sends `ServerTryActivateAbilityByTag`
3. the local return value is `RequestSentToServer`
4. the server validates cooldown and cost, resolves the target, applies the effect, spends cost, and starts cooldown
5. the server mirrors the authoritative debug events back to the owning client

This keeps the final result authoritative while still giving the client visibility into what happened.

## Cooldowns and costs in multiplayer

### Cooldowns

Cooldown start happens only on authority.

The owning client receives the resulting replicated cooldown end time on the granted ability spec and any shared cooldown-group state on the component.

### Costs

Cost validation and cost spending happen only on authority through `IMABSCostReceiver`.

Clients do not authoritatively decide affordability.

## Runtime overlay in multiplayer

`AMABSDebugHUD` is still client-side runtime UI. It does not make gameplay decisions.

It reads:

* local recent debug events
* replicated granted ability runtime state, including cooldown end time
* the latest authoritative target trace snapshot for the local owner

## Example

Example multiplayer verification flow:

1. Start PIE with one listen server and one client.
2. Grant `Test.Ability.SelfHeal` on the server-owned player character.
3. Set `CooldownSeconds` and `ResourceCost` on the definition.
4. Press the bound activation input on the remote client.
5. Verify the client records `RequestSentToServer`.
6. Verify the server records `CostValidated`, `EffectApplied`, `CostSpent`, and `CooldownStarted`.
7. Press the input again immediately.
8. Verify the server records `CooldownRejected` and the client receives the same authoritative denial.

## Test checklist

Singleplayer:

* cooldown denial works
* cost denial works

Listen server:

* host cooldown behavior is correct
* host cost spending is authoritative and correct
* remote client receives correct cooldown/cost denial results

Dedicated server:

* cooldown and cost validation happen only on the server
* owning remote client receives authoritative result and debug info
* no editor-only dependency leaks into runtime or server build
