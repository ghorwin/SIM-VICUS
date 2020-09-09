#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# This file is part of asciidoctor-publishing git repo.
# Written by Andreas Nicolai <andreas.nicolai -at- gmx[dot]net>.
#
# Image exchange tool to assist with asciidoctor publishing
#
# Syntax: adoc-image-prep.py <html|pdf> <path/to/adoc/files>
#
#
# AsciiDoc syntax expectations:
#
# image: or image:: tags are followed by filename and either space, tab char or end of line. No spaces or [
#                   are allowed in filename for now.


import os
import glob
import sys
import ntpath

from print_funcs import *

PRINT_FILE_SUFFIX = "-print"

def processAdoc(fpath, mode, scriptPath):
	try:
		adocDirPath, adocFName = ntpath.split(fpath)
		print("\n  Processing '{}'".format(adocFName))
		
		# read the file
		fobj = open(fpath, 'r')
		lines = fobj.readlines()
		fobj.close
		del fobj
		
		imagesdir = ""
		# now process line by line, search for :imagesdir: property
		for i in range(len(lines)):
			line = lines[i]
			pos = line.find(":imagesdir:")
			if pos != -1:
				imagesdir = line[11:].strip()
				imagesdir = os.path.abspath(os.path.join(adocDirPath, imagesdir))
				print ("    Images dir = '{}'".format(imagesdir))
				continue
			# search for image: or image:: in text
			pos = line.find("image:")
			while (pos != -1):
				if len(line) > pos+2:
					if line[pos+6] == ':':
						pos = pos + 7
					else:
						pos = pos + 6
				# search for first delimiter, either [ or ' '
				pos_braket = line.find("[", pos)
				pos_space = line.find(" ", pos)
				pos2 = pos_braket
				if pos_space != -1:
					if pos2 != -1:
						if pos_space < pos_braket:
							pos2 = pos_space
					else:
						pos2 = pos_space
				# pos2 now either holds the position of the first space, first '[' or -1 (end of line)
				imagefname = line[pos:pos2].strip()
				print("    Image ref   = {}".format(imagefname))
				
				# check for existing file
				if len(imagesdir) != 0:
					fullImagePath = os.path.join(imagesdir, imagefname)
				else:
					fullImagePath = os.path.join(adocDirPath, imagefname)
				if not os.path.exists(fullImagePath):
					raise RuntimeError("    ERROR: {}:{}:Image file {} ({}) not found".format(adocFName, i+1, imagefname, fullImagePath))

				# now we check for -print suffix 
				(basename,ext) = os.path.splitext(imagefname)
				posPrint = basename.rfind(PRINT_FILE_SUFFIX)
				# do we have a filename without 
				if posPrint == -1:
					htmlName = basename + ".png"
					pdfName = basename + PRINT_FILE_SUFFIX + ext
				else:
					htmlName = basename[:-len(PRINT_FILE_SUFFIX)] + ".png"
					pdfName = basename + ext
				
				# based on mode, determine target filename
				if mode == "html":
					targetFile = htmlName
				else:
					targetFile = pdfName
				# check if it exists
				if len(imagesdir) != 0:
					fullTargetPath = os.path.join(imagesdir, targetFile)
				else:
					fullTargetPath = os.path.join(adocDirPath, targetFile)
				if not os.path.exists(fullTargetPath):
					# in case of print mode, we might have a xxx-print.pdf, xxx-print.svg or xxx-print.pdf file
					# look for all of those
					if mode != "html":
						targetFile2 = targetFile[0:-4] + ".svg"
						fullTargetPath2 = os.path.join(imagesdir, targetFile2)
						if os.path.exists(fullTargetPath2):
							targetFile = targetFile2
							fullTargetPath = fullTargetPath2
				if not os.path.exists(fullTargetPath):
					printWarning("    WARNING: Target file {} not found, keeping original file name".format(targetFile))
					targetFile = imagefname
				else:
					print("    --> new ref = {}".format(targetFile))
				# now replace filenames
				line = line[0:pos] + targetFile + line[pos2:]
					
				pos = line.find("image:", pos2)
			# while loop end
			
			# store (potentially) modified line object
			lines[i] = line
			
		# for loop end
		
		
		# finally dump out the file again
		fobj = open(fpath, 'w')
		fobj.writelines(lines)
		fobj.close()
		del fobj
		
		
	except IOError as e:
		printError(str(e))
		raise RuntimeError("Error processing adoc file.")



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
