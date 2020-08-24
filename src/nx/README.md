# Onyx source code

This directory contains implementaions of some parts of the built-in Onyx API.
Authors find it more convenient than relying on C++ code generation.

Obviosly, some things, such as lambda initialization, can not be expressed in Onyx.
Those are implemented elsewhere in C++ and marked `native`.

The built-in API is guaranteed to be the same for a translator build, therefore it is shipped in the form of precompiled Onyx binary source code.
