# Macros

You can interact with the program's AST during compilation using *inline macros*. The interaction is wrapped in `{% %}` blocks and is written in Lua.

```onyx
def foo
  {% inspect(_def) %} # => {kind: "def", name: "foo", ...}
end
```

Macros evaluate as soon as they're met by the compiler, without additional iterations whatsoever. The Onyx compiler is built in such way that it builds the AST as soon as possible, just after tokens are read. The order of objects declaration thus matters. It implies the need to think of the program structure in a way that a macro is evaluated after the needed object is put into the AST.

```onyx
{% inspect(foo) %} # Panic! Unknown variable `foo`

var foo : Int32

{% inspect(foo) %} # => {kind: "var", name: "foo", type: "Int32", ...}
```

## Emitting the code

Macros allow to emit code directly into their site. There is a special Lua function `emit()` for that. Once a macro is run, its emitted code gets compiled. It's possible that a macro emits another macro; in such case, the second macro is evaluated and so on. In addition to `emit()`, there is a special syntax `{{ }}` to emit the evaluated code faster.

```onyx
var foo : Int{% emit("32") %} # Int32
var bar : Int{{ 32 }}         # ditto
```

## Deferred macros

If a macro resides within a function, it then wouldn't have detailed access to its arguments. To deal with it, define a deferred macro with `\` prepended, which would run on every function specialization.

```onyx
def sum(a, b)
  {%
    print(_def.name) -- => sum
    print(inspect(a)) -- => {kind: "arg", name: "a"}
  %}

  \{% print(inspect(a)) %} # Doesn't output anything yet

  a + b
end

sum(1, 2)     # => {kind: "var", type: "Number", ...}
sum(0.1, 0.2) # No output

sum("foo", "bar") # => {kind: "var", type: "String", ...}
```

## Intrinsics

To dry up common macros patterns, you can have `macro` declarations, which are called *intinsics*. Intrinsics begin with the `@` symbol. Everything what an intrinsic emits gets inserted into the caller site, wrapped in parenthesis (thus variables declared within are scoped). You may use `{% %}` and `{{ }}` (and also `\`) as you would normally do.

```onyx
# This is one of the built-in intrinsics
macro @loop(~> block)
  while true
    {{ block }}
  end
end

@loop ~> puts "yes"

# Would be expanded to:
(while true
  puts "yes"
end)
```

All intrinsic calls with arity more than one require parenthesis wrapping their arguments, in any compilation mode. Intrinsics evaluate each time they're called, thus full compile-time information can be obtained from the passed argument (i.e. `block.type.type` is `Block` in the example above). In fact, it's the only way to interact with the program's type system.

It's possible to scope intrinsics by defining them within namespaces or objects. In such cases, an intrinsic call must be prepended with the type, e.g. `Foo@bar`.

By convention, unsafe intrinsics (ones which may produce undefined behavior or crash the program) begin with `@@`, for example `@@cast`.

### Hooks

There is a number of pre-defined hook intrinsics which are called once something happens. These hooks should be implemented by the programmer.

  * `@derived(by)` is triggered once a containing object is derived *by* another object. Note that the hook is called from the derivative site. Also note that indirect derivations won't trigger the hook.

  ```onyx
  module Foo
    macro @derived(by)
      def foo
        puts {{ by.name.stringify }}
      end
    end
  end

  class Baz
    derive Foo # The method `foo` is defined now
  end

  Baz.new.foo # => Baz

  class Qux
    derive Baz # Won't trigger the hook
  end
  ```

  * `@specialized(site)` is triggered once the containing object is declared with an unique generics and derivative combination. It may happen either on derivation or upon a call from the code. In either case, the evaluation happens on the *site*. A `@specialized` hook may also be triggered **after** the `@derived` one.

  ```onyx
  class Array(T)
    macro @specialized(site)
      {% print(typeof(site) .. ", " .. typeof(T)) %}
    end
  end

  [1, 2] # => Array(Int32), Int32
  [3, 4] # No output here, because we already have had this combination
  [0.5]  # => Array(Float64), Float64

  class Int32Array
    derive Array(Int32) # => Int32Array, Int32
  end

  class StaticArray(T)
    derive Array(T) # No specialization happens here yet
  end

  StaticArray(Text).new # => StaticArray(Text), Text

  class SubArray(T)
    derive Array(T) # Nothing happens
  end

  class Int32SubArray
    derive SubArray(Int32) # => Int32SubArray, Int32
  end
  ```

### Built-in intrinsics

Onyx ships with a number of built-in intrinsics. One of them is the `@loop` which you've seen before. Another one is `@unreacheable`, which panics once the compiler reaches it.

#### Stack manipulation

Some intrinsics allow to manupulate the callstack.

  * `@alloca(T, quantity = 1) : T*` allocates a new object on the stack. In fact, it just moves the stack pointer, thus being very fast; furthermore, it's possible that the object is allocated directly in a register instead. The macro does not initialize the object, though; we say that its memory content is uninitialized, similar to the `malloc` call.

  ```onyx
  var &foo = @alloca(Foo)
  puts foo # Crash! Segmentation fault
  foo.initialize
  puts foo # Ok
  ```

  * `@stacksave : void*` allows to save the current stack pointer, and `@stackrestore(void*) : void` reverts the stack to that pointer. It comes in very handy when having lots of stack-allocating operations going on in a single function call; it allows to avoid  stack overflowing.

  ```onyx
  def foo
    var result = 0

    1000.times do
      var stackptr = @stacksave # Save the pointer to a variable

      # Each loop iterations would move the stack pointer further
      var &slice = @alloca(Slice(1000, Int32))

      # Do some things with the slice...

      @stackrestore(stackptr) # Restore the pointer,
                              # effectively cancelling the @alloca
    end

    puts result # This var is still accessible
  end
  ```

#### Memory manipulation

It's recommeded to use given intrinsics instead of their C variants due to better optimization opportunities:

  * `@memmove(from*, to*, size) : void` moves a block of memory of *size* bytes *from* pointer *to* another pointer; it allows overlapping. You don't need to free the source region afterwards

  * `@memcopy(from*, to*, size) : void` behaves similarly to `@memmove`, but it copies the memory instead, thus disallowing to overlap

  * `@memset(pointer*, value : Byte, size) : void` fills a block of memory with a particular byte value

#### Type information

Onyx instances **do not** store their type information, except unions. Also types are not first-class types themselves (i.e. `var x = Int32` is invalid). To interact with types, use intrinsics below:

  * `@is?(arg, T) : Bool` to check in runtime if its **union** argument *is* currently some type `T` or its derivative. The macro narrows the type and introduces optimization opportunities in case of unreacheable conditions.

  ```onyx
  var foo : Int32 | String = 42

  if @is?(foo, Int32)
    do_stuff_with_i32(foo)
  else
    # This branch wouldn't be compiled at all, because foo is always Int32
    @unreacheable
  end
  ```

  * `@as(arg, T) : T` to (up)cast its argument to another type `T`, which it derives from. Note that upcast instances lose their initial type information (i.e. declared variables). The compiler would panic in case of possibility of type mismatch, e.g:

  ```onyx
  class Parent
  end

  class Foo
    derive Parent
  end

  class Bar
  end

  var foo = rand(Foo.new, Bar.new)

  var x = @as(foo, Foo)    # Panic! Can't cast `Foo | Bar` to `Foo`
  var y = @as(foo, Parent) # Panic! Can't cast `Foo | Bar` to `Parent`

  if @is?(foo, Foo)
    var x = @as(foo, Foo)    # Ok, but useless
    var y = @as(foo, Parent) # Ok!
  else
    # Would still be compiled, because it's possible that foo is Bar
  end
  ```

  * `@valid?(expression) : BoolLiteral` to check if the *expression* would compile, e.g. `@valid?(foo)` or `@valid?(foo.baz(42, foo: s))`. The expression checked must be either a variable reference or a function call. The expression is not emitted into the compiled code. Note that the expression must be syntactically correct, otherwise it panics.

  * `@typeof(argument) : StringLiteral` macro compiles into string representation of the compile-time type of its *argument*. It may be used within other macros, e.g. `@is(foo, @typeof(foo))` (note that `@is` evaluates earlier than `@typeof` in this case).

  * `@sizeof(argument(.variable?)) : SizeLiteral` compiles into the *argument* (or its *variable*) size in **bytes**, for example:

  ```onyx
  struct Foo
    var i : Int32
  end

  puts @sizeof(Foo)   # => 4
  puts @sizeof(Foo.i) # => 4

  class Bar
    var i : Int32
  end

  puts @sizeof(Bar) # => 12 (8 bytes on 64-bit arch because of the class header)
  ```

  * `@bitsizeof` does the same but in **bits**.

  * `@offsetof(argument.variable) : SizeLiteral` compiles to the offset of an *argument*'s *variable* in **bytes**, for example:

  ```onyx
  struct Foo
    var i : Int32
  end

  puts @offsetof(Foo.i) # => 0

  class Bar
    var i : Int32
  end

  puts @offsetof(Bar.i) # => 8 (because of the class header)
  ```

  * `@bitoffsetof` does the same, but in **bits**.

  * `@pointerof(call) : void*` returns an address of the function mentioned, e.g. `@pointerof(foo.bar(42))`. Note that such addresses are not very usable, as their actual prototypes differ from what you see in the Onyx code.

#### Nilability-related intrinsics

  * `@nil?(argument) : Bool or BoolLiteral` checks in runtime if the *argument* is nil. It may also compile to a boolean literal if the compiler is sure about the permanent nilability state of the argument.

  * `@nonil!(argument : T) : T or raise` checks if the *argument* is `nil`, and if so, raises a `NilAssertionError`. It may also be eventually compiled to noop or instant raise.

  * `@try(argument : T, T ~> U) : U or nil` executes the block iff the *argument* is not currently `nil`, otherwise returning `nil`. It passes its argument as the only argument to the block, and the return value of the block is the return value of the macro. This macro may also be optimized out, turning either in bare `nil` or the raw block of code itself.

Some examples:

```onyx
var foo = nil
puts @nil?(foo) # => true

@nonil!(foo) # Error! Nil assertion failed (no runtime check happens,
             # because foo is known to be always nil)

foo >> @try ~> puts(& * 2) # It would work if foo wasn't nil
```

#### Class intrinsics

They are `@refcount`, `@dearcify`, `@arcify`. See [classes]().

#### Atomic intrinsics

They are `@atomic.get`, `@atomic.set`, `@atomic.cmpset`, `@atomic.add`, `@atomic.sub`, `@atomic.wadd`, `@atomic.wsub`, `@atomic.sadd`, `@atomic.ssub`, `@atomic.and`, `@atomic.nand`, `@atomic.or`, `@atomic.xor`, `@atomic.min`, `@atomic.max` and also `@fence`. See [atomics]().
