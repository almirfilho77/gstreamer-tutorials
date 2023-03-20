#!/usr/bin/bash

if [ $# -eq 1 ]
then
    echo "Building path is $1"
    cd $1
    mkdir -p ./bin
    mkdir build
    cd ./build
    cmake ../
    cmake --build .
    cd ../
    rm -rf build
else
    echo "Please provide only the path to the repository that you want to build!"
fi
