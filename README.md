# Description of repository
This is a repository that a custom stm32 bootloader is being developed.

The bootloader is designed to have an easily portable core, by just replacing the driver layer, with the board specific one.
The current implementation focuses on the STM32F401RE board.

## Table of contents
- [Bootloader features](#bootloader-features)
- [Configuration](#configuration)
- [Examples](#examples)
- [How to use](#how-to-use)
- [Repository structure](#repository-structure)

# Bootloader features
- Adaptable application space.
- Checksum verification, before booting the application.
- Authentication of the application. (Secure boot)
- Easy to port to other microcontrollers.
- Recovery mode (firmware update support).
- Backup image recovery. If the main application is broken, the secondary is tested.
- Secure communication through the custom communication protocol.

# Configuration
TODO.

# Examples
TODO.

# How to use
TODO.

# Repository structure
The structure of the repository is as follows:

*build*: folder under which all build artifacts should end up to (e.g. *.hex, *.bin files)

*docs*: folder under which all the relevant documentation resides for the projects under this repository.

*projects*: folder under which all the projects exist. The source code and CMakeLists files can be found under it.

*scripts*: folder under which all scripts should exist.

*etc*: folder under which random stuff can be found. Just a place to store things.

*third_party*: folder under which third party applications reside (e.g. ST Drivers)