#!/bin/bash

# author        Oliver Blaser
# date          20.11.2022
# copyright     GNU GPLv3 - Copyright (c) 2022 Oliver Blaser

cd ./omw/build/
omwBuildOk=0
{
    cd cmake &&
    cmake . &&
    make omw-static &&
    make omw-unit-test-static &&
    cd ../ &&
    mkdir -p ../lib &&
    cp cmake/libomw-static.a ../lib/libomw.a &&
    omwBuildOk=1
} ||
{
    echo -e "\033[91merror\033[39m"
}
if [ $omwBuildOk -ne 0 ]
then
    cd ./cmake/
    ./omw-unit-test-static
    cd ..
fi
cd ../../
