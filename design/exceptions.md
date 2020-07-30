# Exceptions

Exceptions in OnyxC are an implicit part of function return values. This allows to eliminate any runtime dependencies a classic exception handling implementation would have.

If a call may throw, then its real return value is a tagged union where switch is a thrown exception type ID; if it's zero, then the call did not throw and the union value is an expected, user-defined return value.

A `throws` function implicitly declares a `void* backtrace` argument. If the argument is NULL, then no write to it happens upon throwing an exception. Otherwise, current source-level location is written into the backtrace.

The `Backtrace` type is defined as `Stack<SourceLoc, N>`, whereas `SourceLoc` is `struct { path : String^, row : UInt16, col : UInt16 }`.
