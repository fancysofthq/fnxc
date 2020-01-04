# Functions

Functions are declared using the `def` keyword. A function name may consist either exclusively of `a-zA-Zα-ωΑ-Ω0-9_` symbols (i.e. latin and greek letters, digits and underscore), or exclusively of other Unicode codepoints. In the latter case, a function is allowed to be used as either an unary or binary operator.

There are some exceptions, though. `!`, `?` and `=` symbols are reserved and can only be placed in the end of non-operators (e.g. `def empty?`, `def map!` or `def name = (value)`). You can't use `#$@;:.,/\`, brackets or quotes for a function name. Note that in the strict compilation mode, non-exported and non-operator Onyx functions are required to begin with a lowercase letter or an underscore. In all cases, no function name can begin with a digit.

Functions of objects (i.e. of modules, primitives, structs and classes) are called *methods*. Methods can be declared as `static`, which means that they would be called on the type itself instead of on its instances. If a method is static, it can then be used as an unary operator. Otherwise, it may only be used as a binary operator. Note that it is prohibited to have instance and static methods with the same name. Furthermore, method names can't clash with object variables and constants.

A method can be marked `const`, which disallows its body to modify the instance (i.e. call non-const methods on it). `const` implies `threadsafe`. You can make a method explicitly `volatile const`, though.

Global functions and functions declared on namespaces can not be `static`, and they can only act as unary operators. Furthermore, they can't be `const` themselves. But it's possible to declare certain arguments `const` (which also applies to method arguments declaration), so it would be prohibited to call mutating methods on them or pass them as non-const arguments to inner calls.

Global and namespace function names may also not interfere with same-scope variable and constant names.

A function arguments may or may not have restriction declared, as well as the return type. However, explicit restrictions are required in the strict compilation mode.

It is possible to declare variable arguments, or *vargs*. If not restricted, every vargs types combination codegenerates a new, distinct function. To make the combination accessible in macros, use the `forall` syntax. This also applies to simple argument types.

## Index methods

There are special *index* methods, which have an unique declaration semantics. Note that the index methods can't be static, as the syntax is reserved for containers declaration.

```onyx
class Array(T)
  # Called on `ary[0]`.
  def [index : Size] : T
  end

  # Called on `ary[0] = 42`.
  def [index : Size] = (value : T) : T
  end
end

primitive Slice(T, N)
  # Called on `slice{0}`.
  def {index : Size} : T
  end

  # Called on `slice{0} = 42`.
  def {index : Size} = (value : T) : T
  end
end
```

## Calls

For argless calls, you can't know in advance whether is it a variable access or a real call, as parenthesis are't required. Single argument calls are allowed to not to have parens as well. Calls with arity more than one are **always required** to have wrapping parenthesis, though.

```onyx
def foo
  return 42 # return is an instruction, its parens thus are optional
end

foo   # Ok
foo() # Also ok

def bar(baz)
  return baz + 42
end

bar 17  # Ok
bar(17) # Still ok

def qux(a, b)
  return a + b
end

qux 1, 2  # Panic! Calls with arity > 1 require parenthesis
qux(1, 2) # Ok
```

Note that in the strict compilation mode, parens are mandatory for **any** call.

### Call modifiers

Calls may be modified with a space-separated list of predefined modifiers wrapped in parenthesis. Modifiers include LLVM Fast-Math flags (`nnan`, `ninf`, `nsz`, `arcp`, `contract`, `afn`, `reassoc` and `fast`) and LLVM function modifiers (`musttail`, `notail`, `cold`, `hot` etc.).

```onyx
var x = 42.0.(hot nnan ninf nsz)add(1)
var x = 42.0 (hot nnan ninf nsz)+ 1
```

## Blocks of code as arguments

If callee is a [generator](/blocks.md), then a block of code would be inlined using the `do; end` syntax. You can also pass a proc to a generator (e.g. `foo ~> bar`), and it would have a chance to be inlined as a generator. Otherwise you must use the procs syntax (`~>`) and pass it as a fully qualified argument. However, proc argument restriction can be delegated to the callee. For example:

```onyx
# Is a code generator
def foo(Float64 ~> Float64)
  yield 42
end

# No chance to be turnend into a runtime proc call
foo do
  & * 2
end

# A proc likely to be inlined
foo ~> & * 2

# A simple function accepting proc
def bar(proc : Float64 ~> Float64)
  proc.call(42)
end

# Panic! Bar is not a generator
bar do
  & * 2
end

# Ok. The proc still has a chance to be inlined,
# under certain circumstances
bar ~> & * 2
```
