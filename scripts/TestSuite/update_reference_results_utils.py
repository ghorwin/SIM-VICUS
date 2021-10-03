#!/usr/bin/env python3

# Script to update all files within reference directories (with given suffix) from currently computed cases
#
# > update_reference_results.py <directory_suffix>


import os
import os.path
import shutil

def copyIfNotEqualSummary(newFile, oldFile):
	"""Compares both summary.txt files and only copies the new file
	over the old file if at least one of the statistic counters is
	different.

	Returns True of file was copied.
	"""
	nf = open(newFile)
	nflines = nf.readlines()
	of = open(oldFile)
	oflines = of.readlines()
	del nf
	del of

	allTheSame = True

	# different number of lines -> different files
	if len(nflines) != len(oflines):
		allTheSame = False
	else:
		# process all lines, line is the same when:
		# - keyword must match
		# - keyword must contains Time or values are the same
		for i in range(len(nflines)):
			if nflines[i].strip() == "":
				continue # skip empty lines
			nrow = nflines[i].split("=")
			orow = oflines[i].split("=")
			if len(nrow) != len(orow):
				allTheSame = False
				break
			if nrow[0] != orow[0]:
				allTheSame = False
				break
			if nrow[0].find("Time") != -1:
				continue # skip timers
			if int(nrow[1]) != int(orow[1]):
				allTheSame = False
				break
	if not allTheSame:
		shutil.copyfile(newFile, oldFile)
	return not allTheSame


def copyIfNotEqualD6O(newFile, oldFile):
	"""Compares both d6o files and only copies the new file
	over the old file if anything but the date differs.

	Returns True of file was copied.
	"""
	nf = open(newFile)
	nflines = nf.readlines()
	of = open(oldFile)
	oflines = of.readlines()
	del nf
	del of

	allTheSame = True

	# different number of lines -> different files
	if len(nflines) != len(oflines):
		allTheSame = False
	else:
		# process all lines, all lines except that with the CREATION date must be exactly the same
		for i in range(len(nflines)):
			if nflines[i].strip() == "":
				continue # skip empty lines
			if nflines[i].startswith("CREATED"):
				continue # skip date
			if nflines[i] != oflines[i]:
				allTheSame = False
				break
	if not allTheSame:
		shutil.copyfile(newFile, oldFile)
	return not allTheSame



def processCaseDir(c, rootDir, suffix, caseDirFilter, copyAll=False):
	"""
	Processes a test case subdirectory that contains several project files with their
	output folders and associated reference result folders.

	Arguments:

	* c - relative path to case directory (relative to rootDir), for example 'AirFlowTests'
	* rootDir - parent directory of test case subdirectory, for example '../../data/tests'
	* suffix - the reference result directory suffix, for example 'gcc_linux'
	* caseDirFilter - optionally a set() with list of full file paths to process,
	  pass empty set to always check case directory
	"""
	caseDir = os.path.join(rootDir, c)
	print ("Case: '{}'".format(c))

	refDirs = [fname for fname in os.listdir(caseDir )
		       if os.path.isdir(os.path.join(caseDir, fname)) and fname.endswith(suffix)]
	refDirs.sort()

	for d in refDirs:
		srcDir = d[:-(len(suffix)+1)]
		# check if srcDir is in caseDirFilter()
		fullPathToSrcDir = os.path.abspath(os.path.join(caseDir, srcDir))
		if len(caseDirFilter) > 0:
			if not fullPathToSrcDir in caseDirFilter:
				print ("  Project '{}' is not in filter list, skipped".format(os.path.relpath(fullPathToSrcDir, rootDir)))
				continue
		print ("  Project: '{}'".format(os.path.relpath(fullPathToSrcDir, rootDir)))

		# process all files in each directory
		# if file with same name exists in base directory (without suffix), copy file over
		for root, dirs, files in os.walk(os.path.join(caseDir, d)):
			for f in files:
				#print "    File : {}".format(f)
				fullPath = os.path.join(root, f)
				relFilePath = os.path.relpath(fullPath, os.path.join(caseDir,d) )
				# compose filepath of calculated files
				srcFilePath = os.path.join(caseDir, os.path.join(srcDir, relFilePath))
				# if file exists, copy it over
				if os.path.exists(srcFilePath):
					# special handling for output files where only time stamp of creation has changed
					if fullPath[-3:] == 'd6o':
						if copyIfNotEqualD6O(srcFilePath, fullPath):
							print ("    Updated: {}".format(os.path.relpath(fullPath, caseDir)))
					elif srcFilePath.endswith("summary.txt"):
#						print "Comparing summary.txt files"
						if copyAll:
							shutil.copyfile(srcFilePath, fullPath)
						elif copyIfNotEqualSummary(srcFilePath, fullPath):
							print ("    Updated: {}".format(os.path.relpath(fullPath, caseDir)))
					else:
						print ("    Updated: {}".format(os.path.relpath(fullPath, caseDir)))
						shutil.copyfile(srcFilePath, fullPath)
