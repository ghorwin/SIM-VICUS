# SIM-VICUS

We have a continuous integration scripts running (build-test of all C/C++ code without UI) and solver regression tests. For details, see [Actions/build](../../actions).

| Test | Result|
|-----|-----|
| Continuous Integration (CI) (successfully building code) | ![CI](https://github.com/ghorwin/SIM-VICUS/workflows/CI/badge.svg)  |
| Solver regression test (correctly simulating test suite) | ![RegressionTest](https://github.com/ghorwin/SIM-VICUS/workflows/RegressionTest/badge.svg)  |

Tests are located in `data/tests`, the validation suite(s) are in `data/validation` with summary given on the [Validation-Wiki-readme](https://github.com/ghorwin/SIM-VICUS/tree/master/data/validation/SimQuality) page.

## Directory Structure

```
bin           - binaries
build         - build scripts and session projects
data          - all kinds of data files
doc           - base directory for all documentation, see doc/README.md for details
docs          - generated AsciiDoctor-based documentation, displayed in github-pages
externals     - libraries including third-party libs
NandradSolver - sources for the NANDRAD command line solver
```


## Repo-Documentation

The `README.md` files are written with MarkDown syntax, GitHub flavour, so that they can be read in the web-view of github.
All extensive documentation is written in AsciiDoctor format inside the doc directory.

Source code documentation is done with Doxygen, which generates its documentation for the _entire_ source code and stores
the output in `docs/api`. For this purpose run the `NandradSolver.doxyfile` in the `NandradSolver/doc` directory through Doxygen.


## Developer Info

Any developer might (=should) read the developer documentation in:

https://ghorwin.github.io/SIM-VICUS/Developer-Documentation

### Libraries

NANDRAD uses a bunch of external/third-party libraries:

* **IBK** - core utility library, extends c++ standard library by useful functions (so we do not need boost and other heavy libs)
* **IBKMK** - IBK math kernel lib, low level optimized routines for efficient number crunching
* **DataIO** - utility lib for writing DataIO output containers (needed when profiles across wall constructions are written)
* **CCM** - _Climate Calculation Module_ (CCM), implements climate data loading and calculation of solar radiation loads
* **TiCPP** - TinyXML parser library, used for reading/writing XML files
* **Zeppelin** - graphing library (see [Graf Zeppelin](https://de.wikipedia.org/wiki/Ferdinand_von_Zeppelin) :-), needed to determine evaluation order
* **zlib** - generation of zip files, needed for FMU export
* **SuiteSparse** - implements sparse direct solver KLU
* **sundials** - includes CVODE integrator, and GMRES and BiCGStab iterative les solvers
* **ITSOL2** - implements ILUT preconditioner
* **IntegratorFramework** - library that includes several numerical interation libraries and provides a convenient framework to error-controlled time integration
* **NANDRAD** - the data model library (project handling)

