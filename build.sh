#!/bin/bash
if [ ! -d build ]; then
  mkdir build
fi
cmake -S . -B build -G Ninja
sudo ninja -C build install
./build/indicadores $1
