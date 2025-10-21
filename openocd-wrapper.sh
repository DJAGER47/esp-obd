#!/bin/bash

ORIGINAL_OPENOCD="/bin/openocd"
CUSTOM_OPENOCD="/home/plavrovskiy/.espressif/tools/openocd-esp32/v0.11.0-esp32-20221026/openocd-esp32/bin/openocd"
BACKUP_OPENOCD="/bin/openocd.backup"

function replace_openocd() {
    if [ ! -f "$CUSTOM_OPENOCD" ]; then
        echo "Custom OpenOCD not found at $CUSTOM_OPENOCD"
        return 1
    fi

    if [ ! -f "$BACKUP_OPENOCD" ]; then
        sudo mv "$ORIGINAL_OPENOCD" "$BACKUP_OPENOCD"
    fi

    sudo ln -s "$CUSTOM_OPENOCD" "$ORIGINAL_OPENOCD"
    echo "Created symlink from $ORIGINAL_OPENOCD to custom version"
}

function restore_openocd() {
    if [ -f "$BACKUP_OPENOCD" ]; then
        sudo mv "$BACKUP_OPENOCD" "$ORIGINAL_OPENOCD"
        echo "Original OpenOCD restored"
    else
        echo "No backup found to restore"
        return 1
    fi
}

case "$1" in
    replace)
        replace_openocd
        ;;
    restore)
        restore_openocd
        ;;
    *)
        echo "Usage: $0 {replace|restore}"
        exit 1
        ;;
esac