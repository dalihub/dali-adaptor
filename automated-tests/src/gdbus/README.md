# GDBus client regression tests

These standalone tests need a C++17 compiler, CMake, pkg-config, GIO development
headers, and `dbus-daemon`. They use a private `GTestDBus` bus and do not require a
DALi build or a connected device.

From the repository root:

```sh
cmake -S automated-tests/src/gdbus -B /tmp/dali-gdbus-tests
cmake --build /tmp/dali-gdbus-tests
ctest --test-dir /tmp/dali-gdbus-tests --output-on-failure
```

The suite checks that proxy construction does not activate a service or load
properties, and covers late service startup, property notifications, owner
replacement, timeout recovery, stale replies, invalidation, and listener cleanup.
Multiple properties share one name watch and one asynchronous `GetAll` for initial
state and owner replacement; tests also check that this produces one activation
request and that a concurrent signal does not discard other properties' values.
Failed activation is retried asynchronously when a bus name is initially
unavailable. Cleanup is also tested when a callback destroys the client during
initial-read or signal dispatch.
