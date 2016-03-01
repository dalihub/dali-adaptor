#!/bin/bash

# This script builds dali-adaptor using Emscripten.

# DESKTOP_PREFIX is required to build Emscripten DALi.
if [ -z ${DESKTOP_PREFIX} ]; then
  echo "DESKTOP_PREFIX is required to build adaptor against dali-core. Please make sure you have sourced your setenv script (created by dali_env)."
  exit 1
fi


mkdir -p build
cd build
rm CMakeCache.txt

/usr/bin/cmake .. -DCMAKE_BUILD_TYPE=Debug -DEMSCRIPTEN=1

make -j8

mv dali-emscripten dali-emscripten.bc

# Non-optimised build.
emcc dali-emscripten.bc -o dali-emscripten.html --memory-init-file 0 -s FULL_ES2=1 -s STB_IMAGE=1 -s ALLOW_MEMORY_GROWTH=1 -s ASSERTIONS=1 -s DEMANGLE_SUPPORT=1 -s DISABLE_EXCEPTION_CATCHING=0 -s EXPORT_NAME=\"dali\" -g4 --bind

# Debug build.
#emcc dali-emscripten.bc -o dali-emscripten.html --memory-init-file 0 -s FULL_ES2=1 -s STB_IMAGE=1 -s ALLOW_MEMORY_GROWTH=1 -s ASSERTIONS=2 -s DEMANGLE_SUPPORT=1 -s DISABLE_EXCEPTION_CATCHING=0 -s EXPORT_NAME=\"dali\" --js-opts 0 -g4 --bind

# Optimised build.
#emcc dali-emscripten.bc -o dali-emscripten.html -s FULL_ES2=1 -s STB_IMAGE=1 -s ALLOW_MEMORY_GROWTH=1 -s ASSERTIONS=0 -s DISABLE_EXCEPTION_CATCHING=2  -s EXPORT_NAME=\"dali\" -O2 --bind

# Copy the required built artifacts to dali-env.
mv ./dali-emscripten.js ${DESKTOP_PREFIX}/share/emscripten/
mv ./dali-emscripten.html ${DESKTOP_PREFIX}/share/emscripten/

# If static memory initialisation code was created in a separate file, copy this too.
if [ -a ./dali-emcripten.html.mem ]; then
  mv ./dali-emscripten.html.mem ${DESKTOP_PREFIX}/share/emscripten/
fi
