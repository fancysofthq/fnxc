# Classes

Classes are tried to be moved to heap as late as possible. `@stackalloc`'ed instances are automatically heapified upon exiting the scope if their `@refcount` is greater than zero.

All class intrinsics are **atomic** and have proper orderings set.

  * `@refcount` to get actual references count for given class instance in runtime. For example:

  ```onyx
  var foo = Foo.new
  puts(@refcount(foo)) # => 0

  var bar = Bar.new(foo)
  puts(@refcount(foo)) # => 1
  ```

  * `@dearcify` and `@arcify` allow to switch ARC'ing for given class instance. Dearcifying is useful upon passing an instance to some external function. A dearcified instance would not be freed even its references counter reached zero. Note that despite of state, the references counting itself continues, still affecting the runtime performance. `@arcify` turns ARC on and immediately frees the instance if its references counter is zero. Use `@arcified?` to check if ARC is currently enabled for this instance.

  ```onyx
  var foo : Foo? = Foo.new

  @dearcify(foo)
  puts(@arcified?(foo)) # => false

  foo = nil # The instance now has zero references, but it's not removed yet

  # ...

  @arcify(foo) # Immediately frees the instance if its @refcount is zero
  ```

  * `@heapify` to move a class instance to the heap, if it's not already there. Returns the heap memory pointer if it's just moved, and `false` otherwise. `@heapified?` returns `true` in runtime if instance is currently in heap. Note that there is no way to put the instance back into the stack.

  ```onyx
  if !@heapified?(foo)
    const ptr = @heapify(foo)
    puts("Heapified foo to #{ptr}")
  end
  ```
