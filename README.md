# mlinwrf
Run trained machine learning models within WRF.

### Installation
1) Install anaconda python 3
2) Create a virtual env
   $conda create --name mlinwrf 
3) Activate the virtual env
   $conda activate mlinwrf   
4) Install the python module and all of it's depdencies
   $pip install ./setup.py
5) Follow instructions in fortran/README to build the Fortran dynamic library for linking with WRF.
