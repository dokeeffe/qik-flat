#!/bin/bash
mkdir build
cd build
rm * -rf
cmake -DCMAKE_INSTALL_PREFIX=/usr ../qik_flat/
sudo make install
