# Lua source code

This directory contains Lua code used in the macros.

The structure mirros the namespace organaization with tables in Lua API.
For example, the `nx/utils.lua` file contents goes into the `nx.utils` Lua table.

Prior to translation, the `lua` folder contents is evaluated by the translation context.
Therefore, it is reasonable to precompile the Lua code and ship the precompiled version with the translator, instead of raw Lua code.
