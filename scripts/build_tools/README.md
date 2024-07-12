## Table of contents
- [Utilizing the build script: build.sh](#utilizing-the-build-script:-build.sh)
- [create_dfu_image.py description](#create_dfu_image.py-description)
- [extract_public_key.py description](#extract_public_key.py_description)

# Utilizing the build script: build.sh
The build.sh script is a simple bash script that is used to build the desired application.
In order to build the project called: <project_name> you need to add the name of the project as input argument.
For example, in order to build the bootloader project:

```bash
.\build.sh bootloader
```

The repository architecture allows to add more projects in a modular way. Given that someone follows the same pattern,
while adding a new project in this repository, as the <repo_root>/projects/app and <repo_root>/projects/bootloader,
they should be able to utilize the same script for building the new project.

It is also important, for the build_info.yaml to be properly set-up to provide the build.sh script with the correct information to facilitate the needs of each project.
For example, the **app** project should also create a firmware update image, after build. The firmware update image needs some information like:
- version (major.minor.patch)
- vendor private key (for signing)

The **bootloader** project on the other side, does not need a dfu (device firmware update) image, therefore the **dfu_image** option is disable for that. On the other hand, the **bootloader** project needs a headerfile that contains the public key (based on the private-public key pair). That is why, in the **build_info.yaml** the option **create_public_key** is enabled for this project.

Therefore, the two options supported right now, for further project binary processing after build, are:
- Create firmware update image.
- Create public key headerfile.

To achieve those extra features, the following scripts are utilized by the build.sh script automatically:
- <repo_root>/scripts/build_tools/create_dfu_image.py
- <repo_root>/scripts/build_tools/extract_public_key.py

## build: all
The build.sh script supports a build.sh all functionality. When the command all is issued, the build script builds every project found under build_info.yaml, that the build_all: true applies. The **build_all** option provided in the **build_info.yaml** adds the relevant project to the build all group.

```bash
.\build.sh all
```

## What does the script do? Script steps...
The build.sh script basically does the following:
1) Given the project name as input, it locates the relevant CMakeLists.txt file, in our repository.
2) A build folder is created under <repo_root>/projects/<project_name>/ (if already existis, it is being removed first).
3) The command:
```bash
cmake -G "Ninja" ..
```
is being executed.
4) The command:
```bash
ninja
```
is triggered to produce the final binaries.
5) Goes through extra steps (other than building) as described earlier (utilizing the build_info.yaml information)

NOTE: In order to utilize this script, you need to **install yq package** on your computer, this module is being used by the build.sh script to parse the yaml file config.
To install in Windows:

```bash
choco install yq
```

# create_dfu_image.py description
This script is being used to post process a firmware binary file, to make it compatible with the bootloader firmware update requirements.
More information for that, can be found inside the python script.
To utilize it for a specific binary, run the following command:

```bash
python create_dfu_image.py </path/to/bin> <path/to/linker_script> <version_major> <version_minor> <patch>
```

After the execution of this script, a new binary will be created that will be ready to be flashed to the board, and be compatible with out bootloader.
Also, a yaml file will be produced that will contain information related to the final binary. (e.g. the crc, sha256 hash, version of the binary).

# extract_public_key.py description
This script is being used pre-process a project firmware headerfile and rewrite it in order to add the public key information, in C-array format. The final result of that header file will be:
```bash
/**
 * @file ecdsa_pub_key.h
 * @brief ECDSA public key in C array format
 * @version 0.1
 * @date 2024-06-09
 *
 * @copyright Copyright (c) 2024
 *
 */
#ifndef ECDSA_PUB_KEY_H
#define ECDSA_PUB_KEY_H

// --- includes --------------------------------------------------------------------------------------------------------
#include <stdint.h>

// --- constants -------------------------------------------------------------------------------------------------------
const uint8_t ecdsa_public_key[] = { <public_key info> };

#endif // ECDSA_PUB_KEY_H

```
Usage:
```bash
python extract_public_key.py <public_key.pem> <ecdsa_pub_key.h>
```