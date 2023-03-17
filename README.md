# SIM-VICUS - NEXT LEVEL DISTRICT SIMULATION

SIM-VICUS is a 3D modeling environment for building and district networks and an innovative dynamic simulation engine (NANDRAD). https://sim-vicus.de hosts the official webpage with user-centered information. Development-related information is stored and handled on github.

![SIM-VICUS_SAB](https://www.sim-vicus.de/wp-content/uploads/2023/03/grafik-2048x1169.png)



## Quality Assurance

We have continuous integration scripts running (build-test of all C/C++ code) and solver regression tests. For details, see [Jenkins](https://baukli01.arch.tu-dresden.de/jenkins).

| Test | Result|
|-----|-----|
| CI - Linux 64-bit (Ubuntu 20.04.3 LTS; Qt 5.12.9) | [![Build Status](https://baukli01.arch.tu-dresden.de/jenkins/buildStatus/icon?job=NR-Linux-SimVicus&style=plastic)](https://baukli01.arch.tu-dresden.de/jenkins/job/NR-Linux-SimVicus/)    |
| CI - Windows 64-bit (Win10, VC 2019, Qt 5.15.2) | [![Build Status](https://baukli01.arch.tu-dresden.de/jenkins/buildStatus/icon?job=NR64-Win-SimVicus&style=plastic)](https://baukli01.arch.tu-dresden.de/jenkins/job/NR64-Win-SimVicus/)   |
| CI - MacOS 64-bit (10.11 "El Capitan", Qt 5.11.3) | _currently disabled...Mac releases are built on demand_ |
| Solver regression test (correctly simulating test suite) | [![Regression Test](https://baukli01.arch.tu-dresden.de/jenkins/buildStatus/icon?job=Daily-Test-SimVicus_NandradSolver&style=plastic&subject=Testsuite)](https://baukli01.arch.tu-dresden.de/jenkins/job/Daily-Test-SimVicus_NandradSolver/)   |

CI tests are located in directory `data/tests`. See [developer documentation](https://ghorwin.github.io/SIM-VICUS/Developer-Documentation/index.html) for information about the Python test suite scripts.

### Validation

The NANDRAD solver is thoroughly validated with the [SimQuality test suite](https://simquality.de) and other published validation test cases.
The validation suite(s) are in directory `data/validation` with summary given on the [Validation-Wiki-readme](https://github.com/ghorwin/SIM-VICUS/tree/master/data/validation/SimQuality) page.

## Repo-Documentation

The `README.md` files are written with MarkDown syntax, GitHub flavour, so that they can be read in the web-view of github.
All extensive documentation is written in AsciiDoctor format inside the doc directory.

Source code documentation is done with Doxygen, which generates its documentation for the _entire_ source code and stores
the output in `docs/api`. For this purpose run the `NandradSolver.doxyfile` in the `NandradSolver/doc` directory through Doxygen.

General documentation can be also found inside the [overview](https://ghorwin.github.io/SIM-VICUS/).

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
data                 - all kinds of data files, including test suite
doc                  - base directory for all documentation, see doc/README.md for details
docs                 - generated AsciiDoctor-based documentation, displayed in github-pages
externals            - libraries including third-party libs
FMUs                 - base directory for source code of FMUs that extend NANDRAD solver functionality 
lib_x64              - contains generated x64 libraries (nothing in here except .gitignore)
NandradCodeGenerator - the NANDRAD code generator (keyword list and serialization support)
NandradDevTests      - code snippeds used only during development/testing, not for production code, may not compile
NandradFMUGenerator  - GUI tool for FMU export configuration and FMU generation
NandradSolver        - sources for the NANDRAD command line solver
NandradSolverFMI     - sources for the NANDRAD Functional Mock-Up Interface module
scripts              - Python scripts (test suite etc.)
SIM-VICUS            - sources for the SIM-VICUS user interface
View3D               - View3D program from NIST
```

### Libraries

NANDRAD Solver and SIM-VICUS use a bunch of common libraries from IBK and external/third-party libraries:

* **CCM** - _Climate Calculation Module_ (CCM), implements climate data loading and calculation of solar radiation loads
* **clipper** - implements robust clipping calculation for two-dimensional polygons
* **RoomClipper** - implements robust clipping and connection generation via component instances of VICUS rooms and their surfaces
* **DataIO** - utility lib for writing DataIO output containers (needed when profiles across wall constructions are written)
* **IBK** - core utility library, extends c++ standard library by useful functions (so we do not need boost and other heavy libs)
* **IBKMK** - IBK math kernel lib, low level optimized routines for efficient number crunching
* **IDFReader** - library for IDF file parsing and data import to VICUS
* **IntegratorFramework** - library that includes several numerical integration libraries and provides a convenient framework to error-controlled time integration
* **ITSOL2** - implements ILUT preconditioner (included in IntegratorFramework, this directory contains the original sources)
* **Nandrad** - the NANDRAD data model library (holds the project data for the NANDRAD solver)
* **QtExt** - Qt extension library from IBK, lots of utility functions and widgets to assist with UI development
* **QuaZIP** - zip-support (needed for creating FMU archives)
* **qwt** - plotting library/charts
* **Shading** - implements shading factors calculation
* **SuiteSparse** - implements sparse direct solver KLU
* **sundials** - includes CVODE integrator, and GMRES and BiCGStab iterative les solvers
* **TiCPP** - TinyXML parser library, used for reading/writing XML files
* **Vicus** - SIM-VICUS data model library (holds the data model for the user interface)
* **Zeppelin** - graphing library (see [Graf Zeppelin](https://de.wikipedia.org/wiki/Ferdinand_von_Zeppelin) :-), needed to determine evaluation order of the NANDRAD model objects
