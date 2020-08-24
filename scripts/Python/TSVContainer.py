#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# A reader/writer/manipulator class for TSV files

class TSVContainer:
	
	def __init__(self):
		# data is stored in columns, either as strings or floats, first index column index (time column = idx 0)
		# second index is row - does not contain header labels
		self.data = []
		# header names for the columns
		self.headers = []
		# empty flag - True for all columns that have only empty strings except for header
		self.emptyColumn = []
	
	def readAsStrings(self, fname):
		"""Reads the file but keeps all tokens as strings"""
		try:
			print("Reading {}".format(fname))
			fobj = open(fname, 'r')
			lines = fobj.readlines()
			fobj.close()
			del fobj
			for lidx in range(len(lines)):
				l = lines[lidx]
				if len(l.strip()) == 0:
					break # stop on first empty line
				t = l.split('\t')
				t[-1] = t[-1].strip()
				if len(t[-1]) == 0:
					t = t[0:-1]
				# header line
				if len(self.headers) == 0:
					columnCount = len(t)
					self.headers = t
					# initialize data storage
					self.data = []
					for i in range(columnCount):
						self.data.append([])
					self.emptyColumn = [True] * columnCount
				else:
					# sanity check - enough columns in row?
					if len(t) != columnCount:
						raise RuntimeError("  Error: Column count {} in row #{} mismatches header column count {}".format(len(t), lidx, columnCount))
					# for all columns (including time column)
					for colIdx in range(len(t)):
						self.data[colIdx].append(t[colIdx])
						# if we have data in the column, mark it as not-empty
						if len(t[colIdx].strip()) > 0:
							self.emptyColumn[colIdx] = False
			
			if len(self.headers) == 0:
				raise RuntimeError("Missing in header line, empty file?")
			if len(self.data[0]) <= 1:
				raise RuntimeError("Missing data, only one line in file?")
			print("  {} columns, {} data rows, ".format(columnCount, len(self.data[0])-1))
		except IOError as e:
			print(str(e))
			raise RuntimeError("Error reading file '{}'".format(fname))


	def removeEmptyCols(self):
		"""All columns, that only contain data in the header row, are removed"""
		colCount = len(self.headers)
		if colCount == 0:
			return
		# now extract only data and header columns that are not empty
		remainingHeaders = []
		remainingData = []
		for colIdx in range(colCount):
			if self.emptyColumn[colIdx]:
				print("Removing empty column {} '{}'".format(colIdx, self.headers[colIdx]))
			else:
				remainingData.append( self.data[colIdx] )
				remainingHeaders.append( self.headers[colIdx])
		
		self.data = remainingData
		self.headers = remainingHeaders
		self.emptyColumn = [False] * len(self.headers)


	def convert2Double(self):
		"""Converts read data (except first line) to floats. A 0.0 is stored in place of 
		empty cells."""
		if len(self.headers) < 1:
			return
		for colidx in range(len(self.data)):
			col = self.data[colidx]
			for rowidx in range(len(col)):
				if len(col[rowidx].strip()) == 0:
					col[rowidx] = 0 # in-place substitution
				else:
					try:
						val = float(col[rowidx]) # in-place substitution
						col[rowidx] = val
					except:
						print("Data conversion error for value '{}' in row {} and column {}, keeping string value".format(col[rowidx], rowidx, colidx))
	
	
	def write(self, fname):
		try:
			print("Writing {}".format(fname))
			fobj = open(fname, 'w')
			if len(self.headers) > 0:
				# write header line
				fobj.write("\t".join(self.headers) + "\n")
				# now all the data rows
				for ridx in range(len(self.data[0])):
					newLine = ""
					for c in self.data:
						val = c[ridx]
						if val == 0:
							newLine = newLine + "0\t"
						else:
							if type(val) is float:
								newLine = newLine + "{:.6}\t".format(val)
							else:
								newLine = newLine + "{}\t".format(val)
					fobj.write(newLine.strip() + "\n")
			fobj.close()
			del fobj
		except IOError as e:
			print(str(e))
			raise RuntimeError("Error writing to file {}".format(fname))
			
