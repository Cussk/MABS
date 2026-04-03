# MABS Combos

## What it is

This document explains the basic Phase 7 combo model in MABS.

## Why it exists

Many melee-heavy games need a simple authored follow-up chain such as `Attack1 -> Attack2 -> Attack3`. Phase 7 adds that common case without building a large branching combo framework.

## Authored fields

Use the `Combo` group on a melee ability:

* `NextComboAbilityTag`
* `ComboWindowStart`
* `ComboWindowEnd`
* `bBufferComboInput`

Phase 7 combo support is intentionally melee-focused.

## Runtime behavior

On authority, MABS:

1. starts the current melee ability normally
2. opens the combo window during the authored interval
3. accepts or rejects requests for the authored `NextComboAbilityTag`
4. stores the queued follow-up on the current ability spec
5. auto-triggers the queued follow-up when the current ability finishes recovery

If buffering is disabled, the follow-up is accepted only when the current ability is already in recovery.

## How to use it

1. Grant every combo step as its own normal ability.
2. Author `Attack1` with `NextComboAbilityTag = Attack2`.
3. Author `Attack2` with `NextComboAbilityTag = Attack3`.
4. Set a recovery window that gives the follow-up room to start cleanly.
5. Press the next combo input during the authored combo window.

## Example

Example opener:

* `AbilityTag = Ability.Combat.Attack1`
* `DeliveryMode = Melee`
* `Combo.NextComboAbilityTag = Ability.Combat.Attack2`
* `Combo.ComboWindowStart = 0.18`
* `Combo.ComboWindowEnd = 0.42`
* `Combo.bBufferComboInput = true`

## Not included

Phase 7 does not add:

* branching combo trees
* section-graph montage logic
* fighting-game style cancel rules
