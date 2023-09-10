# PlayStation Eye driver

A cross-platform user-mode driver for the PlayStation Eye camera based on [libusb](https://libusb.info/) and written in
C++20.

## Features

- Cross-platform library for accessing and manipulating the camera.
- DirectShow filters for 32/64-bit Windows applications (supporting RGBA and UYVY formats).

## Building

### Requirements

- MSVC 2022 or Clang with C++20 support (and compatible libc++ version)
- [gclient](https://github.com/timniederhausen/gclient) which requires Python in turn
- [CMake](https://cmake.org/)

### Instructions

Download all dependencies via:
```
$ gclient sync
```

Then use CMake to generate project files for your preferred generator:
```
$ cmake -B build -G <generator> ...
```
Some CMake [presets](CMakePresets.json) for Windows are also available.

## Supported hardware

According to [Alex Popovich](https://alexpopovich.wordpress.com/2008/09/05/sony-ps3eye-b304061-ir-filter-removal/) there
are multiple versions of the PlayStation Eye featuring different camera bridge processors.
This code should work with both the OV534 and the newer OV538.

So far it has only been tested with a OV538 bridge and a corresponding OV7721 camera sensor (i.e. a newer European model
of the PlayStation Eye).

### Related work

* [The Linux kernel supports the PlayStation Eye](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/tree/drivers/media/usb/gspca/ov534.c)
* [Alex Popovich's driver](https://alexpopovich.wordpress.com/2008/09/03/sony-ps3-eye-camera-on-windows-os/)
* [Jim Paris' & others research](https://forums.ps2dev.org/viewtopic.php?f=25&t=9238)
* [jkevin's DirectShow filter](https://github.com/jkevin/PS3EyeDirectShow)
* [inspirit's Windows/macOS driver](https://github.com/inspirit/PS3EYEDriver)
