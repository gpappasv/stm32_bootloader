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

## build: all
The build.sh script supports a build.sh all functionality. When the command all is issued, the build script builds every project found under build_info.yaml, that the build_all: true applies.

```bash
.\build.sh all
```

## What does the script do?
The build.sh script basically does the following:
1) Given the project name as input, it locates the relevan CMakeLists.txt file, in our repository.
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
5) Utilizes the firmware_update_tools (if needed), to post-process the binary produced, to follow with the bootloader firmware update requirements.

NOTE: In order to utilize this script, you need to install yq package on your computer, this module is being used by the build.sh script to parse the yaml file config.
To install in Windows:

```bash
choco install yq
```