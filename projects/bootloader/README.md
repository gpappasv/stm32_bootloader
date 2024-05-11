This folder contains the relevant source files of the stm32 bootloader

# Build instructions
- First make sure you have already downloaded arm toolchain in your computer.
(gcc-arm-none-eabi-10.3-2021.10-win32.exe preferably)

```bash
mkdir build
cd build
cmake -G "Ninja" ..
```

## build the final binaries
```bash
ninja
```

At this point, under the /build folder you should be able to find the hex/elf/bin files.