#!/bin/sh

meson setup builddir
ninja -C builddir
cd builddir
ninja -t compdb -x c_COMPILER cpp_COMPILER > compile_commands.json
cp -r ../.idea ./
