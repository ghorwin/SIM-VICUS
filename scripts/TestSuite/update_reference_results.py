#!/usr/bin/env python3

# Script to update all files within reference directories (with given suffix) from currently computed cases
#
# > update_reference_results.py <directory_suffix>


import os
import os.path
import optparse
import shutil

from update_reference_results_utils import copyIfNotEqualD6O
from update_reference_results_utils import copyIfNotEqualSummary
from update_reference_results_utils import processCaseDir

HELPTEXT = """
Syntax: update_reference_results.py <suffix>

Run this script in the directory of a single test case, for example 'data/tests/EN_10211_No1'.
The script will search for subdirectories with the given suffix, for example
'gcc_linux' and copy newly generated results over reference data files.
For each file replaced the script will print a line with the updated relative file path.
"""

# *** main ***

# setup command line parser
parser = optparse.OptionParser()
(options, args) = parser.parse_args()

if len(args) < 1:
	print(HELPTEXT)
	exit(1)

suffix = args[0]
print("Updating reference results for suffix '{}'".format(suffix))

# process all subdirectories with given suffix
rootDir = os.getcwd()
print("Processing base directory '{}'\n".format(rootDir))

# now process all test projects in this test case directory (here, caseDir == rootDir)
processCaseDir(rootDir, rootDir, suffix, set())
