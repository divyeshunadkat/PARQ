PARQ
====


Installation
============

Compiles with gcc-7 (on Linux) and clang-1001 (on Mac). Assumes preinstalled <a href="https://gmplib.org/">GMP</a>, and Boost (libboost-system1.74-dev) packages. Additionally, armadillo package to get candidates from behaviors. 

* `cd aeval ; mkdir build ; cd build`
* `cmake ../`
* `make` to build dependencies (Z3)
* `make` (again) to build PARQ

The binary of PARQ can be found at `build/tools/parq/`.
Run `parq --help` for the usage info.

The tools print `Success ...` if the system is parallelizable.

Benchmarks
==========

Collection of the SMT-LIB2 translations of the CHC systems can be found at `bench_parq`. PARQQ is expected to parallelize these systems. 

