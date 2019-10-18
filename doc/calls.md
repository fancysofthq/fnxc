# Functions

Functions are declared using the `def` keyword. A function name may consist either exclusively of `a-zA-Zα-ωΑ-Ω0-9_` symbols (i.e. latin and greek letters, digits and underscore), or exclusively of other Unicode codepoints. In the latter case, a function is allowed to be used as either an unary or binary operator.

There are some exceptions, though. `!`, `?` and `=` symbols are reserved and can only be placed in the end of non-operators (e.g. `def empty?`, `def map!` or `def name = (value)`). You can't use `#$@;:.,/\`, brackets or quotes for a function name. Note that in the strict compilation mode, non-exported and non-operator Onyx functions are required to begin with a lowercase letter or an underscore. In all cases, no function name can begin with a digit.

Functions of objects (i.e. of modules, primitives, structs and classes) are called *methods*. Methods can be declared as `static`, which means that they would be called on the type itself instead of on its instances. If a method is static, it can then be used as an unary operator. Otherwise, it may only be used as a binary operator. Note that it is prohibited to have instance and static methods with the same name. Furthermore, method names can't clash with object variables and constants.

A method can be marked `const`, which disallows its body to modify the instance (i.e. call non-const methods on it). `const` implies `threadsafe`. You can make a method explicitly `volatile const`, though.

Global functions and functions declared on namespaces can not be `static`, and they can only act as unary operators. Furthermore, they can't be `const` themselves. But it's possible to declare certain arguments `const` (which also applies to method arguments declaration), so it would be prohibited to call mutating methods on them or pass them as non-const arguments to inner calls.

Global and namespace function names may also not interfere with same-scope variable and constant names.

A function arguments may or may not have restriction declared, as well as the return type. However, explicit restrictions are required in the strict compilation mode.

It is possible to declare variable arguments, or *vargs*. If not restricted, every vargs types combination codegenerates a new, distinct function. To make the combination accessible in macros, use the `forall` syntax. This also applies to simple argument types.

## Indexable

There is a special *indexable* methods syntax, which has an unique declaration semantics.

```onyx
class Foo
  # Called on `foo[0]`.
  def [index : T] : U
  end

  # Called on `foo[0] = 42`.
  def [index : T] = (value : U) : U
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

## Blocks of code as arguments

If callee is a [generator](/blocks.md), then a block of code can be inlined using the `do; end` and `{}` syntaxes. Otherwise you must use the procs syntax (`~>`) and pass it as a fully qualified argument. However, proc argument restriction can be delegated to the callee. For example:

```onyx
# Is a code generator
def foo(Float64 ~> Float64)
  yield 42
end

foo do
  & * 2
end

# A simple function accepting proc
def bar(proc : Float64 ~> Float64)
  proc.call(42)
end

bar(~> do
  & * 2
end)
```

```
var new = [1, 2, 3, 4].map.nth(2).index { |e, i| e * (i + 1) } == [2, 8, 0, 0]
var coro = [1, 2, 3, 4].map # Is a class? We should store the index
```
