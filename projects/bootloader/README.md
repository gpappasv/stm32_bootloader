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

## Alternatively: using the build.sh script
Alternatively, to build the bootloader project without hustle, the build.sh script is there for you, under:
<repo_root>/scripts/build.sh. You can find the relevant instructions for that under the <repo_root>/scripts folder,
inside the relevant README.md file.

At this point, under the /build folder you should be able to find the hex/elf/bin files.

**IMPORTANT!**: In order to properly and easily utilize the bootloader and create a firmware binary that can be supported by it, it is best to use the **build.sh** script, as there are quite a few steps that need to be taken to make a firmware binary compatible with the bootloader.
More information about that can be found in the relevant README, under scripts/build_tools.

# Bootloader architecture
The bootloader software architecture can be found under docs, in high level design. One important aspect of the design
is that, changing the implementations under src/drivers to be compatible with another device, should make the porting
of the bootloader smooth.

TODO: GPA: provide a full bootloader architecture documentation when finished!

# Steps to use the bootloader security features (authentication)

