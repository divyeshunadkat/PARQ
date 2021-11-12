PARQ
====

PARQ is an automatic parallelization engine for array modifying programs specified as a CHC system of constraints.

Installation
============

Compiles with gcc-7 (on Linux) and clang-1001 (on Mac). Requires <a href="https://gmplib.org/">GMP</a>, and <a href="https://www.boost.org/">Boost</a> and <a href="http://arma.sourceforge.net/">armadillo</a> packages to be pre-installed.

* `cd parq ; mkdir build ; cd build`
* `cmake ../`
* `make` to build dependencies (Z3)
* `make` (again) to build PARQ

The binary of PARQ can be found at `build/tools/parq/`.

Run `parq --h` for the usage information.

The tool prints `Success ...` if the system is parallelizable.

Benchmarks
==========

PARQ is expected to parallelize the SMT-LIB2 translations of the CHC systems in `bench_parq`.

