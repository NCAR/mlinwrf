# mlinwrf
Run trained machine learning models within WRF.

### Building the mlinwrf shared library.
1) Install the Scons build tool
$conda install -c anaconda scons
2) Building libmlinwrf.so dynamic library using the Intel Fortran Compiler. The file SConstruct can be modified to build using a different compiler if desired but ifort is available in the users PATH by default on Cheyenne.
$scons
3) Set path to the shared library for compiling with WRF.
$export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$(pwd)

\[Cleaning code\]
$scons -c
