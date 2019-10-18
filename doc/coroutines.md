# Coroutines

Coroutines are very much like C++ generators.

```onyx
class Array(T)
  # This is a coroutine, because it has the special last argument declaration,
  # also called "coroutine proc"; and the `yield` instruction.
  const def each(T ~>) : void
    var i = 0

    while i < size
      yield self[i]
      i += 1
    end
  end
end

[1, 2].each do |element|
  puts element # Would output 1 and 2
end
```

Unlike common functions, coroutines marked `const` are not implicitly `threadsafe`; that's because a coroutine may be invoked in different times. You must explicitly declare a coroutine threadsafe if you're sure it wouldn't blow in a multi-threaded environment.

A coroutine call without a block returns a `Coroutine` class instance. The instance store current state and can be resumed using the `#resume` method. You can check if coroutine is done with the `#done?` method. To get the result, call `#result`. Note that you may only retrieve the result when the coroutine is done; a error would be raised otherwise.

The `Coroutine` class contains a number of useful methods like `nth` and `indexed`. They may be called on a coroutine instance to extend its functionality.

```onyx
class Array(T)
  const def map(T ~> U) : self(U) forall U
    var i = 0
    var new = self(U).new(capacity)

    while i < size
      new << yield self[i]
      i += 1
    end

    return new
  end
end

[1, 2].map { & * 2 } == [2, 4]

# Note that we must have U known when indirectly coroutining.
# That's why a special syntax is applied here.
[1, 2].map.indexed do |element, index|
  element * (index + 1)
end == [1, 4]

var coro = [1, 2].map

loop do
  # `#resume` is a simple function accepting a proc, it's not a coroutine itself
  var continue? = coro.resume ~> { & * 3 }

  if !continue?
    puts "Result: #{coro.result}" # => [3, 6]
    break
  end
end
```
