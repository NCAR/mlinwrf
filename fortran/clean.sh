#!/bin/bash

#
#Cleans the mlinwrf.so test program and test_libmlinwrf testing program
#

echo "Setting up the environment"
module -q load intel
module -q load mkl
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$(pwd)
echo "Cleaning with Scons"
scons -c

