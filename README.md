<img src="https://dalihub.github.io/images/DaliLogo320x200.png">

# Table of Contents

   * [Build Instructions](#build-instructions)
      * [1. Building for Ubuntu desktop](#1-building-for-ubuntu-desktop)
         * [Minimum Requirements](#minimum-requirements)
         * [Building the Repository](#building-the-repository)
         * [Build target options](#build-target-options)
         * [Building and executing test cases](#building-and-executing-test-cases)
      * [2. GBS Builds](#2-gbs-builds)
         * [DEBUG Builds](#debug-builds)

# Build Instructions

## 1. Building for Ubuntu desktop

### Requirements

 - Ubuntu 14.04 or later
 - Environment created using dali_env script in dali-core repository
 - GCC version 6

DALi requires a compiler supporting C++11 features.
Ubuntu 16.04 is the first version to offer this by default (GCC v5.4.0).

GCC version 6 is recommended since it has fixes for issues in version 5
e.g. it avoids spurious 'defined but not used' warnings in header files.

### Building the Repository

To build the repository enter the 'build/tizen' folder:

         $ cd dali-adaptor/build/tizen

Then run the following commands:

         $ autoreconf --install
         $ ./configure --prefix=$DESKTOP_PREFIX
         $ make install -j8

### Building and executing test cases

See the README.md in dali-adaptor/automated-tests.

## 2. GBS Builds

         $ gbs build -A [TARGET_ARCH]

### DEBUG Builds

         $ gbs build -A [TARGET_ARCH] --define "%enable_debug 1"

