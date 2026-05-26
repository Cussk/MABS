# MABS Abilities

## What it is

This document explains the Phase 13 ability model.

The gameplay ability model itself is still the Phase 12 runtime:

* authored `UMABSAbilityDefinition`
* optional grouped `UMABSAbilitySet`
* granted runtime specs on `UMABSAbilityComponent`
* optional combo, AoE, periodic, montage, and presentation data
* optional custom delivery handlers

Phase 13 adds one sample-facing authoring layer on top:

* `UMABSDemoDisplayConfig`

That sample asset does not change gameplay execution. It only tells the demo HUD how to label and describe sample abilities.

## Why it exists

The gameplay asset should answer what an ability does.

The sample display asset should answer how the sample scene explains it.

Keeping those two concerns separate avoids polluting core authored ability data with sample-map-only labels, input hints, and onboarding text.

## Ability definitions

`UMABSAbilityDefinition` remains the authored single-ability asset in `MABSCore`.

Current authored fields still cover:

* identity and activation
* delivery mode and optional `DeliveryHandlerClass`
* target intent and gameplay outcomes
* timing
* cost and cooldown
* combo, AoE, and periodic data
* montage and presentation groups

Phase 13 does not add new gameplay fields to `UMABSAbilityDefinition`.

## Authoring validation

Phase 12 validation still applies.

The validator continues to catch:

* invalid `DeliveryHandlerClass`
* invalid built-in delivery authoring
* invalid projectile class authoring
* invalid combo, AoE, periodic, and basic gameplay-effect authoring

Phase 13 depends on that safety because the sample scene is meant to be the first thing a new user trusts.

## Ability sets

`UMABSAbilitySet` is still the grouped-grant asset in `MABSCore`.

Use it for:

* starter loadouts
* sample loadouts
* archetype bundles
* test bundles

Ability sets still reuse the normal grant path and do not introduce a second runtime state container.

## Sample display authoring

`UMABSDemoDisplayConfig` is a project-side data asset for the Phase 13 demo HUD.

It currently stores:

* overlay title and subtitle
* hotbar ordering entries by `AbilityTag`
* input labels for the readable sample hotbar
* feature-summary text for each sample ability
* help panel text
* validation panel text

This asset is for onboarding only. It does not drive authority, replication, delivery, or effects.

## Granted abilities

Granting still creates `FMABSAbilitySpec` entries on `UMABSAbilityComponent`.

Those runtime specs still hold:

* definition reference
* gameplay tag
* stable handle
* runtime state
* last activation result
* cooldown timestamps
* startup, delivery, and recovery timestamps
* combo window timing
* queued combo follow-up tag

The Phase 13 demo HUD reads the existing debug summaries derived from that state. It does not replace it.

## Runtime state versus sample display data

Authored ability data answers:

* what an ability is
* how it delivers
* what effect it applies
* when it delivers
* whether it combos, uses AoE, or starts a periodic effect

Runtime state answers:

* which abilities are granted
* whether an ability is idle, startup, active, recovery, or blocked
* cooldown and combo timing
* recent trace, delivery, and effect events
* active periodic effects

Sample display data answers:

* what label the sample hotbar should show
* what input label the sample scene should teach
* what short feature summary the sample HUD should call out

## How to use it

1. Author or reuse your `UMABSAbilityDefinition` assets.
2. Optionally group them in a `UMABSAbilitySet`.
3. Create a `UMABSDemoDisplayConfig` for the sample map.
4. Add one display entry per sample ability tag with an input label and short feature summary.
5. Assign the config on a Blueprint subclass of `AMABSDemoHUD`.
6. Grant abilities on authority and activate them normally.
7. Use the sample HUD to confirm ready/cooldown state, costs, combo windows, and feature callouts.

## Example

Example Phase 13 sample set:

* `Ability.Sample.SelfHeal`
* `Ability.Sample.RifleShot`
* `Ability.Sample.SwordSwing`
* `Ability.Sample.SwordFollowup`
* `Ability.Sample.Fireball`
* `Ability.Sample.KnockbackShot`

Example matching demo display entries:

* `InputLabel = "1"` and `FeatureSummary = "Direct self-heal, cost, cooldown"`
* `InputLabel = "2"` and `FeatureSummary = "Built-in hit trace, tracer, impact"`
* `InputLabel = "3"` and `FeatureSummary = "Built-in melee sweep, combo starter"`
* `InputLabel = "4"` and `FeatureSummary = "Follow-up combo window proof"`
* `InputLabel = "5"` and `FeatureSummary = "Projectile, AoE, periodic burn"`
* `InputLabel = "6"` and `FeatureSummary = "Custom knockback hit trace handler"`
