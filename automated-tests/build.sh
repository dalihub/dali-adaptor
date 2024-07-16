#!/bin/bash


TEMP=`getopt -o rn --long rebuild,no-gen \
     -n 'genmake' -- "$@"`

if [ $? != 0 ] ; then echo "Terminating..." >&2 ; exit 1 ; fi

# Note the quotes around `$TEMP': they are essential!
eval set -- "$TEMP"

opt_rebuild=false
opt_generate=true

while true ; do
    case "$1" in
        -r|--rebuild) opt_rebuild=true ; shift ;;
        -n|--no-gen)  opt_generate=false ; shift ;;
        --) shift ; break ;;
        *) shift ;;   # Ignore
    esac
done

if [ false == $opt_rebuild -o ! -d "build" ] ; then
    rm -rf build
    mkdir build
fi

function build
{
    echo Building $1

    BUILDSYSTEM="Unix Makefiles"
    BUILDCMD=make
    if [ -e ../build/tizen/build.ninja ] ; then
        BUILDSYSTEM="Ninja"
        BUILDCMD=ninja
    fi

    if [ $opt_generate == true -o $opt_rebuild == false ] ; then
        (cd build ; rm CMakeCache.txt ; cmake .. -C /dev/null -G "$BUILDSYSTEM" -DMODULE=$1)
        (cd src/$1; ../../scripts/retriever.sh -l)
        (cd src/$1; ../../scripts/tcheadgen.sh tct-$1-core.h)
        if [ $? -ne 0 ]; then echo "Aborting..."; exit 1; fi
    fi

    (cd build ; cmake .. -DMODULE=$1 -G "$BUILDSYSTEM" ; $BUILDCMD -j7 )
}

# Query main build to determine if we are using vulkan or egl
ENABLE_VULKAN=0
(cd ../build/tizen ; cmake -LA -N 2>/dev/null | grep ENABLE_VULKAN | grep "\=ON")
if [ $? -eq 0 ] ; then
    ENABLE_VULKAN=1
fi
if [ -n "$1" ] ; then
  echo BUILDING ONLY $1
  build $1
else
  for mod in `ls -1 src/ | grep -v CMakeList `
  do
      build=0
      # Don't build dali-egl-graphics if we are using Vulkan...
      if [ $mod == 'dali-egl-graphics' ] ; then
          if [ $ENABLE_VULKAN == 0 ] ; then
              build=1
          fi
      elif [ $mod != 'common' ] && [ $mod != 'manual' ]; then
          build=1
      fi

      if [ $build == 1 ] ; then
          echo BUILDING $mod
          build $mod
          if [ $? -ne 0 ]; then echo "Build failed" ; exit 1; fi
      fi
  done
fi

echo "Build succeeded"
exit 0
