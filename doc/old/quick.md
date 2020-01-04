# Onyx differences from Crystal

## Syntax

### Entry point

Onyx programs require the `main` function to be defined.

```onyx
def main
  `puts("Hello world!")
end
```

Note that the `main` function is implicitly exported. If you want to make use of the arguments passed to the program, multiple overloads are available:

```onyx
def main(args : Array(String))
end

# Or the longer form
export def main(argc : `int, argv : `char*[]`)
  args = Args.parse(argc, argv) # Array(String)
end
```

### Variables

Variables are declared using the `var` keyword. It's a error to try using an undeclared variable. It's also a error to try redeclaring a variable.

```onyx
var x = 42
x += 1

var x = "foo" # Panic! `x` already declared
x = "foo"     # Ok, the compiler would infer the type as `Int32 | String`

y = 43 # Panic! `y` is not declared
```

#### Properties

Objects have their properties defined using the same `var` keyword. These vars are freely accessible from the outside. A default initializer is generated for publicly accessible variables.

```onyx
class User
  var name : String
  var age : Int8
  var balance = 0.0

  def info
    `puts("My name is %s, my age is %d", name, age)
  end
end

var user = User.new(name: "John", age: 18)
user.name = "Jake"

`puts(user.name)
```

Sometimes you may want to limit the accessibility of a variable. To do so, you may use visibility modifiers, such as `private` and `protected`. It's also possible to declare a variable using the `getter` keyword, so it would only be publicly gettable, but not settable.

```onyx
class Post
  private var moderator_note : String?
  getter published : Bool

  def has_note?
    !moderator_note.nil?
  end
end

var post = Post.new

post.moderator_note = "foo" # Panic! Can not access `private` variable
post.published = true       # Panic! Can not modify `getter` variable
```

Visibility modifiers may be circumvented using explicit *access modifiers*. Basically, most designed API restrictions can be ignored by a developer taking the responsibility for themselves. For example:

```onyx
# A developer took the responsibility
# of accessing a private variable
post.(private)moderator_note = "foo"

# Ditto for getter
post.(getter)published = false
```

Object properties can be `static`, so they are not bound a particular instance. It's also possible to declare static variables within functions; the value of such variables would be preserved throughout successive calls.

An object property may have one of safety modifiers — `safe`, `volatile` or `unsafe`, with `volatile` being the default one.

### Constants

Constants are defined using the `const` keyword. Depending on a context, a compiler may or may not put a constant into the read-only area of a program. A program must never directly modify a constant.

```onyx
const x = 42
x = 43 # Panic! `x` is constant
```

Object-scope constants can be undefined; such undefined constants must be set in an initializer. The immutability can be circumvented explicitly using the `(const)` access modifier.

```onyx
class User
  const name : String
  private const db_table = "users"
end

var user = User.new(name: "John")

user.name = "Jake"        # Panic!
user.(const)name = "Jake" # Ok
```

Object instances can be declared constant as well. In that case, it's restricted to call mutating methods on them (variable mutation counts as a mutating method).

```onyx
class User
  var name : String
end

const user = User.new(name: "John")

user.name = "Jake"        # Panic!
user.(const)name = "John" # Ok
```

Constants are always implicitly `static` and `safe`.

### Functions

Neither functions nor
