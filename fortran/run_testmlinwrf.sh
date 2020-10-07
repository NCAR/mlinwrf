#!/bin/bash

#
#Script to run test_libmlinwrf
#
#Example Usage: ./run_testmlinwrf.sh neural_net_nc ../mlinwrf/1d_nn_test.nc
#               ./run_testmlinwrf.sh random_forest_csv ../mlinwrf/1d_rf_test/random_forest/
#

if [ -z "$1" -o -z "$2" ];
then
    echo "USAGE: $0 model_type model_path"
    exit 1
fi

export PATH=$PATH:.
module -q load intel
module -q load mkl
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:.
#gdb --args test_libmlinwrf "$1" "$2"
test_libmlinwrf "$1" "$2"

