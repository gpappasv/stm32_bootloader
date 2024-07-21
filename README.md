# Description of repository
This is a repository that a custom lightweight stm32 bootloader is being developed.

The bootloader is designed to have an easily portable core, by just replacing the driver layer, with the board specific one.
The current implementation focuses on the STM32F401RE board.

## Table of contents
- [Bootloader features](#bootloader-features)
- [Configuration](#configuration)
- [Porting bootloader to other boards](#porting-bootloader-to-other-boards)
- [Examples](#examples)
- [How to use](#how-to-use)
- [Repository structure](#repository-structure)

# Bootloader features
- Adaptable application space.
- Checksum verification, before booting the application.
- Authentication of the application. (Secure boot). ECDSA is used.
- Easy to port to other microcontrollers.
- Recovery mode (firmware update support).
- Backup image recovery. If the main application is broken, the secondary is tested.
- Secure communication through the custom communication protocol.
- Bootloader flash used: ~14.5kB

NOTE: More in-depth documentation about how the bootloader works can be found under **docs**.

# Configuration (make your application compatible with the bootloader)
- Adapt the bootloader:
The bootloader needs to have some information about the following:
- The bootloader needs to know where the primary application is to be found (the flash starting address).
- The bootloader needs to know where the primary application ends (and based on that also determine its size).
- The bootloader needs to know where the secondary application is to be found (the flash starting address).
- The bootloader needs to know where the secondary application ends (and based on that also determine its size).

This information is provided to the bootloader, through the linker script file (e.g. STM32F401RETx_FLASH.ld, found under projects/bootloader/boards/stm32f401re).
In order to configure the bootloader to your own needs, you will need to open that file and modify the relevant linker script code:

```bash
/* --- BOOTLOADER CONFIGURATION SPECIFIC INFORMATION START --- */
__header_size_bytes__ = 72;
__header_crc_size_bytes__ = 4;
__header_fw_ver_size_bytes__ = 4;
__header_hash_size_bytes__ = 64;

__flash_app_start__ = 0x08008000;
__flash_app_end__ = 0x0803FFFF;
__header_app_start__ = __flash_app_end__ - __header_size_bytes__ + 1;
__header_app_end__ = __flash_app_end__;
__header_app_crc_start__ = __header_app_start__;
__header_app_fw_version_start__ = __header_app_start__ + __header_crc_size_bytes__;
__header_app_hash_start__ = __header_app_start__ + __header_crc_size_bytes__ + __header_fw_ver_size_bytes__;

__flash_app_secondary_start__ = 0x08040000;
__flash_app_secondary_end__ = 0x08077FFF;
__header_app_secondary_start__ = __flash_app_secondary_end__ - __header_size_bytes__ + 1;
__header_app_secondary_end__ = __flash_app_secondary_end__;
__header_app_secondary_crc_start__ = __header_app_secondary_start__;
__header_app_secondary_fw_version_start__ = __header_app_secondary_start__ + __header_crc_size_bytes__;
__header_app_secondary_hash_start__ = __header_app_secondary_start__ + __header_crc_size_bytes__ + __header_fw_ver_size_bytes__;

/* Specify the memory areas */
MEMORY
{
RAM (xrw)      : ORIGIN = 0x20000000, LENGTH = 96K
FLASH (rx)      : ORIGIN = 0x8000000, LENGTH = 32K /* 32K of flash is reserved for the bootloader */
}
/* --- BOOTLOADER CONFIGURATION SPECIFIC INFORMATION END --- */
```

NOTE: While configuring this part of the bootloader linker script you need to carefully consider the flash layout of your MCU.
A good practice would be to always start the primary and secondary applications from the beginning of the desired flash page. Also you need to be careful to not have any overlaps between the two.
Also, since the bootloader cannot update itself, once you flash the bootloader, you cannot modify the flash layout after that, on future application releases.

- Adapt your application:
Similarly to the bootloader, the application should also have that information inside its linker script. This is optional though, in case that you need that information inside your application for any reason.

It is important to not mess it up and be careful while copy-pasting that information to your linker script, as the tools that are provided under scripts/build_tools parse the application linker to find the relevant information. More information on how to use those tools can be found in the relevant README.md files.

- Utilize the script **scripts/build_tools/create_dfu_image.py** to make your app binary compatible to the built bootloader. More info on how to use it can be found in **scripts/build_tools/README.md**. The modified binary will be exported to the current working directory.

- You also need to modify the **private_key.pem** and **public_key.pem**, under **projects/**. They are being used to sign the application, and provide the bootloader with the relevant information on authenticating the signature.

# Porting bootloader to other boards
Porting a project that someone else has built to your own board, is never an easy task. There has been a good effort to
design the project in a way to be as easily portable as possible. Still there are a lot of improvements that need to happen
in that area, but you could start by reading the **README.md**, under **projects/bootloader**. There, you can find some useful notes
that can help you port the project faster to your board.

**You can always reach out to me, to guide you through it. Feel free to push a commit, adding support for your board.**

TODO: In the future, projects/bootloader/boards/<board_name> will contain all the **driver level** code for each specific board, and a simple define will utilize the correct board subfolder to build the bootloader.

# Examples
TODO.

# How to use
1. Modify the bootloader (linker script/drivers) as you like based on the information provided before.
2. Use the build.sh script to build the bootloader.bin. (e.g. build.sh bootloader)
3. Flash the bootloader.bin on flash address 0x08000000.
4. You can develop your application following the example of projects/app. In that case, modify the linker script accordingly, update the private-public key pair and use the build.sh script to build the application. (e.g. build.sh all OR build.sh app)
5. You can always have a binary yourself and utilize the underlying tools (create_dfu_image.py). With this script you can convert your binary to a compatible application to be used by the bootloader.

IMPORTANT: More information about each script can be found inside the relevant folder that contains it.

# Repository structure
The structure of the repository is as follows:

*build*: folder under which all build artifacts should end up to (e.g. *.hex, *.bin files)

*docs*: folder under which all the relevant documentation resides for the projects under this repository.

*projects*: folder under which all the projects exist. The source code and CMakeLists files can be found under it.

*scripts*: folder under which all scripts should exist.

*etc*: folder under which random stuff can be found. Just a place to store things.

*third_party*: folder under which third party applications reside (e.g. ST Drivers)