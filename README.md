# SIM-VICUS

![CI](https://github.com/ghorwin/SIM-VICUS/workflows/CI/badge.svg)

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
the output in `docs/api`.


## Developer Info

### Coding Style

#### Indentation and line length limit

- only tabs for indentation, shown in display as 4 spaces
- line length not strictly limited, but keep it below 120 (good for most screens nowadays)

#### Files

- Line endings LR (Unix/Linux)
- UTF-8 encoding
- File name pattern:   `<lib>_<NameInCamelCase>.*`, for example: `IBK_ArgsParser.h` or `NANDRAD_Project.h`
- Header guards: `#ifndef <filenameWithoutExtension>H`, example: `#ifndef NANDRAD_ArgsParserH`

#### Namespaces

Each library has its own namespace, matching the file prefix. Example: `NANDRAD::Project` get `NANDRAD_Project.h`

#### Class and variable naming

- camel case for variable/type names, example: `thisNiceVariable`
- type/class names start with capital letter, example: `MyClassType` 
- member variables start with `m_`, example: `m_myMemberVariableObject`
- getter/setter functions follow Qt-Pattern:

Example:
```c++
	std::string m_myStringMember;
	
	const std::string & myStringMember() const;
	void setMyStringMember(const std::string & str);
```
!!! **never ever** write `getXXX` !!!!!

#### Documentation

Doxygen-style, prefer:

```c++
    /*! Brief description of function.
        Longer multi-line documentation of function.
	\param arg1 The first argument.
	\param temperature A temperature in [C]
    */
    void setParams(int arg1, double temperature);
    
    /*! Mean temperature in [C]. */
    double m_meanTemperature;
```

Mind to specify **always** physical units for physical value parameters and member variables!


### Libraries

NANDRAD uses a bunch of external/third-party libraries:

* IBK - core utility library, extends c++ standard library by useful functions (so we do not need boost and other heavy libs)
* IBKMK - IBK math kernel lib, low level optimized routines for efficient number crunching
* DataIO - utility lib for writing DataIO output containers (needed when profiles across wall constructions are written)
* CCM - _Climate Calculation Module_ (CCM), implements climate data loading and calculation of solar radiation loads
* TiCPP - TinyXML parser library, used for reading/writing XML files
* Zeppelin - graphing library (see [Graf Zeppelin](https://de.wikipedia.org/wiki/Ferdinand_von_Zeppelin) :-), needed to determine evaluation order
* zlib - generation of zip files, needed for FMU export
* SuiteSparse - implements sparse direct solver KLU
* sundials - includes CVODE integrator, and GMRES and BiCGStab iterative les solvers
* ITSOL2 - implements ILUT preconditioner
* IntegratorFramework - library that integrates all numerical library and provides a convenient framework to error-controlled time integration
* NANDRAD - the data model library (project handling)

