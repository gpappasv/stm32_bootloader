#!/bin/bash

# Function to build a single project
build_project() {
    local project=$1
    local project_dir="$GIT_ROOT/projects/$project"

    if [ ! -d "$project_dir" ]; then
        echo "Project '$project' does not exist."
        return 1
    fi

    # Create the build directory if it doesn't exist
    local build_dir="$project_dir/build"
    if [ ! -d "$build_dir" ]; then
        mkdir -p "$build_dir"
    else
        # If the build directory exists, remove all files and folders under it
        rm -rf "$build_dir"/*
    fi

    # Navigate to the build directory
    cd "$build_dir" || return 1

    # Run cmake with Ninja generator in the parent directory
    echo "Running cmake command for $project..."
    cmake -G "Ninja" ..

    # Run Ninja to build the project
    echo "Building $project with Ninja..."
    ninja
}

# Function to create DFU image
create_dfu_image() {
    local script_dir="$GIT_ROOT/scripts/build_tools"
    if [ ! -d "$script_dir" ]; then
        echo "Folder '$script_dir' does not exist."
        return 1
    fi

    # Navigate to the script directory
    cd "$script_dir" || return 1

    local binary_path=$1
    local linker_script=$2
    local version_major=$3
    local version_minor=$4
    local version_patch=$5
    local private_key_path=$6

    # Check if the Python script exists
    if [ ! -f "create_dfu_image.py" ]; then
        echo "Error: 'create_dfu_image.py' script not found in '$script_dir'. Skipping DFU image creation."
        return 1
    fi

    echo "Creating DFU image..."
    pwd
    python create_dfu_image.py "$GIT_ROOT/$binary_path" "$GIT_ROOT/$linker_script" "$version_major" "$version_minor" "$version_patch" "$GIT_ROOT/$private_key_path"
    #if python fails, try with python3
    if [ $? -ne 0 ]; then
        python3 create_dfu_image.py "$GIT_ROOT/$binary_path" "$GIT_ROOT/$linker_script" "$version_major" "$version_minor" "$version_patch" "$GIT_ROOT/$private_key_path"
    fi
    #TODO: GPA: don't fail the rest of the script if the python script fails.
}

# Function to run the extract_public_key.py script
extract_public_key() {
    local script_dir="$GIT_ROOT/scripts/build_tools"
    if [ ! -d "$script_dir" ]; then
        echo "Folder '$script_dir' does not exist."
        return 1
    fi

    # Navigate to the script directory
    cd "$script_dir" || return 1

    local public_key_path=$1
    local public_key_header_path=$2

    # Check if the Python script exists
    if [ ! -f "extract_public_key.py" ]; then
        echo "Error: 'extract_public_key.py' script not found in '$script_dir'. Skipping public key extraction."
        return 1
    fi

    echo "Extracting public key..."
    python extract_public_key.py "$GIT_ROOT/$public_key_path" "$GIT_ROOT/$public_key_header_path"
    #if python fails, try with python3
    if [ $? -ne 0 ]; then
        python3 extract_public_key.py "$GIT_ROOT/$public_key_path" "$GIT_ROOT/$public_key_header_path"
    fi
}

# Check if a project name is provided as an argument
if [ $# -ne 1 ]; then
    echo "Usage: $0 <project_name|all>"
    read -rp "Press Enter to exit..."
fi

# Extract project name from command-line argument
PROJECT_NAME=$1

# Find the root directory of the git repository
GIT_ROOT=$(git rev-parse --show-toplevel)

# Check if the required yaml parser is installed
if ! command -v yq &> /dev/null; then
    echo "yq command could not be found. Please install yq to continue."
    read -rp "Press Enter to exit..."
fi

# Parse the YAML file
YAML_FILE="$GIT_ROOT/scripts/build_tools/build_info.yaml"
if [ ! -f "$YAML_FILE" ]; then
    echo "YAML file '$YAML_FILE' not found."
    read -rp "Press Enter to exit..."
fi

# Function to process a specific project
process_project() {
    local project=$1

    # Check if the project exists in the YAML file
    project_exists=$(yq e ".projects[] | select(.project_name == \"$project\") | .project_name" "$YAML_FILE")

    if [ -z "$project_exists" ]; then
        echo "Project '$project' not found in YAML file."
        return 1
    fi

    # Check if public key extraction is enabled
    public_key_enabled=$(yq e ".projects[] | select(.project_name == \"$project\") | .create_public_key.enabled" "$YAML_FILE")
    if [ "$public_key_enabled" == "true" ]; then
        public_key_path=$(yq e ".projects[] | select(.project_name == \"$project\") | .create_public_key.public_key_path" "$YAML_FILE")
        public_key_header=$(yq e ".projects[] | select(.project_name == \"$project\") | .create_public_key.public_key_header" "$YAML_FILE")

        echo "Extracting public key for project: $project"
        extract_public_key "$public_key_path" "$public_key_header"
        if [ $? -ne 0 ]; then
            echo "Failed to extract public key for project '$project'. Continuing with the rest of the script..."
        fi
    else
        echo "Public key extraction not enabled for project: $project"
    fi

    echo "Building project: $project"
    build_project "$project"
    if [ $? -ne 0 ]; then
        echo "Failed to build project '$project'. Exiting."
        read -rp "Press Enter to exit..."
        exit 1
    fi

    # Check if dfu_image creation is enabled
    dfu_enabled=$(yq e ".projects[] | select(.project_name == \"$project\") | .dfu_image.enabled" "$YAML_FILE")
    if [ "$dfu_enabled" == "true" ]; then
        binary_path=$(yq e ".projects[] | select(.project_name == \"$project\") | .dfu_image.binary_path" "$YAML_FILE")
        linker_script=$(yq e ".projects[] | select(.project_name == \"$project\") | .dfu_image.linker_script" "$YAML_FILE")
        version_major=$(yq e ".projects[] | select(.project_name == \"$project\") | .dfu_image.version.major" "$YAML_FILE")
        version_minor=$(yq e ".projects[] | select(.project_name == \"$project\") | .dfu_image.version.minor" "$YAML_FILE")
        version_patch=$(yq e ".projects[] | select(.project_name == \"$project\") | .dfu_image.version.patch" "$YAML_FILE")
        private_key_path=$(yq e ".projects[] | select(.project_name == \"$project\") | .dfu_image.private_key_path" "$YAML_FILE")

        echo "Creating DFU image for project: $project"
        create_dfu_image "$binary_path" "$linker_script" "$version_major" "$version_minor" "$version_patch" "$private_key_path"
        if [ $? -ne 0 ]; then
            echo "Failed to create DFU image for project '$project'. Continuing with the rest of the script..."
        fi
    else
        echo "DFU image creation not enabled for project: $project"
    fi
}

# Handle the "all" case
if [ "$PROJECT_NAME" == "all" ]; then
    # Read project names from the YAML file where build_all is true
    project_names=$(yq e '.projects[] | select(.build_all == true) | .project_name' "$YAML_FILE")

    for project in $project_names; do
        process_project "$project"
    done
else
    # Process the specific project provided as an argument
    process_project "$PROJECT_NAME"
fi

# Keep the terminal open
read -rp "Press Enter to exit..."
