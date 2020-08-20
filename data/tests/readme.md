# Regression Tests

This directory contains small and fast regression tests, that each checks
an individual aspect of the solver implementation/project parametrization.

The tests in this directory should run very fast (< 1 s) and test individual aspects of
the solver.

The tests are done automatically (once a day) to check if changes in the code base
didn't accidentally break something.

The regression tests only successful simulation projects - correct error handling, for
example for invalid project files cannot be done by the regression test suite (so far).

## Procedure

To get deterministic results we only run the solver in sequential mode - OMP_NUM_THREADS = 1.

## Test criteria

Computed results must be **identical** to previous runs (that's why we cannot use parallel calculations).
**Identical** means:

- Solver statistics (counters) must be exactly the same, since any change in physical behavior
  will (alsmost) always change the counters for iterations/convergence and error test failures and steps.
  This is only true if the same platform/compiler is used for building and running the regression tests.
  Hence we store test results for different combinations and compiler and platform.
- Content of result files must be identical (bytewise identical).

Note: solver timings may be different (test suite will be run on different machines).

### Result storage

A test case `<project>.nandrad` will generate its results in subdirectory `<project>`. The reference
results are stored in directories `<project>.<compiler/platform suffix>` with:

- `gcc_linux` for linux OS and gcc as compiler
- `vc14_win64` for windows with Visual Studio 2015 (VC14) x64 builds on Windows 7/8/10 64-bit

Visual Studio builds are usually only needed for tests that check platform-specific features, like
path handling and/or timing code (anything that uses win API). Most regression tests shall be
written and defined for Linux and GCC.

The result directory has the same structure as the regular output directory, yet without the `vars`
subdir. The `logs` subdir should only contain the `summary.txt` file (needed for stats comparison).
Any `*.tsv` or `*.btf` files in the `results` subdirectory are used in the byte-wise comparison.

## Things to check

- all models and model variants
- output handling:
  + grids
  + None, Mean, Integral
  + output file grouping
- solver options:
  + different integrators
  + different LES solvers
  + different preconditioners


