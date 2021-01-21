# SIM-VICUS

We have a continuous integration scripts running (build-test of all C/C++ code without UI) and solver regression tests. For details, see [Jenkins](https://baukli01.arch.tu-dresden.de/jenkins).

| Test | Result|
|-----|-----|
| CI - Linux 64-bit (Ubuntu 18.04) | ![CI](https://baukli01.arch.tu-dresden.de/jenkins/job/NR-Linux-SimVicus-Trunk/badge)  |
| CI - Windows 64-bit (Win10, VC14) | ![CI](https://baukli01.arch.tu-dresden.de/jenkins/job/NR64-Win-SimVicus-Trunk/badge)  |
| CI - MacOS 64-bit (10.11 "El Capitan") | [![Build Status](https://baukli01.arch.tu-dresden.de/jenkins/buildStatus/icon?job=NR-IOS-SimVicus-Trunk)](https://baukli01.arch.tu-dresden.de/jenkins/job/NR-IOS-SimVicus-Trunk/) |
| Solver regression test (correctly simulating test suite) | ![RegressionTest](https://baukli01.arch.tu-dresden.de/jenkins/view/Daily-Tests/job/Daily-Test-SimVicus_Nandrad2/badge)  |

Tests are located in `data/tests`, the validation suite(s) are in `data/validation` with summary given on the [Validation-Wiki-readme](https://github.com/ghorwin/SIM-VICUS/tree/master/data/validation/SimQuality) page.

## Repo-Documentation

The `README.md` files are written with MarkDown syntax, GitHub flavour, so that they can be read in the web-view of github.
All extensive documentation is written in AsciiDoctor format inside the doc directory.

Source code documentation is done with Doxygen, which generates its documentation for the _entire_ source code and stores
the output in `docs/api`. For this purpose run the `NandradSolver.doxyfile` in the `NandradSolver/doc` directory through Doxygen.


## Mailing List 

Community communication is handled through our mailing list:

https://www.listserv.dfn.de/sympa/info/sim-vicus


## Developer Info

Any developer might (=should) read the developer documentation in:

https://ghorwin.github.io/SIM-VICUS/Developer-Documentation

### Directory Structure

```
bin                  - binaries
build                - build scripts and session projects
data                 - all kinds of data files
doc                  - base directory for all documentation, see doc/README.md for details
docs                 - generated AsciiDoctor-based documentation, displayed in github-pages
externals            - libraries including third-party libs
NandradCodeGenerator - the NANDRAD code generator (keyword list and serialization support)
NandradSolver        - sources for the NANDRAD command line solver
SIM-VICUS            - sources for the SIM-VICUS user interface
scripts              - Python scripts (test suite etc.)
```

### Libraries

NANDRAD uses a bunch of external/third-party libraries:

* **CCM** - _Climate Calculation Module_ (CCM), implements climate data loading and calculation of solar radiation loads
* **DataIO** - utility lib for writing DataIO output containers (needed when profiles across wall constructions are written)
* **IBK** - core utility library, extends c++ standard library by useful functions (so we do not need boost and other heavy libs)
* **IBKMK** - IBK math kernel lib, low level optimized routines for efficient number crunching
* **IntegratorFramework** - library that includes several numerical interation libraries and provides a convenient framework to error-controlled time integration
* **ITSOL2** - implements ILUT preconditioner (included in IntegratorFramework)
* **Nandrad** - the data model library (project handling)
* **SuiteSparse** - implements sparse direct solver KLU
* **sundials** - includes CVODE integrator, and GMRES and BiCGStab iterative les solvers
* **TiCPP** - TinyXML parser library, used for reading/writing XML files
* **Zeppelin** - graphing library (see [Graf Zeppelin](https://de.wikipedia.org/wiki/Ferdinand_von_Zeppelin) :-), needed to determine evaluation order
* **zlib** - generation of zip files, needed for FMU export

Libraries related to UI development:

* **QtExt** - Qt extension library
* **QuaZIP** - zip file support library
* **qwt** - plotting library
* **Vicus** - the data model for the user interface and overall geometry/BIM data

