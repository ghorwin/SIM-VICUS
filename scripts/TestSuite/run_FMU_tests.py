#!/usr/bin/env python

# FMU export and co-simulation test script.
# Specialized for NANDRAD and MasterSim
# Requires MASTERSIM_PATH as environment variable.
#
# FMU test suites (--path) require in each subdirectory:
# - exactly one msim file
# - one or more nandrad files (all of these will be converted to fmus)
# 
# Path to binaries (--binaries) defines path to NandradSolver and NandradFMUGenerator,
# and also libNandradSolverFMI.so.
# 
# License: 
#   BSD License
#
# Authors: 
#   Andreas Nicolai <andreas.nicolai@tu-dresden.de>
#
# Syntax:
# > python run_FMU_tests.py --path <path/to/testsuite>


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

	parser = argparse.ArgumentParser("run_tests.py")
	parser.description = '''
Runs the regression test suite. Can be used for init-tests (--test-init) 
or performance evaluation (--performance) as well.'''

	parser.add_argument('-p', '--path', dest='path', required=True, type=str, 
	                    help='Path to FMU test suite root directory.')
	parser.add_argument('-b', '--binaries', dest='binaries', required=True, type=str, 
	                    help='Path to binaries.')
	parser.add_argument('--no-colors', dest="no_colors", action='store_true', 
	                    help='Disables colored console output.')
	parser.add_argument('--run-all', dest="run_all", action='store_true', 
	                    help='If set (in regression test mode), also the test cases without reference results '
	                    'are simulated (can be used to generate reference results for all cases).')

	return parser.parse_args()


def checkResults(dir1, dir2, evalTimes):
	"""
	Compares two result directories for equal contents.

	Compared are:

	- physical results
	- solver counters (/log/summary.txt)

	This function uses IBK.SolverStats

	Arguments:
	* dir1 (reference results) and dir2 (computed results)
	* evalTimes is a dictionary with filepath (key) and wall clock time (value), 
	  new entries are always added to the dictionary


	Returns: True on success, False on error
	"""
	try:
		# open stat files and compare them
		stats1 = SolverStats()
		if not stats1.read(dir1 + "/log/summary.txt"):
			return False
		stats2 = SolverStats()
		if not stats2.read(dir2 + "/log/summary.txt"):
			return False

		if not SolverStats.compareStats(stats1, stats2, []):
			printError("Mismatching statistics.")
			return False

		# compare all result files (d60, tsv), if any reference result files exist
		if os.path.exists(dir1 + "/results"):
			if not SolverStats.compareResults(dir1 + "/results", dir2 + "/results"):
				printError("Mismatching values.")
				return False
		evalTimes[dir2] = stats2.timers['WallClockTime']
	except Exception as e:
		printError("Error comparing simulation results, error: {}".format(e))
	return True




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
	compilerID = "VC14_win64"

elif currentOS == "Darwin" :
	compilerID = "gcc_mac"

if compilerID == None:
	printError("Unknown/unsupported platform")
	exit(1)
else:
	print "Compiler ID            : " + compilerID

args.path = os.path.abspath(args.path)
print "Test suite             : " + args.path
print "Binaries directory     : " + args.binaries

if not 'MASTERSIM_PATH' in os.environ:
	printError("Environment variable 'MASTERSIM_PATH' missing.")
	exit(1)
MASTERSIM_BIN = os.environ['MASTERSIM_PATH']
print "MasterSimulator        : " + MASTERSIM_BIN

NANDRADFMUGEN_BIN = os.path.join(args.binaries, 'NandradFMUGenerator')
print "NandradFMUGenerator    : " + NANDRADFMUGEN_BIN

# walk all subdirectories (except .svn) within testsuite and collect project file names
projects = []
for root, dirs, files in os.walk(args.path, topdown=False):
	for name in files:
		if name.endswith('.msim'):
			projectFilePath = os.path.join(root, name)
			projects.append(projectFilePath)

projects.sort()
print("Number of MSIM projects : {}".format(len(projects)))
print "\n"

failed_projects = []
eval_times = dict() # key - file path to project, value - eval time in [s]

for project in projects:
	print project
	path, fname = os.path.split(project)
	#print "Path    : " + path
	#print "Project : " + fname

	# compose path of result folder
	resultsFolder = project[:-(1+len('msim'))]

	# remove entire directory with previous results
	if os.path.exists(resultsFolder):
		shutil.rmtree(resultsFolder)
	
	# 'path' contains msim and several nandrad files
	nandradFiles = [f for f in os.listdir(path) if os.path.isfile(os.path.join(path, f)) and f.endswith(".nandrad")]

	# now generate all FMUs for all nandrad projects
	fmuGenerationFailed = False
	for np in nandradFiles:
		fmuBaseName = np[:-(1+len('nandrad'))]
		fmuPath = os.path.join(path, fmuBaseName + ".fmu")
		printNotification('Generating {}'.format(fmuPath))
		cmdline = [NANDRADFMUGEN_BIN, '--generate=' + fmuBaseName, os.path.join(path, np)]
		try:
			# run solver 
			FNULL = open(os.devnull, 'w')
			if platform.system() == "Windows":
				retcode = subprocess.call(cmdline, creationflags=subprocess.CREATE_NEW_CONSOLE, cwd=args.binaries)
			else:
#				retcode = subprocess.call(cmdline, cwd=args.binaries)
				retcode = subprocess.call(cmdline, stdout=FNULL, stderr=subprocess.STDOUT, cwd=args.binaries)
			# check return code
			if not retcode == 0:
				# mark project as failed
				failed_projects.append(project)
				# and print error message
				printError("Generation of NANDRAD FMU '{}' failed.".format(fmuPath))
				fmuGenerationFailed = True
		except OSError as e:
			printError("Error starting NandradFMUGenerator executable '{}', error: {}".format(args.binaries + '/NandradFMUGenerator', e))
			exit(1)
			
	if fmuGenerationFailed:
		# mark project as failed
		failed_projects.append(project)
		continue # next msim-project
	

	skipResultCheck = False

	referenceFolder = resultsFolder + "." + compilerID
	if not os.path.exists(referenceFolder):
		if not args.run_all:
			failed_projects.append(project)
			printError("Missing reference data directory '{}'".format(os.path.split(referenceFolder)[1]))
			continue
		else:
			skipResultCheck = True # no ref dir available, cannot check for results

	try:
		# run co-simulation master 
		
		cmdline = [MASTERSIM_BIN, project]
		
		FNULL = open(os.devnull, 'w')
		if platform.system() == "Windows":
			cmdline.append("-x")
			cmdline.append("--verbosity-level=0")
			retcode = subprocess.call(cmdline, creationflags=subprocess.CREATE_NEW_CONSOLE)
		else:
#			retcode = subprocess.call(cmdline)
			retcode = subprocess.call(cmdline, stdout=FNULL, stderr=subprocess.STDOUT)
		# check return code
		if retcode == 0:
			# successful run
			if not skipResultCheck:
				# now check against reference results
				if not checkResults(referenceFolder, resultsFolder, eval_times):
					if not project in failed_projects:
						failed_projects.append(project) # mark as failed
					printError("Mismatching results.")
		else:
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
print "{:80s} {}".format("Project path", "Wall clock time [s]")
filenames = eval_times.keys()
filenames.sort()
for filename in filenames:
	fname = os.path.basename(filename)
	onedir = os.path.join(os.path.basename(os.path.dirname(filename)), os.path.basename(filename))
	printNotification("{:80s} {:>10.3f}".format(onedir, eval_times[filename]))

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


