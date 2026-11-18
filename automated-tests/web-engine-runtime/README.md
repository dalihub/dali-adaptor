# Web engine runtime regression tests

Install the changed adaptor before configuring these tests, using its pkg-config
and library paths:

```sh
cmake -S automated-tests/web-engine-runtime -B build/runtime-tests
cmake --build build/runtime-tests -j6
ctest --test-dir build/runtime-tests --output-on-failure
```

Tests run in separate processes against the installed adaptor and two independent
fixture plugins. The alias matrix covers environment selection and default
symlinks for both Chromium and LWE, in both creation orders. It exercises legacy
`Dali::WebEngine::New()` alongside explicit selection, profile/view library
identity, retained startup arguments, one-time initialization and rejection of
the other backend after initialization. No native engine is executed.

`test-plugin.h` implements the plugin interface for construction/destruction;
unused virtual calls throw so accidental fixture behavior does not pass silently.
Assertions remain enabled in Release builds.
