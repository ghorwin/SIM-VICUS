#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#
# Utility script to manipulate TSV files. TSV files are files with tab-separated columns.
# Written by Andreas Nicolai (2020/04).
#
# Syntax: tsvtool.py <operation> <arguments>
#
# Use tsvtool.py --help for Syntax-Help.

LONG_HELP_TEXT = """
This small utility is used in SimQuality-project to generate TSV files that can be automatically
processed by the analysis scripts.

Common mandatory arguments:

  <input file>  - first argument is always an input file
  <operation>   - a keyword from the list below
  
Common optional arguments:

  -o <output file>  - if given, the result is written to this file (can overwrite input file), 
                      otherwise output file name is automatically generated.
  -d                - convert values to doubles (hereby check if columns contain valid numbers),
                      when writing the file, numbers are written with default precision
  

Operations

  remove_empty    - removes empty columns
    - no extra arguments

  
  empty2zero      - inserts 0 in columns with no data
    - no extra arguments

	
  insert_columns  - inserts columns from one TSV file into another
    
    > tsvtool.py insert_columns <input file> <file to insert> <insert index>

    
  remove_columns  - removes columns from one TSV file into another
    
    > tsvtool.py remove_columns <input file> <column selection>
    
    where <column selection> is a | separated list of single column indexes or ranges, for example
    "0|5-10|17|20-200"


  extract_columns - extracts columns from one TSV file, optionally replacing header labels
   
    > tsvtool.py extract_columns <input file> <column indexes> [<new headers>]

    where <column selection> is a | separated list of single column indexes or ranges, for example
    "0|5-10|17|20-200". 
    
    The optional argument <new headers> argument is a list of header labels to be used in place of the original
    column headers, for example "Temperature [C]| Velocity [m/s]". However, when specifying <new headers>, you 
    *must* specify column indexes one-by-one and the number of column indexes *must* match the number of new headers!
    
    Note: this is (almost) the inverse of 'remove_columns'. 
          
    Tip:  you can use this operation to re-order the columns in the data set, for example, when you pass "2|1|0" 
          as column arguments, you effectively swap columns 0 and 2
    

  remove_rows     - remove rows from a TSV file
  
    > tsvtool.py remove_rows <input file> <row selection>
    
    where <row selection> is a | separated list of single column indexes or ranges, for example
    "0|5-10|17|20-200". Note, row index starts with 0 as first data row (you cannot select/remove the header row).

  
  extract_rows    - extracts rows from a TSV file
  
    > tsvtool.py extract_rows <input file> <row selection>
    
    where <row selection> is a | separated list of single column indexes or ranges, for example
    "5-10|17|20-200". Note, row index starts with 0 as first data row (you cannot select the header row).
  
    Note: this is the inverse of 'remove_rows' operation.
    
"""

import sys
import argparse

from TSVContainer import TSVContainer

def expandRange(r):
	"""Expands a range, either as single number or in format "start-end"
	to a list of numbers.
	"""
	
	try:
		tokens = r.split("-")
		if len(tokens) == 1:
			return [ int(r) ]
		elif len(tokens) == 2:
			startIdx = int(tokens[0])
			endIdx = int(tokens[1])
			if startIdx > endIdx:
				s = startIdx
				startIdx = endIdx
				endIdx = s
			res = []
			for i in range(startIdx, endIdx+1):
				res.append(i)
			return res
		else:
			raise RuntimeError("bad")
	except:
		raise RuntimeError("Invalid range value '{}'".format(r))


def removeEmptyCols(c, args):
	"""Removes empty columns from file."""
	c.removeEmptyCols()


def insertColumns(c, args):
	"""Inserts columns from a second TSV file into the first"""
	if len(args.vars) < 2:
		raise RuntimeError("Missing arguments to 'insert_columns' operation") 
	c2 = TSVContainer()
	print("Reading file with columns to insert {}".format(args.vars[0]))
	c2.readAsStrings(args.vars[0])
	
	# sanity check, number of rows must match
	if len(c.data[0]) != len(c2.data[0]):
		raise RuntimeError("Mismatching row counts.")

	insertIdx = int(args.vars[1])
	
	# special handling - insert at end
	if insertIdx > len(c.headers):
		print("Inserting at end")
		c.headers = c.headers + c2.headers
		c.data = c.data + c2.data
	else:
		print("Inserting colums before #{}".format(insertIdx))
		c.headers = c.headers[0:insertIdx] + c2.headers + c.headers[insertIdx:]
		c.data = c.data[0:insertIdx] + c2.data + c.data[insertIdx:]


def removeColumns(c, args):
	"""Removes selected columns from file."""
	if len(args.vars) < 1:
		raise RuntimeError("Missing arguments to 'remove_columns' operation")
	cols = args.vars[0].split("|")
	colList = []
	for col in cols:
		colList = colList + expandRange(col)
	# make this is sorted list without duplicates
	colSet = set(colList)
	
	# now process all columns and only keep those that are not in the list
	newHeader = []
	newData = []
	for i in range(len(c.headers)):
		if not i in colSet:
			newHeader.append(c.headers[i])
			newData.append(c.data[i])
	
	c.headers = newHeader
	c.data = newData


def extractColumns(c, args):
	"""Extracts selected columns from file."""
	if len(args.vars) < 1:
		raise RuntimeError("Missing arguments to 'extract_columns' operation")
	cols = args.vars[0].split("|")
	
	# check, if we have a newHeaders argument
	if len(args.vars) > 1:
		newHeaders = args.vars[1].split("|")
		newHeaders = [n.strip() for n in newHeaders]
		if len(newHeaders) != len(cols):
			raise RuntimeError("Number of selected columns does not match number of given new headers")
	else:
		newHeaders = None
		
	colList = []
	for col in cols:
		colList = colList + expandRange(col)
		
	# sanity check: must not have duplicates
	if len(set(colList)) != len(colList):
		raise RuntimeError("Duplicate column indexes in selection range")
	
	# now process all columns and only keep those that are in the list
	newHeader = []
	newData = []
	for i in range(len(colList)):
		colIdx = colList[i]
		if colIdx >= len(c.headers):
			print("Column index {} out of range(0,{}), skipped".format(colIdx, len(c.headers)))
			continue
		if newHeaders != None:
			newHeader.append(newHeaders[i])
		else:
			newHeader.append(c.headers[colIdx])
		newData.append(c.data[colIdx])
	
	c.headers = newHeader
	c.data = newData

	
def removeRows(c, args):
	"""Removes selected rows from file."""
	if len(args.vars) < 1:
		raise RuntimeError("Missing arguments to 'remove_rows' operation")
	rows = args.vars[0].split("|")
	rowList = []
	for row in rows:
		rowList = rowList + expandRange(row)
	# make this is sorted list without duplicates
	rowSet = set(rowList)
	
	# now process copy all rows that are not in the selected row set
	colCount = len(c.headers)
	newData = [ [] ] * colCount
	for r in range(len(c.data[0])):
		if not r in rowSet:
			for col in range(colCount):
				newData[col].append(c.data[col][r])
	
	c.data = newData


def extractRows(c, args):
	"""Extracts selected rows from file."""
	if len(args.vars) < 1:
		raise RuntimeError("Missing arguments to 'extract_rows' operation")
	rows = args.vars[0].split("|")
	rowList = []
	for row in rows:
		rowList = rowList + expandRange(row)
	# make this is sorted list without duplicates
	rowSet = set(rowList)
	print("Extracting {} rows".format(len(rowSet)))
	
	# now process copy all rows that are in the selected row set
	colCount = len(c.headers)
	newData = [ ]
	for col in range(colCount):
		newData.append( [] )
	for r in range(len(c.data[0])):
		if r in rowSet:
			for col in range(colCount):
				newData[col].append(c.data[col][r])
	
	c.data = newData


# --- main ----

parser = argparse.ArgumentParser(
    description='A utility to manipulate TSV files.', epilog=LONG_HELP_TEXT,
    formatter_class=argparse.RawDescriptionHelpFormatter)
parser.add_argument("operation", help="The operation.")
parser.add_argument("inputfile", help="The primary TSV file to work on.")
parser.add_argument("-o", "--output", type=str, 
                    help="Output file name, if missing, output name is automatically generated.")
parser.add_argument("-d", "--double", 
                    help="If flag is given, values are converted to floating point before being written to file.", 
                    action='store_true')
parser.add_argument('vars', nargs='*')

args = parser.parse_args()

if args.output == None:
	if len(args.inputfile) < 4:
		args.output = args.inputfile + "_out.tsv"
	else:
		args.output = args.inputfile[0:-4] + "_out.tsv"

op = args.operation
inputFile1 = args.inputfile

# first argument is always a tsv file, so we can read it already
try:
	c = TSVContainer()
	print("Reading input file {}".format(inputFile1))
	c.readAsStrings(inputFile1)

	# depending on operation, call different functions
	if op == "remove_empty":
		removeEmptyCols(c, args)
	elif op == "insert_columns":
		insertColumns(c, args)
	elif op == "remove_columns":
		removeColumns(c, args)
	elif op == "extract_columns":
		extractColumns(c, args)
	elif op == "remove_rows":
		removeRows(c, args)
	elif op == "extract_rows":
		extractRows(c, args)
	else:
		raise RuntimeError("Unknown operation {}".format(op))

	if args.double:
		c.convert2Double()
	print("Writing result file {}".format(args.output))
	c.write(args.output)
	print("Done.")
	
except RuntimeError as e:
	print(str(e))
	exit(1)
exit(0)
