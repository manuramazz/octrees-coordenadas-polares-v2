#!/bin/bash

rm -rf build
mkdir -p build && cd build
mkdir -p ~/compilacion_temporal
export TMPDIR=$HOME/compilacion_temporal

cmake -DCMAKE_C_COMPILER=gcc \
      -DCMAKE_CXX_COMPILER=g++ \
      -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_PREFIX_PATH=$HOME/my_libs/LAStools \
      ..

cmake --build . -j2


