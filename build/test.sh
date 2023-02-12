#!/bin/bash

# author        Oliver Blaser
# date          11.02.2023
# copyright     GNU GPLv3 - Copyright (c) 2023 Oliver Blaser

rm -rf ../test/system/out-dir

./cmake/phodime -v ../test/system/Joe/ ../test/system/Mary/ ../test/system/a-file ../test/system/Emily ../test/system/Emily2/Emily ../test/system/out-dir
