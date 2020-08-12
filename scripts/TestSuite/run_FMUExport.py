#!/usr/bin/env python

# Solver test suite runner script, used for FMU export
#
# License: 
#   BSD License
#
# Authors: 
#   Andreas Nicolai <andreas.nicolai@tu-dresden.de>
#
# Syntax:
# > python run_FMUEXport.py --path <path/to/testsuite> --solver <path/to/solver/binary> --extension <project file extension>
#
# Example:
# > python run_FMUEXport.py --path ../../data/tests --solver ./NandradSolver --extension nandrad
#
# Returns:
# 0 - if all tests could be simulated successfully and if all solver results/metrics match those of reference results
# 1 - if anything failed
#
# Note: if run with --run-all option, test cases without reference results will always be accepted.
#

import subprocess		# import the module for calling external programs (creating subprocesses)
import sys
import os
import os.path
import shutil
import filecmp          # for result file comparison
import argparse
import platform         # to detect current OS

from colorama import *
from SolverStats import *
from print_funcs import *
from config import USE_COLORS


def configCommandLineArguments():
    """
    This method sets the available input parameters and parses them.

    Returns a configured argparse.ArgumentParser object.
    """

    parser = argparse.ArgumentParser("run_FMUExport.py")
    parser.description = '''
Runs the regression test suite. Can be used for init-tests (--test-init) 
or performance evaluation (--performance) as well.'''

    parser.add_argument('-p', '--path', dest='path', required=True, type=str, 
                        help='Path to test suite root directory.')
    parser.add_argument('-s', '--solver', dest='solver', required=True, type=str, 
                        help='Path to solver binary.')
    parser.add_argument('-e', '--extension', dest="extension", required=True, type=str, 
                        help='Project file extension.')
    parser.add_argument('--no-colors', dest="no_colors", action='store_true', 
                        help='Disables colored console output.')
    
    return parser.parse_args()


# *** main script ***

args = configCommandLineArguments()

if not args.no_colors:
    init() # init ANSI code filtering for windows
    config.USE_COLORS = True
    printNotification("Enabling colored console output")

# process all directories under test suite directory
currentOS = platform.system()
compilerID = None
if currentOS   == "Linux" :
    compilerID = "gcc_linux"

elif currentOS == "Windows" :
    compilerID = "win_VC14"

elif currentOS == "Darwin" :
    compilerID = "gcc_mac"

if compilerID == None:
    printError("Unknown/unsupported platform")
    exit(1)
else:
    print "Compiler ID            : " + compilerID

print "Test suite             : " + args.path
print "Solver                 : " + args.solver
print "Project file extension : " + args.extension
print "\n"

# walk all subdirectories (except .svn) within testsuite and collect project file names
projects = []
for root, dirs, files in os.walk(args.path, topdown=False):
    for name in files:
        # exclude unpacked fmu project files 
        fpath = os.path.join(root, name)
        dirpath = os.path.basename(os.path.dirname(fpath))
        dirname = os.path.basename(dirpath)
        if dirpath == "resources":
            continue
        if name.endswith('.'+args.extension):
            projectFilePath = os.path.join(root, name)
            projects.append(projectFilePath)

projects.sort()


failed_projects = []
eval_times = dict() # key - file path to project, value - eval time in [s]

for project in projects:
    print project
    path, fname = os.path.split(project)
    #print "Path    : " + path
    #print "Project : " + fname

    cmdline = [args.solver, project]
 
	# if in fmu-export mode, append --fmu-export to command line
    fmuBaseName = os.path.splitext(fname)[0]
    fmuName = os.path.join(path, fmuBaseName + "." + "fmu")
    cmdline.append("--fmu-export=" + fmuName )
    
    cmdline.append("--fmu-export=" + fmuName )
    
    # compose path of result folder
    resultsFolder = project[:-(1+len(args.extension))]

    try:
        # run solver 
        FNULL = open(os.devnull, 'w')
        if platform.system() == "Windows":
            cmdline.append("-x")
            cmdline.append("--verbosity-level=0")
            retcode = subprocess.call(cmdline)
        else:
            retcode = subprocess.call(cmdline, stdout=FNULL, stderr=subprocess.STDOUT)
        # check return code
        if retcode != 0:
            # mark project as failed
            failed_projects.append(project)
            # and print error message
            printError("Simulation failed, see screenlog file {}".format(os.path.join(os.getcwd(), 
                    resultsFolder+"/log/screenlog.txt"  ) ) )
    except OSError as e:
        printError("Error starting solver executable '{}', error: {}".format(args.solver, e))
        exit(1)

print ""
print "Successful projects:"
print ""
print "{:60s} {}".format("Project path", "Wall clock time [s]")
filenames = eval_times.keys()
filenames.sort()
for filename in filenames:
    fname = os.path.basename(filename)
    onedir = os.path.join(os.path.basename(os.path.dirname(filename)), os.path.basename(filename))
    printNotification("{:65s} {:>10.3f}".format(onedir, eval_times[filename]))

if len(failed_projects) > 0:
    print ""
    print "Failed projects:"
    for p in failed_projects:
        printError(p)
    print ""
    printError("*** Failure ***")
    exit(1)
    

printNotification("*** Success ***")
exit(0)


