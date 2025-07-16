#!/bin/bash
if [ ! -d build ]; then
  mkdir build
fi
cmake -S . -B build -G Ninja
ninja -C build
./build/indicadores
