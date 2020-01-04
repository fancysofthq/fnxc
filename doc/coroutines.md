# Coroutines

Freestanding coroutines in the Onyx compiler are implemented using the LLVM coroutines. Due to the fact that LLVM coroutines may require heap allocations to store their frames, the feature is considered non-freestanding.

However, inline generators (e.g. `ary.map -> & * 2`) do not rely on LLVM coroutines, thus they are usable in the freestanding compilation mode.

```onyx
# Freestanding coroutines are not allowed
# in the freestanding mode (no pun intended)
const g = ary.each
until g.done? { @p g.resolve }

# Inline coroutines are allowed in the freestanding mode
ary.each -> @p(g.resolve)
```
