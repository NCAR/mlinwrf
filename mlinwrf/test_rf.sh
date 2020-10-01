#!/bin/bash

#Script performs a simple diff of Python vs Fortran random forest predictions
#Prints a passing message and returns 0 on a successful test or a failing message and 1 on failure
#Example Usage: ./test_rf.sh

export PATH=$PATH:.
test_dir=$(pwd)/1d_rf_test

function cleanup {
    rm -rf $test_dir
}
trap cleanup EXIT

echo "Generating test data and performing predictions in Python"
py_rf_out=$(gen_test_models.py random_forest_csv ${test_dir})
echo "Performing predictions in Fortran"
curr_dir=$(pwd)
cd ../fortran
fort_rf_out=$(run_testmlinwrf.sh random_forest_csv "${test_dir}/random_forest/")
cd $curr_dir

echo "Comparing Python Neural Net vs Fortran Neural Net predictions"
diff <(echo "$py_rf_out") <(echo "$fort_rf_out")
exit_status="$?"
if [ $exit_status -eq 0 ]; 
then
    echo "**Test Passed**: Python and Fortran are identical"
    exit 0
else
    echo "**Test Failed**"
    exit 1
fi
