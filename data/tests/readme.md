# Regression Tests

*Written by Andreas Nicolai. Mail me, if you have questions! In any case, take regression testing seriously!*

## Overview

This directory contains small and fast regression tests, that each checks
an individual aspect of the solver implementation/project parametrization.

The tests in this directory should run very fast (< 1 s) and test individual aspects of
the solver.

The tests are done automatically (once a day) to check if changes in the code base
didn't accidentally break something.

The regression tests only successful simulation projects - correct error handling, for
example for invalid project files cannot be done by the regression test suite (so far).

## Test procedure

### Jenkins Automatic Regressions Test Runs

Our Jenkins Build Service runs regression tests daily. It starts the script 

```
SIM-VICUS/build/cmake/run_tests.sh
```

which runs the *Linux* solver and compare the newly generated results with the stored
results with suffix (`gcc_linux`, see below). The script itselfs searches recursively
for `.nandrad` files and executes the solver with them.

*Tipp*: If you want to deactivate a test, just rename the project file's extension.

### Regression Test Success Criteria

Computed results must be **identical** to previous runs (that's why we cannot use parallel calculations).
**Identical** means:

- *Solver statistics (counters)* must be exactly the same, since any change in physical behavior
  will (almost) always change the counters for iterations/convergence and error test failures and steps.
  This only works if the *same platform/compiler* is used for building and running the regression tests.
  Hence we store test results for different combinations and compiler and platform (see below).
  
- Content of result files must be identical (bytewise identical).

*Note:* solver timings may be different (test suite will be run on different machines). This is not a failure criterion but *excessive differences* in simulation durations may hint on something nasty going on inside the numerical engine. Sometimes something as stupid as a `IBK_Message` inside an inner loop or a needless string operation.

**Important:** To get deterministic results we only run the solver in sequential mode, i.e. with:

```bash
# either set environment variable OMP_NUM_THREADS = 1 in shell, or run test suite with
> OMP_NUM_THREADS = 1  ./run_tests.sh
```

### Same Compiler and OS Version

Both the operating system and thus installed math/c/c++ runtime libraries, as well as the compiler that generates
the NANDRAD solver executable impact the numeric results of a solver and thus also the 
solver results that are compared during regression tests.

Hence, **you need to use always the same compiler and OS** to generate and check regression tests.
As of now, this is `gcc version 9.4.0 (Ubuntu 9.4.0-1ubuntu1~20.04.1)` (GCC is installed as Ubuntu package and thus linked
to the Ubuntu version).

**Important:** After an OS update, you may need to update all regression test results. It is important that this OS-update-related 
change of reference results is *never* mixed up with regression test result updates due to development.


## Adding Test Cases

When adding or updating regression tests, you need to be *very careful*, otherwise
we risk to verify programming/modelling errors by accidentally submitting wrong test results.

Hence, please ***strictly*** follow the regression test procedure described below. Be aware that creating good regression tests takes time - but this is time well spent since regression tests are the *key critical quality assurance method* to guarantee a reliable model solver!

### Designing a new Regression Test

Regression tests are simple, limited tests of an _individual_ isolated model feature/solver capability. Avoid dumping example projects or real life projects
as regression test. A good regression test checks for a single or only a few related features, and should be named accordingly. 
The execution time of a regression test **should never exceed 0.5 seconds.** 

*Rationale:* we will have eventuelly hundrets of test cases and when we attempt to find and fix regression test errors due to code changes we need to run the test suite many many times.

The regression test should be configured to generate a limited, but meaningful set of outputs. It is advised to restrict the simulation duration
to cover only the time frame needed to test the model.  If simulation statistics differ, the differences between stored and calculated results should assist in finding clues on what code change caused the discrepancy. Therefore, add results sparsingly, but with the potential help they offer in mind!

*Rationale:* the git repository is pulled *every night* due to the automatic build service. To reduce network traffic and speed up the procedure, the entire repository and all files should be kept small. Hence, avoid pushing large files to the repository - they all add up!

**Tipp:** When you have a test case with very frequent outputs to generate a nice comparison plot, modify the project to have longer output intervals for the purpose of the regression test. When checking for errors later, you can always increase the output frequency again - but for automated regression checks fewer outputs may suffice.

#### Basic guidelines

- *short execution time*
- *few meaningful results* and *small reference result files (< 200kB)*

### Reference Results

The NANDRAD solver will generate the results for a project `<project>.nandrad` in subdirectory `<project>`. In order to register a new
regression test, you need to rename this subdirectory first.

The reference results are stored in directories `<project>.<compiler/platform suffix>` with:

- `gcc_linux` for linux OS and gcc as compiler
- `VC2019_win64` for windows with Visual Studio 2019 x64 builds on Windows 10 64-bit

*Note:* Visual Studio builds are usually only needed for tests that check platform-specific features, like
path handling and/or timing code (anything that uses win API). Also, there are no automatic
Windows regression tests.

Hence, regression tests **must always be written and defined for Linux and GCC** in the exact same version as installed on the Jenkins regression test server.

### Reference Result Directory Content

The result directory has the same structure as the regular output directory, yet without the `var`
subdir. The `log` subdir should only contain the `summary.txt` file (needed for stats comparison).
Any `*.tsv` or `*.btf` files in the `results` subdirectory are used in the byte-wise comparison.

**Important:** In order not to clutter the git-repository, *never* commit any other files from the `var` directory or 
large number of result files. Also clean out all files in `log` except  `summary.txt`. Then commit these files to the git repository.

Then create or edit the `.gitignore` file in the *same directory as* the `<project>.nandrad>` file. This `.gitignore` file should
contain the project name as ignore pattern.

Example:

```
./tests/LongWaveRadiation/LongWaveRadiation.nandrad                       -  the project file
./tests/LongWaveRadiation/LongWaveRadiation                               -  output directory of simulation results generated during regression test runs
./tests/LongWaveRadiation/LongWaveRadiation.gcc_linux/                    -  the reference results for Linux/GCC
./tests/LongWaveRadiation/LongWaveRadiation.gcc_linux/log/summary.txt     -  solver stats
./tests/LongWaveRadiation/LongWaveRadiation.gcc_linux/results/states.tsv  -  sim results
./tests/LongWaveRadiation/.gitignore                                      -  GIT ignore file which ignores temp. directory LongWaveRadiation

---
/LongWaveRadiation/
---
```

**Important:** Do not commit `.gitignore` files in subdirectories `log`, `results` or `var`! Rather ignore the entire temporary project result directory, as shown in the example above.

## Updating Regression Tests

Regression test results need to be updated when the solver has evolved or changed (for example output names or precision of output has changed, or some numerical feature was added resulting in different statistics/results). Or, when a bug was fixed and as a consequence the results differ.

Also possible: the test case definition itself was modified, to test or improve a specific feature.

In all those cases there will be difference between checked-in reference results and newly generated results.

### Visualizing the Differences

First you need to run the test suite. Hereby, **you must use the same GCC and Ubuntu**-Version as on the Jenkins slave. If you're using Windows, install Ubuntu in a VirtualBox environment which you keep in sync with the Jenkins' slave's versions!

Run the script 

```bash
./build/cmake/run_tests.sh
```

to execute the test suite locally. This will generate the temporary project result directories.

The easiest way to check for differences is to simply update the reference results with the newly computed results *in the local working copy only*. For this purpose run the script 

```
./data/tests/update_refresults.sh
```

which will copy all modified files over to the files existing already in the reference result subdirs. You can then use *SmartGit* or *Meld* or plain *git* to show the differences in the * `/data/tests/*` directory. Use this to evaluate the impact of your code/test case changes.

### Updating the Reference Results in the Repository

**DO NOT** directly commit these changes. First, perform a thorough review and then commit the changes with a long and descriptive explanation on *why the results have been updated*.
If you had different changes made in the solver simultaneously, and these changes cause different test cases to differ because of different reasons - commit these individually and use a separate description message.


## Regression Test Failures and Searching for the Reason

Monitoring regression test failures is important. You can import the RSS-Feed from Jenkins https://baukli01.arch.tu-dresden.de/jenkins/view/Daily-Tests/rssFailed into Thunderbird to get informed about new failures.

When a failure occurs, check the git commit log and see what changes have been made on the day that the test first failed. Depending on the traffic on the day, you can usually quickly identify a commit that caused the problem (hence, good commit messages are a really good idea!).

Check with the developer (or with yourself), why this change caused a difference in the regression test. If there is a plausible explanation, you can update the reference results as described above. Otherwise check out the code and debug it, until you find the reason for the differences.

**Be careful**, not to dismiss changes too quickly. If you are in a hurry, come back later and checkup on regression tests in due time. Do not rush anything, otherwise a small change checked in by mistake may give you much debugging hassles in the future!


### Too many Days of Failures for the Jenkins History

If, for some strange reason, you failed to fix regression tests before the history in Jenkins is full and no successful run is still in the list, you need to search for the commit that caused the regression test. Jenkins shows on the summary page

https://baukli01.arch.tu-dresden.de/jenkins/view/Daily-Tests/

the last successful run. You can use this to go back in time and checking out the respective git commits on this day until you find one that runs the test suite successfully. Once you find the commit that introduced the regression, start identifying the bug/change that caused it. 

If you found a bug, **DO NOT** commit this with the currently checked-out repository position. Rather, check out the HEAD again, apply the fix there and make sure that the bug fix results in a fully working test suit.

**WARNING**: if you neglect regression tests for too long, you may have several commits that introduce different changes/bugs. In this case it can be **very time consuming** to identify and fix the individuall changes needed to fix the code. Hence, try to fix regression tests as soon as they appear (this saves you a lot of work, trust me!).



After all remember **A regression test is your friend!** and not a necessary evil, at least if you want to trust in your simulation results! :-)


