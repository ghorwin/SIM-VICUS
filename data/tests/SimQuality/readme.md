# Regression Tests

The tests in this directory should run very fast (< 1 s) and test individual aspects of
the solver.

The tests are done automatically (once a day) to check if changes in the code base
didn't accidentally break something.

The regression tests only successful simulation projects - correct error handling, for
example for invalid project files cannot be done by the regression test suite (so far).

## Procedure

To get deterministic results we only run the solver in sequential mode - OMP_NUM_THREADS = 1.

Results

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

