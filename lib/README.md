# Libraries

This directory contains third-party libraries used during the development of the translator, in different languages.

## SQLite3 extensions

Currently the translator requries SQLite3 basic extensions source code to be present at `lib/cpp/sqlite3/ext`, e.g. `lib/cpp/sqlite3/ext/meta/regexp.c`.

The way a developer obtains these extensions is undefined, and the `lib/cpp/sqlite3` directory must be excluded from the repository index.
