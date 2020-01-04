Onyx has atomic intrinsics, most of them accept integer arguments only. Some intrinsics may also accept floating point and/or pointer arguments. All atomic intrinsics are considered **threadsafe** "calls".

An atomic intrinsic call returns the previous value of its argument. In all cases, the argument bitsize must be a power of two with minimum size of 8 bits and maximum size determined by the target-specific size limit (i.e. 64 bits for x86_64).

All atomic intrinsics have sequentially consistent ordering and no synchronisation scope by default. However, you can modify its `ordering` and `scope` passing them as the last arguments to an atomic call intrinsic. For example, `@atomic.add(i, 42, :acquire, :thread)` or `@atomic.get(x, scope: "custom")`.

  * `@atomic.get(variable : T)` atomically loads a *variable* from memory. In ordered context, it has absolutely the same semantics as simply accessing the variable. However, in `unordered` context, ordering other than sequentially consistent may lead to advanced optimization technics applied by the compiler. The `@atomic.get` intrinsics accepts integer, floating point and pointer arguments.

  * `@atomic.set(variable : T, value : T)` atomically sets a *variable* to the specified *value*. Again, its true power is only unraveled in `unordered` context. The `@atomic.set` intrinsic accepts integer and floating point arguments only.

  * `@atomic.cmpset(variable : T, compared : T, new : T)` atomically compares the *variable* with the *compared* value, and if they're equal, sets the value to *new*. For example, `@atomic.cmpset(i, 42, 43)` would set *i* to `43` iff its current value is `42`. This intrinsic accepts integer and pointer arguments only.

  * `@atomic.add(variable : T, value : T)` and `@atomic.sub` perform atomic addition and substraction. These intrinsics accept both integer and floating point arguments. Integer variants may raise in case of arithmetic overflow; to avoid that, use wrapped (`@atomic.wadd` and `@atomic.wsub`) or saturated (`@atomic.sadd` and `@atomic.ssub`) variants. Note that `@atomic.wadd` and `@atomic.wsub` expand precisely into a single LLVM intrinsic, while others perform runtime checks.

  * `@atomic.and`, `@atomic.nand`, `@atomic.or` and `@atomic.xor` intrinsics represent atomic variants of the mathematical operations. They all have the same semantics like `<op>(variable : T, value : T)`, e.g. `@atomic.and(i, 1, :acquire)`. All these intrinsics accept integer arguments only.

  * `@atomic.max(variable : T, compared : T)` and its sibling `@atomic.min` set the value of the *variable* to the *compared* iff its either greater or less than the actual value. For example, `@atomic.max(i, 10)` would set *i* to `10` iff it's currently less than `10`. `@atomic.max` and `@atomic.min` intrinsics accept integer arguments only.

Consider the following example:

```onyx
var i = 42

async do
  i += 1 # Panic! Can't execute binary operation `+` in asynchronous context
end
```

It won't compile, as *any* operation (either unary or binary) is considered *volatile* in Onyx. To deal with it, you can wrap the code into the explicit `volatile` block, entering into commitments relating to thread-safety.

```onyx
async do
  volatile do
    i += 1 # The same non-atomic operation, which may eventually explode
  end
end
```

However, a better way would be to unleash the power of atomics! It makes the code a bit slower, but introduces better threadsafe guarantees.

```onyx
async do
  const old = @atomic.add(i, 1) # Atomically add 1 to i, returning the old value
  puts(old)                     # => 42
end
```
