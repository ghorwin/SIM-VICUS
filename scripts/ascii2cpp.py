#!/usr/bin/python

import os
import optparse
import sys

def escapeSpecialCharacters ( text, characters ):
    for character in characters:
        text = text.replace( character, '\\' + character )
    return text

def print_syntax():
	print "Syntax: ascii2cpp <input file> [-o outputfile]"

# parse command line
parser = optparse.OptionParser()

parser.add_option("-o", "--output-file", dest="outfile",
                  help="Target file", metavar="FILE")

(options, args) = parser.parse_args()
#print options

if len(args) != 1:
	print_syntax()
	sys.exit(1)

# open output file and parse data
try:
	outfile_obj = open(args[0], 'r')
	lines = outfile_obj.readlines()
except IOError:
	print "Can't read input file '"+args[0]+"'."
	sys.exit(1)
outfile_obj.close

if options.outfile != None:
	resfile_obj = open(options.outfile, 'w')
	for i in range(len(lines)):
		lstr = lines[i]
		if len(lstr) > 0 and lstr[-1]:
			lstr = lstr[0:-1]
		lstr = escapeSpecialCharacters(lstr,'"');
		resfile_obj.write('"' + lstr + '\\n"\n');
else:
	for i in range(len(lines)):
		lstr = lines[i]
		if len(lstr) > 0 and lstr[-1]:
			lstr = lstr[0:-1]
		lstr = escapeSpecialCharacters(lstr,'"');
		print '"' + lstr + '\\n"'
