# C

## Import

Including the `import` keyword into a file enables C lookups within that file. It includes functions, types and also C macros. This allows to mantion those entities without C-namespace aliases (i.e. `int` instead of `C::int`). Whitespaces can be treated as type continuation in such files (e.g. `unsigned long long`).

Note that `#define`s (a.k.a. C macros) are expanded as soon as the last `import` is included, therefore all `import`'s must be in the very top of a file.

Arguments passed to C functions are automatically cast to required C types where appropriate (e.g. `String` can be used as a `char*` argument). It can be said that casts are intuitive.

However, sometimes you have to cast manually because of possible data loss. For example, `Int64` can not be autocast to `long`, because `long` can be either 32 or 64 bytes long depending on the target platform.

```c
void* pq_connect(char* host, long long port) {}
```

```onyx
import "pq.h"

# `pq_connect` and `long long` are looked up in C if missing in Onyx
&conn = pq_connect("localhost", 5432i64.cast(long long))

# It's still legal to use the C namespace
&conn = C.pq_connect("localhost".char_pointer.as(Pointer(C::char)), 5432)

puts typeof(&conn) # => Pointer(Void)
```

## Export

### Exporting functions

Marking a function as `export`'ed requires its arguments and return type to be valid C types. It also enables well-known C types for the declaration even if there is no any `import`s.

Only top-level functions can be exported.

Such functions are still callable from inside Onyx programs, but they are treated as pure C functions, which affects arguments passing etc.

```onyx
export def foo(a : char*, b : int, ..vargs) : void
  puts typeof(a) # => Pointer(UInt8)
end

foo("bar", 42, 43, 44) # Ok
```

Exported functions can not be overloaded, even with non-exported ones. Thus it's illegal to have definitions with the same function name in any other (required) Onyx files.

### Exporting variables

Top-level variables can be `export`ed as well. Their declarations require type restriction (with C types lookup enabled). Note that the `const`, `atomic` and `volatile` keywords would require duplication in this case, as Onyx semantics is different from C.

```onyx
export const foo : unsigned volatile const int = 42
puts foo # => 42
```

### Exporting structs

`struct`s can also be `export`ed. Obviously, their methods would not be exported along with the type, and such structs can not be generic. Instance variables of exported structs are required to have C type restrictions and they cannot have default values.

```onyx
export struct Foo as foo
  @bar : int
  @baz : unsigned long

  # Onyx semantics here.
  def qux(a : String)
    puts typeof(@bar) # => Int32
  end
end

# Variable initializer enables autocasting to C.
# @baz is zero-initialized, as always.
Foo.new(@bar: 42)
```

## Extern

As in C, it's possible to declare entities as `extern` meaning that their definition is in another object file. If the definition is missing upon compilation, a linker error would be raised. Such entities are treated as if they were `include`d.

```onyx
# An external function.
extern def foo(int, long long)

# Should work
foo(42, 42)
```

```onyx
# An external variable.
extern const bar : char

puts typeof(bar) # => UInt8

# The result is unknown until linkage
puts bar
```

```onyx
# An external struct.
extern struct my_struct_t

# It depends on linkage if this succeeds
my_struct_t.new(42)
```
