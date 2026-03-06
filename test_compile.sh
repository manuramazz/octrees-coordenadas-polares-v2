#!/bin/bash

rm -rf build

cmake -B build -DCMAKE_BUILD_TYPE=Release . -DBUILD_TESTS=ON .

cmake --build build


