# SIM-VICUS

We have a continuous integration scripts running (build-test of all C/C++ code without UI and solver regression tests). For details, see [Actions/build](../../actions).

| Test | Result|
|-----|-----|
| Continuous Integration (CI) (successfully building code) | ![CI](https://github.com/ghorwin/SIM-VICUS/workflows/CI/badge.svg)  |
| Solver regression test (correctly simulating test suite) | ![RegressionTest](https://github.com/ghorwin/SIM-VICUS/workflows/RegressionTest/badge.svg)  |



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


!!! **never ever** write `import namespace XXX`, not even for namespace `std` !!!!!!

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

### Exception handling

Basic rule:
- during initialization, throw `IBK::Exception` objects (and **only** `IBK::Exception` objects in **all code that uses the IBK library**) : reason: cause of exception becomes reasonably clear from the exception message string and context and this makes catch-and-rethrow-code so much easier (see below).
- **during calculation** (in parallel code sections), **avoid throwing Exceptions** (i.e. write code that cannot throw); in error cases (like div by zero), test explicitely for such failure conditions and leave function with error codes

When throwing exceptions:
- use function identifier created with `FUNCID()` macro:

```c++
void SomeClass::myFunction() {
    FUNCID(SomeClass::myFunction);
    
    ...
    throw IBK::Exception("Something went wrong", FUNC_ID);
}
```
Do not include function arguments in `FUNCID()`, unless it is important to distinguish between overloaded functions.

When raising exceptions, try to be verbose about the source of the exception, i.e. use `IBK::FormatString`:

```c++
void SomeClass::myFunction() {
    FUNCID(SomeClass::myFunction);
    
    ...
    throw IBK::Exception( IBK::FormatString("I got an invalid parameter '%1' in object #%2")
        .arg(paraName).arg(objectIndex), FUNC_ID);
}
```
See documentaition of class `IBK::FormatString` (and existing examples in the code).

#### Exception hierarchies

To trace the source of an error, keeping an exception trace is imported. When during simulation init you get an exception "Invalid unit ''" thrown from `IBK::Unit` somewhere, you'll have a hard time tracing the source (also, when this is reported as error by users and debugging isn't easily possible).

Hence, if you call a function that might throw, wrap it into a try-catch clause and throw on:

```c++
void SomeClass::myFunction() {
    FUNCID(SomeClass::myFunction);
    
    try {
        someOtherFunctionThatMightThrow(); // we might get an exception here
    }
    catch (IBK::Exception & ex) {          // we can rely on IBK::Exception here, since nothing else is allowed in our code
    
        // rethrow exception, but mind the prepended ex argument!
        throw IBK::Exception(ex, IBK::FormatString("I got an invalid parameter '%1' in object #%2")
            .arg(paraName).arg(objectIndex), FUNC_ID);
    }
}
```
The error message stack will then look like:

```
SomeClass::someOtherFunctionThatMightThrow    [Error]           Something went terribly wrong.
SomeClass::myFunction                         [Error]           I got an invalid parameter 'some parameter' in object #0815
```

That should narrow it down a bit.

### Avoiding memory leaks

NANDRAD creates model objects on the heap during initialization (never during solver runtime!). Since the model objects are first initialized before ownership is transferred, you should always ensure proper cleanup in case of init exceptions. Use code like:

```c++
std::unique_ptr<ModelObject> modelObject(new ModelObject);

modelObject->setup(...); // this may throw

m_modelContainer.push_back(modelObject.release()); // transfer ownership
```

### Documentation

Doxygen-style, prefer:

```c++
/*! Brief description of function.
    Longer multi-line documentation of function.
    \param arg1 The first argument.
    \param temperature A temperature in [C]
*/
void setParams(int arg1, double temperature);

/*! Mean temperature in [K]. */
double m_meanTemperature;
```

Mind to specify **always** physical units for physical value parameters and member variables!
Physical variables used for calculation should always be stored in base SI units.


### Git Workflow

Since we are a small team, and we want to have close communication of new features/code changes, and also short code-review cycles, we use a single development branch *master* with the following rules:

- CI is set up and ensures that after each push to *origin/master* the entire code builds without errors - so before pushing your changes, make sure the stuff builds
- commit/push early and often, this will avoid getting weird merge conflicts and possibly breaking other peoples code
- when pulling, use *rebase* to get a nice clean commit history (just as with subversion) - makes it easier to track changes and resolve errors arising in a specific commit (see solver regression tests)
- before pulling (potentially conflicting) changes from *origin/master*, commit all your local changes and ideally get rid of temporary files -> avoid stashing your files, since applying the stash may also give rise to conflicts and not everyone can handle this nicely
- resolve any conflicts locally in your working directory, and take care not to overwrite other people's code
- use different commits for different features so that later we can distingish based on commit logs when a certain change was made

For now, try to avoid (lengthy) feature branches. However, if you plan to do a larger change and possibly work on the master at the same time, feature branches are a good choice.

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

