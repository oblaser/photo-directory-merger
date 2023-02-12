#!/bin/bash

# author        Oliver Blaser
# date          20.11.2022
# copyright     GNU GPLv3 - Copyright (c) 2022 Oliver Blaser

cd ./omw/

cd ./lib/ &&
{
    rm -f *.a
    rm -f *.so*
    cd ../
}

cd ./build/
./make_clean.sh
./cmake_clean.sh
cd ../../



./build.sh
