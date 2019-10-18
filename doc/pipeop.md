# Piping operator

The piping operator `>>` is somewhat similar to the `.tap` function, but it returns the evaluated result. It uses the anonymous block argument `&` to reference the piped value. By default, the value is passed as a single argument to the pipee.

```onyx
foo >> bar >> &.baz
bar(foo).baz

foo >> @try ~> puts(&)
@try(foo) ~> puts(&)

foo << "Hello, world!" >> &.upcase
foo << "Hello, world!".upcase
```
