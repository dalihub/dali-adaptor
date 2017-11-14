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

### Minimum Requirements

 - Ubuntu 14.04 or later
 - Environment created using dali_env script in dali-core repository

### Building the Repository

To build the repository enter the 'build/tizen' folder:

         $ cd dali-adaptor/build/tizen

Then run the following commands:

         $ autoreconf --install
         $ ./configure --prefix=$DESKTOP_PREFIX
         $ make install -j8

### Build target options

OpenGL ES context:

When building, the OpenGL ES version of the target should be specified.

Valid version options are 20, 30, 31 for OpenGL ES versions 2.0, 3.0 and 3.1 respectively.

With configure:
Add: *--enable-gles=X*

With gbs:
Add to the gbs build line: *--define "%target_gles_version X"*

### Building and executing test cases

See the README.md in dali-adaptor/automated-tests.

## 2. GBS Builds

         $ gbs build -A [TARGET_ARCH]

### DEBUG Builds

         $ gbs build -A [TARGET_ARCH] --define "%enable_debug 1"

