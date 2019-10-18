# Blocks

## Generators

If you see a block of code passed as the argument to a call within `do; end` or `{}`; that means it's yielded either after being captured in a generator; or rendered as-is by a macro. Therefore, `return`, `break` and `next` statements are allowed within such blocks, and they could affect the outer scope.

```onyx
def has_even?(enumerable)
  enumerable.each do
    return true if &.even?
  end

  return false
end
```

Capturing blocks happens with the last argument of a function definition being named `&`. Such functions are called *generators*. `yield` statements are allowed exclusively within generators.

```onyx
module Indexable(T)
  # Is a generator.
  def each(& : T ~>) : Void
    var i = 0

    while i < size
      yield self[i] # Inlines `self[i]` into the captured block
      i += 1
    end
  end
end
```

If a generator called with an inline block, then no real call happens, as the whole code gets inlined. The `has_even?` example above expands to:

```onyx
def has_even?(enumerable)
  var i = 0

  while i < enumerable.size
    return true if enumerable[i].even?
    i += 1
  end

  return false
end
```

Calling a generator with no block is allowed. In this case, a `Generator` instance is returned as a coroutine. To resume the coroutine execution, call `resume` on it. Note that in this case inlining is optional and depends on the LLVM optimizer.

```onyx
gen = {1, 2}.each
puts typeof gen # Generator(Int32)

loop do
  # `more` will be `false` if there are
  # no more `yield`s left in the generator
  more = resume gen do
    puts &
  end

  break unless more
end
```

You can call methods on a `Generator` instance, for example, `indexed`. Such calls are usually generators themselves.

```onyx
{1, 2}.each.indexed do |element, index|
  puts "#{index} = #{element}"
end
```

`do; end` blocks do not allow calls on them, while `{}` do.

```onyx
# Incorrect syntax
%w(fOo bAr).map! do
  &.downcase
end.map! do
  &.capitalize
end

# Ok
%w(fOo bAr).map! { &.downcase }.map! { &.capitalize }
```

Generators are always volatile, as the content could have changed in async context before the next resume.

## Procs

Procs able to enclose outer variables, so they're accessible within the proc. However, closured variables are passed by value, thus it's only possible to mutate such variable if it's a class instance. This rule applies to all enclosing blocks.
