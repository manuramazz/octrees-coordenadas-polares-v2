#!/bin/bash

rm -rf build

cmake -B build -DCMAKE_BUILD_TYPE=Release . -DBUILD_TESTS=OFF . -DFETCHCONTENT_FULLY_DISCONNECTED=ON .

cmake --build build -j$(nproc)


