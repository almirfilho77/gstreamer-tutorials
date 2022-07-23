#!/usr/bin/bash

mkdir -p ./bin
mkdir build
cd ./build
cmake ../
cmake --build .
cd ../
rm -rf build
