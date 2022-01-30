# Debian package creation

## Release Procedure

### First release for a new upstream version

- clone git repository
- extract current version number
- create a cleaned-up copy of the source code
- compress source tarball 

This is all done by a script:

```bash
# ran in this directory
scripts/update_source_code_archive.sh
```

Then enter the distro-specific release directory and create the release

### Releases for same upstream version, but different distro

- copy/download source code tarball into this directory
- extract source tarball like:

```bash
tar -xf simvicus-0.7.1.tar.xz
```

Then enter the distro-specific release directory and create the release
