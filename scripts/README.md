This folder contains all the repository scripts that make our lives easier. Each script should be contained in its own folder and have its own README.md description.

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

## What does the script do?
The build.sh script basically does the following:
1) Given the project name as input, it locates the relevan CMakeLists.txt file.
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