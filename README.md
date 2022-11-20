# popcorn
A hobby microkernel

## Contributing
Please don't yet

## Building
Compile with `make` in the root directory

Currently only tested with `clang` and `lld`

Should produce a live ISO in `build/`

### Build dependencies
- `clang`
- gnu `make`
- `grub`
- `nasm`
- `cargo` - must use nightly toolchain

## Running
Currently only tested in `qemu`

Live ISOs provided on the release page only support BIOS, not UEFI

Run in qemu with `make run` - debug info will be printed to the terminal, or run prebuilt ISOs with `qemu-system-x86_64 -drive file=hug.iso,format=raw`
