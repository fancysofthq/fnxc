# Minix

Minix (*Mini Onyx*, *Macro Onyx*) is an interpreted programming language used for Onyx macros. It uses Virtual Machine to implement basic set of instructions. The language supports declaring custom functions. Minix a C-like strictly-typed language. Minix has a deliberatively limited functionality so it implements only what's really needed for metaprogramming.

## Example

```minix
# A global variable accessible from any file required thereafter
global int a = 42

# A local variable only accessible from the current file;
# It shadows its global sibling
int a = 42

# A local function.
string foo(string a, string b)
  return a + b
end

# A global function.
global void bar()
  # Interpolation is not supported by Minix,
  # use printf-like formatting instead.
  # All function calls require parenthesis in Minix
  print("Current clock is %d", clock())
end
```

```onyx
{% include "./foo" # Inserts the contents of `./foo.mnx` %}

class Integer(Bytesize N : SizeLiteral)
  macro @specialized(derivative)
    {%
      # Any identifier starting with uppercase letter is considered an AST node.
      # Passing an argument to `print()`, `emit()` and `panic()` built-in methods
      # calls the `arg.dump()` method by default, which dumps the argument
      # into a string in the most expected way
      print(N) # => 32

      int limit = 8388607

      # Some node types have other expected methods, like this `>`
      if N > limit
        # Would halt the compilation immediately.
        # ANSI control sequences are supported in strings; they may be either
        # well-known names or explicit numeric codes
        panic("Integer bitsize must be %{red,bright}less than %d%{0}", limit)
      end

      print(N.inspect()) # => {type: SizeLiteral, value: 32, ...}
      print(derivative.inspect()) # => {type: Type, value: "Integer(32)", ...}

      # There are some context-defined variables...
      print($type) # => Integer(32)

      # And also global ones
      print($target.size) # => 64

      # You can declare custom structs...
      type &data(string binop, string method, int precedence)

      # And use them as variables
      &data temp = {binop: "-", precedence: 20, method: "sub"}
      print(temp.binop) # => -

      # You can also do anonymous structs.
      # Note that it is possible to re-declare a local variable
      &(int foo, int bar) temp = {42, 43}
      print(temp.foo) # => 42
    %}

    {% for data in [&data{"+", "add", 10}, &data{"*", "mul", 20}] %}
      % Precedence({% emit(data.precedence) %})
      def {{ data.binop # Same as "emit", but shorter }} (other : self)
        {{ data.method }}(other)
      end
    {% end %}
  end
end
```

## Built-in types

Minix has the following built-in types. All types occupy the **host** pointer size bytes in memory (e.g. 8 bytes in a 64-bit compiler). However, a `string` content is always a sequence of 1- to 4-bytes chars according UTF-8 encoding; the `string` itself is always a pointer, though.

  * `void` — the nothingness
  * `int` — an integer
  * `bool` — an boolean value, can only be either `true` or `false`
  * `string` — a null-terminated 4-bytes-chars UTF-8-encoded string
  * `regex` — a regular expression
  * `array` — a dynamic array similar to Onyx's. Must be declared as `[int] foo` or `[1, 2]`. Note that unions aren't supported in Minix
  * `hash` — a dynamic hashtable similar to Onyx's. The declaration should be as follows: `[int: string] bar` or `[foo: "bar", "42": "baz"]`. Quotes can be omitted for string keys
  * `file` — a file descriptor
  * `node` — an **immutable** AST node

`int`, `bool` and `file` are kept on stack unless declared `global`; and others are always in the heap memory. All global variables accesses are atomic, as they may be accessed from multiple workers.

`string` can be automatically cast to `regex`.

## Instructions

Only `return` and `include` instructions are supported by Minix.

Include is needed when you want to share a piece of Minix code across files, but don't want to declare global objects. If no file extension is provided, `.mnx` is then assumed. The code is inserted as-is and evaluated as if it was written by hand, except that it does not re-declare global variables and functions.

## Statements

Minix supports Onyx-like `case`, `if / unless`, `while` and `until` statements. Just like in Onyx, zero numbers and `false` are considered falsey values. Everything else is truthy.

There is also the lazy `for element, index in array` statement, which yields an *element* of the *array* one-by-one with optional *index*. For hashes, use the `for key, value, index in hash` syntax.

## Built-in functions

There are some built-in functions which map to their C11 implementations.

  * `void print(..)` simply outputs a string. It supports `printf`-style formatting. As a convenience, any non-string argument passed to it is called with the `.dump()` method, which returns the most relevant string describing the object itself (e.g. a number for a `SizeLiteral` node or a quoted type string for  an Onyx type).

  * `void panic(..)` halts the compilation immediately. The compiler displays the location of the `panic` call. Arguments semantics is the same as in `print`.

  * `void nodepanic(node node, ..)` panics at the declaration location of a node.

  * `void locpanic(string filepath, int line, int column, ..)` panics at the given location. See `$filepath`, `$line` and `$column`.

  * `void emit(..)` emits the string as-is. To emit a quoted string, use the `stringify` method. You can also use `{{ }}` as a shortcut.

  <!-- * `void free(arg)` frees the memory occupied by the argument. Freeing is not mandatory, but may be useful in long-running context, i.e. during a hot-reload session. Note that local variables and functions are automatically freed upon exiting the file. Calling `free` may raise a error if the variable is already freed, but it never segfaults -->

  * `int clock()` returns current raw CPU clock in nanoseconds (`10^-9` seconds).

  * `int timestamp()` returns the current system UTC timestamp.

### Strings

There are some string-related functions in Minix.

  <!-- * `string string:wrap(string wrapper)` wraps a string into *wrapper*. It is useful for emitting types, e.g. `Int32.dump().wrap("'") == "'foo'"`. -->

  * `bool match(string, regex)` matches a string to a regular expression. It can also be done as `s ~ /foo/`. The functionality is powered by the [tiny-regex-c](https://github.com/kokke/tiny-regex-c) library modified to work with UTF-8 characters.

### Files

Some built-in functions allow to work with files from Minix.

  * `file open(string filepath, string mode)` opens a file. Returns a falsey result if the file couldn't be opened.

  * `bool eof?(file)` checks if a file is currently at EOF.

  * `int read(file)` reads an UTF-8 character from a file. Returns zero for EOF.

  * `string readline(file)` reads a line from a file. Panics at EOF.

  * `[string] lines(file)` reads all the lines until EOF is met. It performs **lazy** read if used in the `for` statement.

  * `void write(file, int char)` writes an UTF-8 char into a file.

  * `void writeline(file, ..)` writes a string line into a file. The arguments semantics is the same as in `print`.

  * `void close(file)` closes a file. All files are closed automatically at compiler exit. It would be useful for the JIT mode, though.

### Math

There are some mathematic functions like `int abs(float x)` etc.

## Context variables

Some variables depend on the current context. Neither of them are mutable.

  * `&(int size, string arch, string os) $target` stores the target information
  * `hash(string: string) $env` stores environment variables
  * `string $filepath` stores current file path
  * `int $line` if used in a `macro` stores the callee site line. Otherwise it stores the last Onyx code met line
  * `int $column` is same as `$line`
  * `node $type`, `node $def` store current node references
