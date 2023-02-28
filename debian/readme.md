# Debian package creation

## Debian/Ubuntu Distros

* Kinetic (22.10)
* Jammy (22.04 LTS)
* Impish (21.10)
* Focal (20.04 LTS)
* Bionic (18.04 LTS)

## Release Procedure

### First release for a new upstream version

- clone git repository
- extract current version number
- create a cleaned-up copy of the source code
- compress source tarball 

This is all done by a script:

```bash
# run in this directory
update_source_code_archive.sh
```
Then enter the distro-specific release directory and create the release:

```bash
cd ubuntu-20.04-focal
./newUpstreamVersion.sh
```

After creating the source package, you can locally build a binary package with:

```bash
./buildBinary.sh
```

### Releases for same upstream version, but different distro

- copy/download source code tarball into this directory
- extract source tarball like:

```bash
tar -xf simvicus-0.7.1.tar.xz
```

Then enter the distro-specific release directory and create the release
