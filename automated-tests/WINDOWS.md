# DALi Adaptor Common UTC on Windows

The Windows lane builds and runs the existing adaptor UTC sources from
`automated-tests/src/dali-adaptor`. It does not maintain a second set of
Windows-specific testcase bodies.

The platform-specific part is limited to:

- `build.ps1` and `execute.ps1` for MSVC build, process isolation, timeout,
  JUnit output, and result collection;
- `windows-common/CMakeLists.txt` and its runner;
- small compatibility headers and test mocks required at the Windows DLL
  boundary.

When a common TC exposes a real Windows implementation issue, fix the backend.
When Linux and Windows require different platform services, add the equivalent
Windows test mock. Do not change a common TC merely to make Windows green.

## Prerequisites

Build the Windows dependencies, core, and adaptor first:

```powershell
.\windows-dependencies\install.ps1
.\dali-core\build\windows\build.ps1 -Configuration Debug
.\dali-adaptor\build\windows\build.ps1 -Configuration Debug
```

The repositories must be siblings in one workspace, as required by the DALi
Windows build scripts.

## Build

From the workspace root:

```powershell
.\dali-adaptor\automated-tests\build.ps1 -Configuration Debug
```

Use `-Clean` to recreate `automated-tests/_build/windows`.

## Run

```powershell
# List the common UTCs compiled for Windows
.\dali-adaptor\automated-tests\execute.ps1 -List

# Run all common UTCs
.\dali-adaptor\automated-tests\execute.ps1

# Run one existing common UTC
.\dali-adaptor\automated-tests\execute.ps1 `
  -TestCase UtcDaliApplicationNew01

# Run a source-area prefix
.\dali-adaptor\automated-tests\execute.ps1 `
  -Prefix UtcDaliPixelBuffer
```

Each TC runs in a separate process. Results are written to:

```text
automated-tests/results/windows/<timestamp>/
  environment.json
  junit.xml
  summary.json
  logs/
```

## Current portability gaps

The initial MSVC port compiles and exposes all 365 common adaptor UTCs. The
full baseline passed 352 tests; the Windows environment-variable mock fix was
then verified separately with `UtcDaliApplicationGetDataPathP`.

The remaining failures are intentionally visible, not silently skipped:

- FileLoader text-mode handling of CRLF input;
- Win32 timer callback pumping versus the Ecore timer mock;
- Windows type-registry visibility for the Timer invoke-method test;
- X11-specific Window creation and fixed mock screen-size expectations;
- unavailable Linux `.so` GL-window addon tests;
- codec/curl feature differences in two image tests;
- one accessibility coordinate-result difference.

These are the next mock, dependency, or backend tasks. Keeping them visible
prevents a green build from overstating Windows coverage.

## Unverified Windows issues

Issues in this section are hypotheses, not confirmed product defects. Do not
change common product code for them until the failure is reproduced on Windows.
When a failure is reproduced, prefer a Windows test mock or environment fix. A
product-code change is appropriate only when the same behavior can affect a
real Windows application.

### FileLoader text-mode CRLF handling

MSVC text streams may translate CRLF sequences to LF. In that case, `tellg()`
can report the physical file size while `read()` returns fewer logical bytes
and sets EOF. The original `ReadFile()` implementation may consequently return
failure for a valid text file. The expected UTC symptom is a failed
`UtcDaliReadFileNew1` or `UtcDaliReadFileNew2`, not a crash.

Before changing `file-loader-impl-generic.cpp`:

1. Check whether `automated-tests/resources/test.txt` was checked out with LF
   or CRLF line endings.
2. Run `UtcDaliReadFileNew1` and `UtcDaliReadFileNew2` against the unchanged
   baseline implementation.
3. Preserve the resource line-ending information and test logs with the
   result.

If the failure is reproduced only for Windows CRLF text input, limit the
candidate exception to `_WIN32` text-mode reads and compare it against the same
resource and environment. Binary short reads and non-EOF stream errors must
remain failures on every platform. If the failure is not reproduced, leave the
product implementation unchanged and retain this note for future diagnosis.
