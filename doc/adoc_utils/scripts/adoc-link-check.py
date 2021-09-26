#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#encoding: utf-8

# This file is part of asciidoctor-publishing git repo.
# Written by Andreas Nicolai <andreas.nicolai -at- gmx[dot]net>.
#
# Link and reference check tool to assist with asciidoctor publishing
#
# Syntax: adoc-link-check.py <path/to/adoc/files>
#
#
# AsciiDoc syntax expectations:
#
# - [[link]] always start at begin of line and line contains nothing but link
# - [[link]] may be followed in next line by header section, i.e. # on start of next line, 
#            header title is used for annotating the label
# - [[link]] may be followed a title .xxx in next line, title is used for annotating the label 
# - <<link,title>> and <<link>> is parsed wherever found, even in source code blocks.
#
# - image link labels are recognized by "image::" on next line after link label
# - table link labels are recognized by "[width=" on next line after link label
# 
# Warnings are issued when:
# - link is not found
# - link starts with number (invalid)

import os
import glob
import sys
import ntpath

from print_funcs import *

from colorama import init
init()

def findLinkLabel(label, links):
	"""Checks if given label exists in list of link labels.
	
	**Arguments**
	
	*label*
	  the label to search for
	  
	*links*
	  list with link labels collected so far, 
	  list contains tuples of (label, filename, line nr., description, is_duplicate)

	**Returns**
	
	Tuple with existing link label information or None if it does not yet exist in list.
	"""
	
	for l in links:
		if l[0] == label:
			return l
	return None


def scanForLinkLabels(fpath, links):
	"""Processes asciidoctor input file:
	
	**Arguments**
	
	*fpath*
	  full file path to Ascii-Doctor input file
	  
	*links*
	  list with link labels collected so far (will be modified in function),
	  list contains tuples of (label, filename, line nr., description, is_duplicate)
	"""
	
	try:
		adocDirPath, adocFName = ntpath.split(fpath)
		#print("\n  {}".format(adocFName))
		
		# read the file
		fobj = open(fpath, 'r')
		lines = fobj.readlines()
		fobj.close
		del fobj
		
		# now process line by line, search for [[xxx]] pattern
		for i in range(len(lines)):
			line = lines[i]
			pos = line.find("[[")
			# does line start with [[ ?
			if pos == 0: 
				pos2 = line.find("]]",3)
				if pos2 != -1:
					# found new link label
					link_label = line[2:pos2].strip()
					if len(link_label) == 0:
						printError("    empty label at {}:{}".format(adocFName, i+1)) # Mind: human-user line numbering starts with 1
						continue
					desc = ""
					if i+1 < len(lines):
						nextLine = lines[i+1]
						if len(nextLine) != 0:
							# check if following line contains a caption
							if nextLine[0] == "#":
								desc = nextLine.strip()
							# check if following line contains a caption
							elif nextLine[0] == ".":
								desc = nextLine[1:].strip()
								# further determine if the next line contains a table or image, and add a suffix to clarify type of reference
								if i+2 < len(lines):
									overnextLine = lines[i+2]
									if overnextLine.find("image::") == 0:
										desc = "image:" + desc
									elif overnextLine.find("[width=") == 0:
										desc = "table:" + desc
							elif nextLine.find("image::"):
								desc = nextLine[8:].strip()
					# check if such label exists already somewhere
					existingLinkLabel = findLinkLabel(link_label, links)
					if existingLinkLabel == None:
						links.append( (link_label, adocFName, i+1, desc, False) ) # Mind: human-user line numbering starts with 1
					else:
						links.append( (link_label, adocFName, i+1, desc, True) )
						printError("    Duplicate label '{}' found at {}:{} and {}:{}".format(link_label, existingLinkLabel[1], existingLinkLabel[2], adocFName, i))
		
	except IOError as e:
		printError(str(e))
		raise RuntimeError("Error processing adoc file.")


def checkReferences(fpath, links):
	"""Processes asciidoctor input file:
	
	**Arguments**
	
	*fpath*
	  full file path to Ascii-Doctor input file
	  
	*links*
	  list with link labels collected so far,
	  list contains tuples of (label, filename, line nr., description, is_duplicate)
	"""
	try:
		adocDirPath, adocFName = ntpath.split(fpath)
		#print("\n  {}".format(adocFName))
		
		# read the file
		fobj = open(fpath, 'r')
		lines = fobj.readlines()
		fobj.close
		del fobj
		
		# now scan all lines for << followed by >>
		for i in range(len(lines)):
			line = lines[i]
			pos = line.find("<<")
			while pos != -1:
				# look for following >>
				pos2 = line.find(">>", pos+2)
				if pos2 != -1:
					crossRef = line[pos+2:pos2]
					# check if this cross reference exists
					tokens = crossRef.split(",")
					if len(tokens) > 2:
						printWarning("  WARNING: invalid cross reference '{}' "
						             "at {}:{} or not a cross reference at all".format(crossRef, adocFName, i+1))
					else:
						crossRefLabel = tokens[0]
						# try to find the cross ref label in link list
						if findLinkLabel(crossRefLabel, links) == None:
							loc = "{}:{}".format(adocFName, i+1)
							printError("  {:<30s} {:<30s}".format(crossRefLabel, loc))
				pos = line.find("<<", pos+2)

	except IOError as e:
		printError(str(e))
		raise RuntimeError("Error processing adoc file.")



# --- Main program start ---

try:

	# get current working directory, in case we need to resolve relative paths
	scriptFilePath = os.getcwd()

	# check command line
	if len(sys.argv) == 2 and sys.argv[1] == "--help":
		print("Syntax: adoc-link-check.py <path/to/adoc/files>")
		exit(0)

	if len(sys.argv) < 2:
		raise RuntimeError("Invalid command line, expected path argument.")

	adocdir = sys.argv[1]
	print("Scanning for link-labels in adoc files of '{}':".format(adocdir))
	
	# process all adoc files - first pass, scan for [[xxx]] link labels
	links = []
	for f in glob.glob(adocdir+"/*.adoc"):
		fullPath = os.path.abspath(f)
		scanForLinkLabels(fullPath, links)

	print("\nList of labels:")
	# sort labels by link label
	links = sorted(links, key=lambda tup: tup[0])
	# dump out a list of all labels
	for l in links:
		loc = "{}:{}".format(l[1], l[2])
		# either a duplicate or bad label name - print as error
		badLabelError = ""
		if l[0][0].isdigit():
			badLabelError = "must not start with digit"
		elif l[0].find(",") != -1:
			badLabelError = "must not contain ,"
		elif l[4]:
			badLabelError = "duplicate"
		if len(badLabelError) != 0:
			printError("  {:<30s} {:<30s} '{}' : {}".format(l[0], loc, l[3], badLabelError) )
		else:
			print("  {:<30s} {:<30s} '{}'".format(l[0], loc, l[3]) )
	
	print("\nInvalid/problematic cross-references:")
	# process all adoc files - second pass, scan for <<xxx>> references and print errors if encountered
	for f in glob.glob(adocdir+"/*.adoc"):
		fullPath = os.path.abspath(f)
		checkReferences(fullPath, links)
	
except RuntimeError as e:
	printError(str(e))
	exit(1)

exit(0)
