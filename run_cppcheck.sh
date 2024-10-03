#!/bin/bash

BASE_PATH="$HOME"
SCRIPT_DIR="$(cd "$SCRIPT_DIR" && pwd)"
INCLUDE_PATHS="-I/$BASE_PATH/.particle/toolchains/deviceOS/6.1.0/user/inc \
                -I/$BASE_PATH/.particle/toolchains/deviceOS/6.1.0/user/src \
                -I/$BASE_PATH/.particle/toolchains/deviceOS/6.1.0/user/lib \
                /$BASE_PATH/Documents/codes/KNELL_SYS/KNELL/src"
FILE_CHECK="$SCRIPT_DIR/KNELL.cpp"
if ! command -v cppcheck &> /dev/null
then
    echo "cppcheck could not be found, please install it before running this script."
    exit 1
fi

echo "Running cppcheck on the project..."

cppcheck --enable=all --inconclusive --std=c++11 $INCLUDE_PATHS $FILE_CHECK 

echo "cppcheck analysis completed."