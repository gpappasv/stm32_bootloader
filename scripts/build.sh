#!/bin/bash

# Check if a project name is provided as an argument
if [ $# -ne 1 ]; then
    echo "Usage: $0 <project_name>"
    exit 1
fi

# Extract project name from command-line argument
PROJECT_NAME=$1

# Find the root directory of the git repository
GIT_ROOT=$(git rev-parse --show-toplevel)

# Navigate to the specified project directory
PROJECT_DIR="$GIT_ROOT/projects/$PROJECT_NAME"

if [ ! -d "$PROJECT_DIR" ]; then
    echo "Project '$PROJECT_NAME' does not exist."
    exit 1
fi

# Create the build directory if it doesn't exist
BUILD_DIR="$PROJECT_DIR/build"
if [ ! -d "$BUILD_DIR" ]; then
    mkdir -p "$BUILD_DIR"
else
    # If the build directory exists, remove all files and folders under it
    rm -rf "$BUILD_DIR"/*
fi

# Navigate to the build directory
cd "$BUILD_DIR" || exit

# Run cmake with Ninja generator in the parent directory
echo "Running cmake command..."
cmake -G "Ninja" ..

# Run Ninja to build the project
echo "Building $PROJECT_NAME with Ninja..."
ninja

# Keep the terminal open
read -rp "Press Enter to exit..."