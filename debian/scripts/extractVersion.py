#!/usr/bin/env python3

import sys
import os

def extractVersionNumber( pathToConstantsFile ):
	"""
	The version number is a constant in IBK solvers and apps and will be taken to identify the current status.
	After extracting the current number it can be used as suffix in folder names, etc.
	To read the version number the IBK specific notation has to be realized in the *Constants.cpp of solver or app.

	Returns a tuple of (version, longVersion)
	"""
	versionNumber        = ""
	searchExpVersion     = "const char * const VERSION"
	searchExpLongVersion = "const char * const LONG_VERSION"

	version              = None
	longVersion         = None

	# open file to get LONG_VERSION and VERSION

	f = open(pathToConstantsFile, mode='r', buffering=1)
	for line in f.readlines():

		if searchExpVersion in line:
			versionNumber = line.split("=")[1]
			version = versionNumber.strip(" \";\n")

		if searchExpLongVersion in line:
			versionNumber = line.split("=")[1]
			longVersion = versionNumber.strip(" \";\n")

	f.close()

	if longVersion == None or version == None:
		print("Missing or invalid version/long version number in file '{}'"
						   .format(pathToFile))
		exit(1)

	return version, longVersion


# *** main program ***

# Syntax: extractVersion.py <path/to/xxx_version.cpp>
#
# writes versionnumber into stream

if (len(sys.argv) != 2):
	exit(1)

constantsPath = sys.argv[1]
if not os.path.exists(constantsPath):
	print("Invalid file path '{}'".format(constantsPath))
	exit(1)

try:
	(shortVer, longVer) = extractVersionNumber(constantsPath)
	print(str(longVer));
	exit(0)
except RuntimeError as e:
	print(e)
	exit(1)

