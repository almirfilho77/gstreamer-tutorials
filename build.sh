#!/usr/bin/bash

function show_help
{
    help_me="
-----------------------------------------------------------------------
 build.sh -- Script to configure and build projects using cmake
 
 Usage: build.sh [options][path]
     
 Examples: 
 
    ./build.sh -h           --> View the help message
    ./build.sh -b basics-1  --> Configure and build the project inside './basics-1' path.
 
 Options:

 Help

 -h                --> Shows help message


 Build

 -b                --> Build the projet in the provided path
  
-----------------------------------------------------------------------
"
    echo "$help_me"
}

# Show help in case no option is provided
if [ $# -lt 1 ]
then
    show_help
    exit 1
fi

# Get options
while getopts "hb" opt; do
    case "$opt" in
        h|\?) show_help && exit 1;;
        b) build=1 ;;
    esac
done

# Execute core script
if [ $build -eq 1 ]
then
    if [ $# -eq 2 ]
    then
        build_path=$2
        echo "Building path is $build_path"
        cd $build_path
        mkdir -p ./bin
        mkdir build
        cd ./build
        cmake ../
        cmake --build .
        cd ../
        rm -rf build
    else
        echo "Please provide the path to the repository that you want to build!"
    fi
fi