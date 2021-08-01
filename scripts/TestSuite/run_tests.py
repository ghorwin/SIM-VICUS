#!/usr/bin/env python3

# Solver test suite runner script, used for
# * regression tests (default)
# * test-init runs (with --test-init option)
# * performance evaluation (with --performance option)
#
# 1. Regression tests (the default)
# - runs set of projects and compares physical results and solver stats
# - meant to be run with either sequential or parallel solver
# - performance is monitored, but not so important (very short tests!)
# - expects jobs to have reference result directory, otherwise warning is issued
#   and simulation is skipped (with --run-always option all simulations are done even without
#   reference result dirs)
# - result of script:
#   for each job show old/new stats and metrics
#   show summary table with timings for all successful jobs
#   
# 2. Initialization tests
# - checks if solver can initialize set of project files
# - script parses directory structure, generates list of test-init jobs
#   and executes test initialization
# - result of script:
#   for each job result status and time needed for test init (only for information)
#   
# 3. Performance tests
# - collects list of jobs, runs each job 3 times and stores timings for all cases
# - result of script:
#   for each job print individual timings and best evalualtion time in a table
#
# License: 
#   BSD License
#
# Authors: 
#   Andreas Nicolai <andreas.nicolai@tu-dresden.de>
#
# Syntax:
# > python run_tests.py --path <path/to/testsuite> --solver <path/to/solver/binary> --extension <project file extension>
#
# Example:
# > python run_tests.py --path ../../data/tests --solver ./DelphinSolver --extension d6p
# > python run_tests.py -p ../../data/tests -s ./DelphinSolver -e d6p
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

	parser = argparse.ArgumentParser("run_tests.py")
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
	parser.add_argument('--test-init', dest="test_init", action='store_true', 
	                    help='Enables test-initialization mode (runs solvers with --test-init argument and '
	                    'skips result evaluation).')
	parser.add_argument('--performance', dest="performance", action='store_true', 
	                    help='Enables performance evaluation mode (runs solvers three times '
	                    'without result evaluation and dumps timings of all cases and best-of-three timings).')
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


def run_performance_evaluation(args, projects):
	# we basically do the same as the main script, but this time we run all test cases
	# whether they have reference results or not and simply remember the run times

	# we store evaluation times in a dictionary, key is the path to the project file,
	# value is a list of evaluation times
	eval_times = dict()

	failed_projects = []

	ITERATIONS = 3

	for iter in range(ITERATIONS):
		for project in projects:
			print(project)
			path, fname = os.path.split(project)
			#print "Path    : " + path
			#print "Project : " + fname

			cmdline = [args.solver, project]

			# try to read commandline file
			cmdlineFilePath = project + ".cmdline"
			if os.path.exists(cmdlineFilePath):
				fobj = open(cmdlineFilePath)
				cmdlineAddOn = fobj.readline()
				del fobj
				cmdline.append(cmdlineAddOn)
				print("Applying cmdline addon: " + cmdlineAddOn)

			try:
				# run solver 
				FNULL = open(os.devnull, 'w')
				if platform.system() == "Windows":
					cmdline.append("-x")
					cmdline.append("--verbosity-level=0")
					retcode = subprocess.call(cmdline, creationflags=subprocess.CREATE_NEW_CONSOLE)
				else:
					retcode = subprocess.call(cmdline, stdout=FNULL, stderr=subprocess.STDOUT)
				# check return code
				if retcode == 0:
					# read summary file
					resultsFolder = project[:-(1+len(args.extension))]
					# open stat files and compare them
					stats1 = SolverStats()
					if stats1.read(resultsFolder + "/log/summary.txt"):
						if not eval_times.has_key(project):
							eval_times[project] = []
						eval_times[project].append(stats1.timers['WallClockTime'])

				else:
					# mark project as failed
					failed_projects.append(project)
					# and print error message
					printError("Simulation failed, see screenlog file {}".format(os.path.join(os.getcwd(), 
					                                                                          resultsFolder+"/log/screenlog.txt"  ) ) )
			except OSError as e:
				printError("Error starting solver executable '{}', error: {}".format(args.solver, e))
				exit(1)


	print("\nSuccessful projects:\n")
	print("{:60s} {}".format("Project path", "Wall clock times [s], last column is min of all runs"))
	filenames = eval_times.keys()
	filenames.sort()
	perfstats = open(os.path.join(args.path, "performance_stats.txt"), 'w')
	for filename in filenames:
		fname = os.path.basename(filename)
		onedir = os.path.join(os.path.basename(os.path.dirname(filename)), os.path.basename(filename))
		s = "{:65s}".format(onedir)
		minVal = 1e20;
		for t in range(len(eval_times[filename])):
			duration = eval_times[filename][t]
			s = s + (" {:>10.3f}".format(duration))
			minVal = min(minVal, duration)
		s= s + ("     {:>10.3f}".format(minVal))
		printNotification(s)
		perfstats.write(s + '\n')
	del perfstats    
	if len(failed_projects) > 0:
		print("\nFailed projects:")
		for p in failed_projects:
			printError(p)
		print("\n")
		printError("*** Failure ***")
		exit(1)

	return 0




# *** main script ***

args = configCommandLineArguments()

if not args.no_colors:
	init() # init ANSI code filtering for windows
	config.USE_COLORS = True
	printNotification("Enabling colored console output")

if args.test_init and args.performance:
	printError("Either use --test-init or --performance, but not both together.")
	exit(1)

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
	print("Compiler ID            : " + compilerID)

print("Test suite             : " + args.path)
print("Solver                 : " + args.solver)
print("Project file extension : " + args.extension)

# walk all subdirectories (except .svn) within testsuite and collect project file names
projects = []
for root, dirs, files in os.walk(args.path, topdown=False):
	for name in files:
		if name.endswith('.'+args.extension):
			projectFilePath = os.path.join(root, name)
			projects.append(projectFilePath)

projects.sort()
print("Number of projects     : {}\n".format(len(projects)))

# performance tests?
if args.performance:
	res = run_performance_evaluation(args, projects)
	exit(res)

failed_projects = []
eval_times = dict() # key - file path to project, value - eval time in [s]

for project in projects:
	print(project)
	path, fname = os.path.split(project)
	#print("Path    : " + path)
	#print ("Project : " + fname)

	# compose path of result folder
	resultsFolder = project[:-(1+len(args.extension))]

	# remove entire directory with previous results
	if os.path.exists(resultsFolder):
		shutil.rmtree(resultsFolder)

	cmdline = [args.solver, project]
	# if in test-init mode, append --test-init to command line
	if args.test_init:
		cmdline.append("--test-init")
		skipResultCheck = True
		args.run_all = True
	else:
		skipResultCheck = False

	referenceFolder = resultsFolder + "." + compilerID
	if not os.path.exists(referenceFolder):
		if not args.run_all:
			failed_projects.append(project)
			printError("Missing reference data directory '{}'".format(os.path.split(referenceFolder)[1]))
			continue
		else:
			skipResultCheck = True

	try:
		# run solver 
		FNULL = open(os.devnull, 'w')
		if platform.system() == "Windows":
			cmdline.append("-x")
			cmdline.append("--verbosity-level=0")
			retcode = subprocess.call(cmdline, creationflags=subprocess.CREATE_NEW_CONSOLE)
		else:
			if args.test_init:
				# in test-init mode we want to see the output
				retcode = subprocess.call(cmdline)
			else:
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

print("\nSuccessful projects:\n")
print("{:80s} {}".format("Project path", "Wall clock time [s]"))
filenames = eval_times.keys()
filenames = sorted(filenames)
for filename in filenames:
	fname = os.path.basename(filename)
	onedir = os.path.join(os.path.basename(os.path.dirname(filename)), os.path.basename(filename))
	printNotification("{:80s} {:>10.3f}".format(onedir, eval_times[filename]))

if len(failed_projects) > 0:
	print("\nFailed projects:")
	for p in failed_projects:
		printError(p)
	print("\n")
	printError("*** Failure ***")
	exit(1)


printNotification("*** Success ***")
exit(0)


