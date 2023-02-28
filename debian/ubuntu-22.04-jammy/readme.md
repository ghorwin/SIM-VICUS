# Steps to update a package

Update version info in changelog by running

```bash
dch
```

```bash
$ ../scripts/newUpstreamVersion.sh
```

When prompted to upload source package, stop on first run with CTRL+C.
Then run

```bash
$ ../scripts/buildBinary.sh
```

and check compilation.

Afterwards re-run 

```bash
$ ../scripts/newUpstreamVersion.sh
```

and this time upload the files.
