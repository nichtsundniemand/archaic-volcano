# Archaic Volcano #
This repository will contain a basic libretro-vulkan implementation. It's mainly being written for educational purposes.
The goal is to eventually develop a simple, libretro-compatible game engine written in modern C++.

If you found this and are interested in using the code, feel free to do so - I would love to hear about it.

In case you want to contact me: Tips, critique and ideas are always welcome :).

## Building ##
To build this project you can just clone this repository and type `meson build && ninja -C build` in the
project's root-directory.

## Running ##
In order to run this you need a vulkan-capable GPU-driver and the libretro-frontend RetroArch.

In principle you could use any other frontend as well. I just went with RetroArch since it's pretty nice.

To run, just execute `ninja -C build run`.
