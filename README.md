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
      * [3. Building for Emscripten](#3-building-for-emscripten)
         * [Setup dali-env &amp; build dali-core](#setup-dali-env--build-dali-core)
         * [Build the minimal dali-adaptor](#build-the-minimal-dali-adaptor)
         * [Using the Javascript Build](#using-the-javascript-build)

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
         $ /configure --prefix=$DESKTOP_PREFIX
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

## 3. Building for Emscripten

Currently the build for emscripten uses a minimal adaptor which does not support dali-toolkit.

### Setup dali-env & build dali-core

 The build requires the Emscripten SDK, this is installed automatically by running dali_env with the correct parameters.
 dali_env is part of the dali-core repository.
 Please see the README within dali-core to setup dali_env and build dali-core.

### Build the minimal dali-adaptor

  Use the build.sh script build adaptor.
  This uses emscriptens emcc to compile byte code to javascript (full OpenGL ES support with stb-image loading library and cpp bindings).
  Note: Please view the build.sh script for debug build options.

         $ # cd ./build/emscripten
         $ # ./build.sh

### Using the Javascript Build

 The build will create 1 main Javascript artifact, and its html counterpart; dali-emscripten.js

 This is required by any dali JS app, and must be located in the same directory as the app JS in order for the browser to find it.

 After the build, the necessary artifacts (which include dali-wrapper.js) will be placed in the dali-env directory under opt/share/emscripten:

 dali-env/opt/share/emscripten

 If dali-demo is built, any JS examples will also be placed in this directory, so they are ready to run.
