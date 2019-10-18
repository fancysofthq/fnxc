# Variables

```onyx
class Foo
  var bar : Int32

  static var bar # Panic! Already declared variable named `bar`
  static var baz : Int32

  def foo
    puts(bar)
    puts(baz)
    qux()
  end

  static def foo # Panic! Already declared method named `bar`

  static def qux
    puts(bar) # Panic!
    puts(baz)
  end
end

Foo.bar # Panic!
Foo.baz
Foo.foo() # Panic!
Foo.qux()

var foo = Foo.new()
foo.bar
foo.baz
foo.foo()
foo.qux()

class Bar
  private var foo   # Accessible to self only
  protected var foo # Accessible to self and things under same namespace
end
```
