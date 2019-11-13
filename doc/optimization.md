```
class Foo
  var num : Number # -Os: An implicit union of all Number types
                   # -OP: Separate Foo for each case
                   # -O2: Depending on how many times Foo is specialized

  def initialize(self.num)
  end

  def sum(a : Number) # ditto
    num + a
  end
end

# Foo{num : Int16}#sum(a : Int16)
# Foo{Int16}#sum(Int16)
Foo.new(42i16).sum(43i16)
Foo.new(42f64).sum(43i16)
```
