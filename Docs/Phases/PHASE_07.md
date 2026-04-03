# PHASE_07 - Basic Combos, AoE, and DOT/HOT

## What shipped

Phase 7 adds the first broader combat layer on top of the earlier activation, delivery, timing, cost, cooldown, and presentation foundation.

The implemented Phase 7 feature set is:

* basic melee combo chaining
* optional combo input buffering
* optional AoE target resolution
* sphere, box, and capsule AoE shapes
* basic DOT support
* basic HOT support
* timer-driven periodic ticking with refresh-on-reapply
* combo, AoE, and periodic debug events

## Why it matters

Earlier phases made MABS functional and multiplayer-safe, but still mostly single-target.

Phase 7 makes the framework feel more practical for common combat setups:

* melee attacks can chain into simple authored follow-ups
* impacts can affect more than one valid target
* abilities can apply repeated damage or repeated healing over time

It adds that breadth without introducing a large effect graph or stacking framework.

## What changed

### `MABSCore`

* added grouped `Combo`, `AoE`, and `PeriodicEffect` authored fields on `UMABSAbilityDefinition`
* added shared combo, AoE, and periodic enums and structs
* expanded `FMABSAbilitySpec` with combo-window and queued-follow-up runtime fields
* added `FMABSActivePeriodicEffect` for readable runtime periodic state

### `MABSGameplay`

* `UMABSAbilityComponent` now opens and closes combo windows using timers
* combo follow-up requests can queue against the currently active melee ability
* queued combo follow-ups auto-activate when the current ability completes recovery
* delivery can expand into AoE target gathering after the primary impact context resolves
* instant effects and periodic effects now share a multi-target application path
* periodic effects are stored and ticked authoritatively using timers
* projectile impacts can also drive AoE and periodic outcomes

### `MABSDebug`

* updated runtime summaries to show combo timing and queued follow-ups
* added readable formatting support for combo, AoE, and periodic activity through existing debug event output

## Implemented runtime path

Combo flow:

* melee ability starts normally
* authored combo window opens during the configured interval
* follow-up input may queue the authored `NextComboAbilityTag`
* queued follow-up starts when the current ability finishes recovery

AoE flow:

* delivery resolves the normal impact context
* if `AoE` is enabled, authority gathers overlaps from the authored shape
* gathered actors are validated and the valid set receives gameplay effects

Periodic flow:

* successful application can start a `DOT` or `HOT`
* authority stores a lightweight active periodic entry
* timers drive repeated ticks
* reapplying refreshes duration
* expiration removes the entry cleanly

## Debug visibility

Phase 7 adds or highlights:

* `ComboWindowStarted`
* `ComboWindowEnded`
* `ComboQueued`
* `ComboRejected`
* `AoEResolved`
* `AoETargetRejected`
* `PeriodicEffectApplied`
* `PeriodicEffectRefreshed`
* `PeriodicEffectTick`
* `PeriodicEffectExpired`

Earlier timing, delivery, and presentation events still remain.

## Module ownership

### `MABSCore`

Owns:

* shared combo / AoE / periodic enums and structs
* authored combo / AoE / periodic fields on `UMABSAbilityDefinition`
* combo and periodic runtime state structs

### `MABSGameplay`

Owns:

* combo runtime logic
* AoE target resolution
* periodic runtime entries and timers
* integration with delivery and gameplay effect flow

### `MABSDebug`

Owns:

* display and formatting of combo / AoE / periodic runtime visibility

## Documentation updated

Phase 7 updates or adds:

* `Docs/MABS_Abilities.md`
* `Docs/MABS_API.md`
* `Docs/MABS_Debugging.md`
* `Docs/MABS_Delivery.md`
* `Docs/MABS_Effects.md`
* `Docs/MABS_Modules.md`
* `Docs/MABS_Multiplayer.md`
* `Docs/MABS_Overview.md`
* `Docs/MABS_Combos.md`
* `Docs/MABS_AOE.md`
* `Docs/MABS_PeriodicEffects.md`

## Verification

Code compile verification completed with:

* `Build.bat MABSEditor Win64 Development -Project=...`

Recommended runtime validation:

* singleplayer 3-step melee combo chain
* combo rejection outside the authored window
* AoE ability affecting multiple valid actors
* DOT tick, refresh, and expiration behavior
* HOT tick, refresh, and expiration behavior
* listen-server combo and AoE authority behavior
* dedicated-server periodic ticking authority behavior
