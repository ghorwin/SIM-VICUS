# SIM-VICUS


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

Source code documentation is done with Doxygen, which generates its documentation for the _entire_ src directory and stores
the output in `docs/api`.


## Developer Info

### Coding Style

#### Files

- Line endings LR (Unix/Linux)
- UTF-8 encoding
- File name pattern:   `<lib>_<NameInCamelCase>.*`, for example: `IBK_ArgsParser.h` or `NANDRAD_Project.h`
- Header guards: `#ifndef <filenameWithoutExtension>H`, example: `#ifndef NANDRAD_ArgsParserH`

#### Namespaces

Each library has its own namespace, matching the file prefix. Example: `NANDRAD::Project` -> `NANDRAD_Project.h`

#### Class and variable naming

- camel case for variable/type names, example: `thisNiceVariable`
- type/class names start with capital letter, example: `MyClassType` 
- member variables start with `m_`, example: `m_myMemberVariableObject`
- getter/setter functions follow Qt-Pattern:

Example:
```
	std::string m_myStringMember;
	
	const std::string & myStringMember() const;
	void setMyStringMember(const std::string & str);
```
-> **never ever** write `getXXX` !!!!!

