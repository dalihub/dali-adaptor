% Dali Adaptor Automated Testing

Automated tests for the dali-adaptor project. These tests are callable from the
TCT testing framework.

There are currently three executables built, each of which has its own source
directory and contains several test cases. These are:

*   `src/dali-adaptor`
    Uses a mocked platform abstraction, so doesn't test the abstraction
    implementations themselves.
    This currently tests the timer and key presses only.
*   `src/dali-adaptor-internal`
    Tests some internal components of the abstraction implementations without
    attempting to have a live or mocked platform abstraction in place.
    This is the correct place for test cases which can test code in dali-adaptor
    in isolation.
    Currently tied to the SLP platform abstraction directly.
    Noteworthy test cases:
    *   `utc-Dali-GifLoader.cpp`: Runs the gif loader syncronously and checks
        that the laoded bitmap matches a reference buffer on a pixel by pixel
        basis.
    *   `utc-Dali-CommandLineOptions.cpp`: Tests whether commandline options of
         Dali are stripped out of various inputs.
*   `src/dali-platform-abstraction`
    Uses a real live platform abstraction so can be used to test features
    exposed through the `Dali::Integration::PlatformAbstraction` abstract
    interface.
    Intended to test complex interactions operate correctly.
    For example:
    *   Cancelation of a subset of a large number of in-flight
        requests.
    *   Correct reporting of successful and failed requests when large numbers
        of valid and invalid requests are issued in rapid sequence.

If a loader for an image type is added to the platform abstractions, a test case
should be added to `src/dali-adaptor-internal`, styled after
`utc-Dali-GifLoader.cpp`. An image of that type should also be added to the
 existing image test cases in `src/dali-platform-abstraction`.

If a new commandline option is added to Dali, `utc-Dali-CommandLineOptions.cpp`
should be expanded with it.

Building
========

Use the script `build.sh` in the root of `automated-tests/`.
To build all three test executables, run the script without arguments:

    ./build.sh

To build just one executable, pass its name to the build script:

    ./build.sh dali-adaptor

    ./build.sh dali-adaptor-internal

    ./build.sh dali-platform-abstraction

Running
=======

Use the `execute.sh` script. To execute the tests in all three executables, run the script with no arguments:

    ./execute.sh

To run just one executable, pass its name to the script:

    ./execute.sh dali-adaptor

    ./execute.sh dali-adaptor-internal

    ./execute.sh dali-platform-abstraction

To view the results of a test run, execute the following line:

    firefox --new-window summary.xml
