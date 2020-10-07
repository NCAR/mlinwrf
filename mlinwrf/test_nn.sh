#!/bin/bash

#Script performs a simple diff of Python vs Fortran neural network predictions
#Prints a passing message and returns 0 on a successful test or a failing message and 1 on failure
#Example Usage: ./test_nn.sh

export PATH=$PATH:.
test_file=$(pwd)/1d_nn_test.nc

function cleanup {
    rm -f $test_file
}
trap cleanup EXIT

echo "Generating test data and performing predictions in Python"
py_nn_out=$(gen_test_models.py neural_net_nc $test_file)
echo "Generating test data and performing predictions in Fortran"
curr_dir=$(pwd)
cd ../fortran
fort_nn_out=$(run_testmlinwrf.sh neural_net_nc $test_file)
cd $curr_dir

echo "Comparing Python Neural Net vs Fortran Neural Net predictions"
diff <(echo "$py_nn_out") <(echo "$fort_nn_out")
exit_status="$?"
if [ $exit_status -eq 0 ]; 
then
    echo "**Test Passed**: Python and Fortran are identical"
    exit 0
else
    echo "**Test Failed**"
    exit 1
fi
