#!/bin/bash

#
#Script to run test_libmlinwrf
#

module -q load intel
module -q load mkl
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$(pwd)
./test_libmlinwrf

