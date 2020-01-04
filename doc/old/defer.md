# Defer

If current scope has static outer scope, then you can `defer` execution, which is performed as soon as the current scope exits. Therefore, `defer` is not available within freestanding procs.

Inline blocks can control the flow, e.g. with `return`. The controls would immediately exit the block; however, they are replaced with `void`, which may lead to compiler confusion and even coroutines corruption. In such cases `defer` would be useful; it would execute upon exiting the resumption, for example:

```onyx
var coro = [0, 1, 2, 3, 4, 5].map(: ~> Int)

def any_of_first_three_is_truthy?(coro)
  3.times do
    # Panic! Resume is expected to return `Int` by contract, got `Void`
    coro.resume do |value|
      if value
        return true
        #      ^ Here
      else
        convey value
      end
    end

    # Ok, true is returned once exited from the `resume` block
    coro.resume do |value|
      if value
        defer ~> return true
      end

      convey value
    end

    # return true (it's virtually here)
  end

  return false
end

pp any_of_first_three_is_truthy?(coro) # => true
```

Note that, unlike in Go, it's not possible to `defer` outside of a function.
