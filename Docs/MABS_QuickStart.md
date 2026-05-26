# MABS Quick Start

## What it is

This is the Phase 13 quickstart path.

The intended first-use flow is no longer "start from a blank actor and wire everything manually."

It is now:

* open the sample map
* trigger the sample abilities in order
* read the demo HUD
* inspect the sample assets after you already know what working behavior looks like

## Why it exists

The framework already had the runtime features. What it lacked was a fast first proof.

Phase 13 fixes that by making the sample map the default onboarding path.

## How to use it

### Step 1: Open the sample map

Open the Phase 13 sample map in the editor and play from the central spawn area.

The map should point you to the sample stations for:

* direct
* hit trace
* melee
* combo
* projectile / periodic / AoE
* custom delivery handler

### Step 2: Use the sample HUD path

Use `AMABSGameMode` or a derived sample game mode so the default HUD is `AMABSDemoHUD`.

Recommended setup:

* create a Blueprint subclass of `AMABSDemoHUD`
* assign a `UMABSDemoDisplayConfig`
* set that Blueprint HUD on the sample game mode if you want custom labels and help text

The technical debug harness is still available underneath this through `F2`.

### Step 3: Spawn with a sample-ready pawn

Your sample pawn should include:

* `UMABSAbilityComponent`
* granted sample abilities or one sample `UMABSAbilitySet`
* `AMABSCharacter` or a derived sample character if you want the replicated example health/resource HUD values

### Step 4: Trigger the stations in this order

Recommended order:

1. direct self-heal
2. hit trace rifle shot
3. melee sword swing
4. combo follow-up
5. projectile fireball
6. custom knockback handler

The demo HUD is built to make that order easy to read.

### Step 5: Use the built-in toggles

The sample controller now binds:

* `F1` = toggle the demo help panel
* `F2` = toggle the technical debug harness
* `F3` = toggle validation notes

### Step 6: Inspect the sample assets

After you have seen the map working, inspect:

* the sample `UMABSAbilityDefinition` assets
* the sample `UMABSAbilitySet`
* the sample `UMABSDemoDisplayConfig`
* `UMABSExampleKnockbackHitTraceDeliveryHandler`

### Step 7: Duplicate one sample ability and change it

Good first modification:

* duplicate the hit trace or projectile sample ability
* change its effect magnitude, cooldown, or presentation
* update the matching `UMABSDemoDisplayConfig` entry
* retest in the sample map

## Example setup

Minimal Phase 13 sample setup:

* sample game mode uses `AMABSDemoHUD`
* sample HUD Blueprint references one `UMABSDemoDisplayConfig`
* sample character derives from `AMABSCharacter`
* sample character owns `UMABSAbilityComponent`
* authority grants six sample abilities on spawn
* the sample config maps those six ability tags to the hotbar labels `1` through `6`

## Multiplayer note

Phase 13 does not create a separate sample replication model.

The sample HUD reads:

* existing `UMABSAbilityComponent` summaries and debug events
* replicated sample vitals from `AMABSCharacter`

That keeps the readable sample overlay correct on owning clients in standalone, listen server, and dedicated server play.

## Not included

This quickstart does not replace the lower-level setup path.

If you want to integrate MABS into a different project, you still:

* add `UMABSAbilityComponent`
* author `UMABSAbilityDefinition` assets
* grant on authority
* activate by gameplay tag

Phase 13 just makes the sample scene the recommended first proof.
