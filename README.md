# mlinwrf
Run trained machine learning models within WRF.

### Installation
1) Install anaconda python 3
2) Create a virtual env
   $conda create --name mlinwrf 
3) Activate the virtual env
   $conda activate mlinwrf   
4) Install numpy 1.18.5 since tensorflow 2.3.0 requires numpy<1.19.0
   $conda install -c conda-forge numpy=1.18.5
5) Install the python module and all of it's depdencies
   $pip install .
6) Follow instructions in fortran/README to build the Fortran dynamic library for linking with WRF.
