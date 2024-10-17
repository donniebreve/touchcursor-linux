#!/bin/bash

# Check if an argument is provided
if [ $# -eq 0 ]; then
    echo "Error: No configuration file specified."
    echo "Usage: $0 <path_to_config_file>"
    exit 1
fi

CONFIG_FILE="$1"

# Set permissions to allow write for owner and read for others
chmod 644 "$CONFIG_FILE"

# Check if the config file exists
if [ ! -f "$CONFIG_FILE" ]; then
    echo "Error: $CONFIG_FILE does not exist. Please create a basic configuration file first."
    exit 1
fi

# Create a temporary file
TEMP_FILE=$(mktemp)

# Function to get all keyboard device names
get_keyboard_names() {
    grep -E 'Name=|Handlers=|EV=' /proc/bus/input/devices | 
    grep -B2 'EV=120013' --no-group-separator | 
    grep 'Name=' | 
    cut -d= -f2- | 
    sed 's/"//g' |
    sed 's/^[[:space:]]*//' |
    sed 's/[[:space:]]*$//'
}

# Process the configuration file
{
    device_section_found=false
    while IFS= read -r line || [[ -n "$line" ]]; do
        if [[ $line == "[Device]" ]]; then
            echo "$line"
            # Insert all keyboard devices
            while IFS= read -r device_name; do
                echo "Name=\"$device_name\""
            done < <(get_keyboard_names)

            device_section_found=true
        elif $device_section_found && [[ $line == Name=* ]]; then
            # Skip original device configurations
            continue
        elif [[ $line == "["*"]" ]]; then
            # Reset flag when a new section is found
            device_section_found=false
            echo "$line"
        else
            echo "$line"
        fi
    done < "$CONFIG_FILE"
} > "$TEMP_FILE"

# Check if the temporary file is not empty and is readable
if [ -s "$TEMP_FILE" ] && [ -r "$TEMP_FILE" ]; then
    # Create a backup of the original file
    cp "$CONFIG_FILE" "${CONFIG_FILE}.bak"
    
    # Replace the original file with the modified content
    mv "$TEMP_FILE" "$CONFIG_FILE"
    chmod 644 "$CONFIG_FILE"
    
    echo "Configuration ($CONFIG_FILE) has been updated."
    echo "A backup of the original configuration has been saved as ${CONFIG_FILE}.bak"
    echo "All keyboard devices have been added to the [Device] section."
else
    echo "Error: Failed to update the configuration. The original file has not been modified."
    rm "$TEMP_FILE"
    exit 1
fi