# Exceptions

Thanks to the Onyx language standard which doesn't have distinctive header and source files, and the fact that a program must be compiled as a whole (including standard libraries); it's possible for a compiler to determine which error may be raised upon a call.

This allows to implement a zero-cost cross-platform exceptions mechanism, which does not rely on any platform-specific techinques like [SEH](https://en.wikipedia.org/wiki/Microsoft-specific_exception_handling_mechanisms#Structured_Exception_Handling). Instead, if a function raises, it returns a "union" of values with a union switch, which allows to check at the callee site if an exception's been raised. Of course, it requires a CPU operation to check the result before proceeding, but it's much more lightweight than traditional stack unwinding. Moreover, the switch can be stored in a CPU register, which makes the check almost negligible.
