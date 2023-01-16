# popcorn

A hobby microkernel

## Contributing

Please don't yet

## Building

```shell
mkdir build && cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=../cmake/cross-amd64.cmake
```

Then build with your favourite backend (Only `ninja` has been tested)

### Targets:

- `popcorn` produces a live ISO called `popcorn.iso` in the build root
- `docs` produces Sphinx docs in `docs/docs/sphinx` inside the build root
- `convolution` builds the core kernel
- `libk.a` builds the kernel C standard library
- `libk++.a` builds the kernel C++ standard library

Building the kernel will also download and build `libcxxrt` and `libunwind`

### Additional CMake options:

- `-DENABLE_KERNEL_UBSAN` enables or disables clang UBSan - off by default
    - `-DENABLE_LIBK_UBSAN` enables or disabled UBSan within libk - on by default if UBSan is on
    - `-DENABLE_LIBKPP_UBSAN` enables or disabled UBSan within libk++ - on by default if UBSan is on
- `-DRAMDISK_INCLUDED_DRIVERS` semicolon separated list of drivers and servers to include in the ramdisk (unused for
  now)
- `-DCMAKE_BUILD_TYPE` change build type between `Debug`, `Release`, `RelWithDebInfo` and `MinSizeRel` - defaults
  to `Debug`

### Build dependencies

Currently only supports `clang` and `lld`

- `llvm`
- `cmake`
- `grub`
- `nasm`

## Running

**All builds require a minimum of 1GiB of memory**

Live ISOs provided on the release page only support BIOS, not UEFI

Run prebuilt ISOs with `qemu-system-x86_64 -drive file=popcorn.iso,format=raw -m 1G`

Logging is printed on the serial port, which can be dumped on stdout by adding `-serial stdio`
