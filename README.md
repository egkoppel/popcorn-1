# popcorn

A hobby microkernel

## Contributing

Please don't yet

## Building

```shell
mkdir build && cd build
cmake .. -DHUGOS_ARCH=amd64 -DCMAKE_TOOLCHAIN_FILE=../cmake/cross-amd64.cmake
```

Then build with your favourite backend (Only `make` and `ninja` have been tested)

### Targets:

- `popcorn_iso` produces a live ISO called `popcorn.iso` in the build root
- `initramfs` generates an initramfs file called `initramfs.tar.gz` in the build root
- `Sphinx` produces Sphinx docs in `docs/docs/sphinx` inside the build root
- `convolution` builds the core kernel
- `hugos_libk` builds the kernel C standard library

Building the kernel will also download and build `libcxxrt` and `libunwind`

### Additional CMake options:

- `-DENABLE_UBSAN` enables or disables clang UBSan - off by default
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
