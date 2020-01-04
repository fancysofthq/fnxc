# Generics

```onyx
primitive Slice(Type T, Size N : SizeLiteral)
end

struct Range(T)
  def sum
    # ...
  end

  macro @specialized(_, t : Number)
    redef sum
      start + end
    end
  end
end

class Matrix(A : IntegerLiteral, B : IntegerLiteral)
  macro @specialized when A == B
    # ...
  end
end

macro @nil?(object) when object.class?
  &object.nil?
end

# Non-classes can't be nil (except void)
macro @nil?(object)
  false
end
```

## Variance

```onyx
class Plant; end
class Fruit; end
class Orange; end

module Enumerable(out T)
end

foo = Array(Fruit).new
foo << Orange.new
foo << Plant.new # Panic!

Array(Orange).new as Enumerable(Fruit) # Ok
Array(Fruit).new as Enumerable(Orange) # Panic!

class Comparer(in T)
end

bar = Comparer(Fruit).new
bar.compare(Orange.new)
bar.compare(Plant.new) # Panic!

Comparer(Orange).new as Comparer(Fruit) # Panic!
Comparer(Fruit).new as Comparer(Orange) # Ok

def fill(dest : Array(in T), value : T) forall T
  dest << value
end

fill(Array(Fruit).new, Orange.new) # Ok
fill(Array(Orange).new, Fruit.new) # Panic!

def copy(from : Array(out T), to : Array(T)) forall T
  to << from.first
end

copy(Array(Orange).new, Array(Fruit).new) # Ok
copy(Array(Fruit).new, Array(Orange).new) # Panic!

# In and out is only acceptable in type declarations
# and in generic arguments to types.

def foo(x : in T) # Panic!
```
