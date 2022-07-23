# Learning to use GStreamer libraries from the tutorials

This repository's goal is to follow GStreamer tutorials with learning purposes. Mainly I intend to write the code in C/C++, but maybe I'll add Python as well.

To install GStreamer you can follow their instructions in [this link](https://gstreamer.freedesktop.org/documentation/installing/on-linux.html?gi-language=c).

The tutorials themselves can be found [here](https://gstreamer.freedesktop.org/documentation/tutorials/index.html).


## How to compile and run the examples

I'll write a CMakeLists.txt file for each of the examples and I'll leave a "build.sh" script to make it easier to compile. You can compile it by going into the root folder of the example that you want to build and type:

```shell
./build.sh
```

I will also try to compile them as instructed by the tutorials directly with gcc in a "c" folder.

To run the example go into the same folder where you built the binaries and type (without the angular brackets):

```shell
./bin/<EXAMPLE_NAME>
```