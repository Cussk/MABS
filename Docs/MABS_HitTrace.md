# MABS HitTrace

## What it is

`HitTrace` is the instant ranged delivery mode in MABS.

In Phase 11, the built-in hit trace behavior now lives in `UMABSHitTraceDeliveryHandler`.

## Why it exists

Many abilities need a practical rifle, beam, or instant-shot path without spawning a projectile.

Phase 11 also makes that path extensible without editing core runtime code.

## Authored fields

Use:

* `TargetType = Actor`
* `DeliveryMode = HitTrace`
* optional `DeliveryHandlerClass`
* `HitTraceDistance`
* `HitTraceRadius`
* `DeliveryPresentation.Cue`
* `DeliveryPresentation.HitTraceTracer`
* `ImpactPresentation.Cue`

`HitTraceRadius = 0` behaves like a line trace. A positive radius behaves like a sphere trace.

## Runtime behavior

On authority, the built-in hit trace handler:

1. enters delivery at the authored delivery time
2. triggers delivery presentation at the origin
3. traces from the authoritative aim direction
4. spawns tracer presentation from the trace start to the hit or final trace end if authored
5. resolves the primary hit actor
6. expands to AoE targets when authored
7. returns the final target set to the normal MABS effect / presentation / commit flow

## Validation

Phase 12 validates the common hit-trace authoring mistakes:

* `TargetType` must be `Actor`
* `HitTraceDistance` must be greater than zero
* if a custom handler class is set, it must be loadable, non-abstract, and derive from `UMABSDeliveryHandler`
* a handler derived from a different built-in mode-specific base is rejected as incompatible authoring

## How to extend it

Create a class deriving from `UMABSHitTraceDeliveryHandler` when one ability needs custom ranged delivery behavior.

Good uses:

* custom filtering
* chained hits
* knockback
* pull / draw-in
* target ordering changes

Blueprint subclasses can override `ModifyDeliveryResult(...)` and keep the normal built-in trace logic.

## Example

Example knockback rifle shot:

* `DeliveryMode = HitTrace`
* `DeliveryHandlerClass = UMABSExampleKnockbackHitTraceDeliveryHandler`
* built-in hit trace still finds the target
* the custom handler launches each resolved target away from the impact
