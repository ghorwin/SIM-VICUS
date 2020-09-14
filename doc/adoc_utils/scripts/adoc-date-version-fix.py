#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# This file is part of asciidoctor-publishing git repo.
# Written by Andreas Nicolai <andreas.nicolai -at- gmx[dot]net>.
#
# Replaces version and date in master adoc document.
#
# Syntax: adoc-date-version-fix.py <path/to/src/file/with/constants.cpp> <path/to/adoc/main-file>
#
# The file passed as first argument is expected to be a cpp file with a LONG_VERSION constant. The
# second argument is supposed to be the main adoc file with 'v1.0.0, July 2020' or something like that
# in the 4th line.
#

import os
import glob
import sys
import ntpath

from print_funcs import *

# --- Main program start ---

try:
	# get current working directory, in case we need to resolve relative paths
	scriptFilePath = os.getcwd()

	# check command line
	if len(sys.argv) < 3:
		if len(sys.argv) == 2 and sys.argv[1] == "--help":
			print("Syntax: adoc-image-prep.py <html|pdf> <path/to/adoc/files>")
			exit(0)
	
		raise RuntimeError("Invalid command line, expected three arguments.")

	mode = sys.argv[1]
	adocdir = sys.argv[2]
	print("Processing '{}' in '{}' mode.".format(adocdir, mode))
	
	# process all adoc files
	for f in glob.glob(adocdir+"/*.adoc"):
		fullPath = os.path.abspath(f)
		processAdoc(fullPath, mode, scriptFilePath)
	
except RuntimeError as e:
	printError(str(e))
	exit(1)

exit(0)
