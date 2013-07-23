#!/bin/bash
set -e

if [[ $EUID -ne 0 ]]; then
   echo "This script must be run as root"
   exit 1
fi

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cd $DIR
FIRMWARE=led-board.hex

openocd \
    -f ../../config/openocd/olimex-arm-usb-tiny-h.cfg \
    -f ../../config/openocd/vaporlight.cfg \
    -c "init" \
    -c "halt" \
    -c "flash write_image erase unlock $FIRMWARE" \
    -c "reset" \
    -c "exit"

