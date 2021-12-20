# Functional Mock-Up Units to extend NANDRAD solver functionality

This directory contains the source codes for generic FMUs, that can be used in conjunction
with the NANDRAD FMU to extend the built-in functionality.

The subdirectories have the structure as regular projects:

```
FMUs/
  CO2ComfortVentilation/
    /projects
    /src
    /doc
```

FMUs are expected to be parametrized through project files and/or the `modelDescription.xml`.  Hence, the
individual FMU binaries shall be usable for several SIM-VICUS/NANDRAD-Projects.

