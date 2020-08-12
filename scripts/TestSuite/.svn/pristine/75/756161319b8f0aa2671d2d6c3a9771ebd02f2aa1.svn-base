import os
import filecmp
from print_funcs import *

class SolverStats:
	"""
	Reads solver summary.txt file and provides comparison functionality.
	"""
	
	def __init__(self):
		self.timers = dict()
		self.counters = dict()
		
	def read(self, statsFile):
		"""Attempts to read the summary file and returns True on success.
		
		Parameters
		==========
		
		- *statsFile* File path (relative or absolute) to summary.txt file.
		
		Return value
		============
		True on success, False when file could not be opened or parsed.
		"""
		try:
			fobj = open(statsFile, 'r')
			lines = fobj.readlines()
			for line in lines:
				tokens = line.strip().split("=")
				if len(tokens) != 2:
					continue
				if tokens[0].find("Time") != -1:
					self.timers[tokens[0]] = float(tokens[1])
				else:
					self.counters[tokens[0]] = long(tokens[1])
		except Exception as e:
			print "Error opening/reading file '{}', error: {}".format(statsFile, e)
			return False
		return True

	@staticmethod
	def compareStats(s1, s2):
		"""Compares two sets of summary files (statistics counters).
		
		Parameters
		==========
		- *s1* Reference set of summary/statistics
		- *s2* Other set of summary/statistics to compare against
		
		Return value
		============
		True on success, i.e. statistics counters match exactly.
		"""
		
		# compare counters
		print "  {:30s}  {:>12s}    {:>12s}".format("", "Reference", "New")
		# print side-by-side differences
		s1keys = s1.counters.keys()
		s2keys = s2.counters.keys()
		s1keys = list(set(s1keys + s2keys))
		s1keys.sort()
		fail = False
		for k in s1keys:
			if s1.counters.has_key(k):
				if s2.counters.has_key(k):
					match = s1.counters[k] == s2.counters[k]
					if match:
						print "  {:30s}  {:12d} == {:12d}".format(k, s1.counters[k], s2.counters[k])
					else:
						printError("  {:30s}  {:12d} <> {:12d}".format(k, s1.counters[k], s2.counters[k]))
						fail = True
				else:
					printError("  {:30s}  {:12d} <> {:12s}".format(k, s1.counters[k], ""))
					fail = True
			else:
				printError("  {:30s}  {:12s} <> {:12d}".format(k, "", s2.counters[k]))
				fail = True
		
		# compare timings (with threshold)
		s1keys = s1.timers.keys()
		s2keys = s2.timers.keys()
		s1keys = list(set(s1keys + s2keys))
		s1keys.sort()
		print "  --"
		THRESHOLD = 0.1
		for k in s1keys:
			if s1.timers.has_key(k):
				if s2.timers.has_key(k):
					val1 = s1.timers[k]
					val2 = s2.timers[k]
					match = val1*(1+THRESHOLD) > val2 and val2*(1+THRESHOLD) > val1
					# if time is too small to be meaningful, always accept match
					if abs(val1-val2) < 1:
						match = True
					if match:
						print "  {:30s}  {:12.2f} ~~ {:12.2f}".format(k, s1.timers[k], s2.timers[k])
					else:
						printWarning("  {:30s}  {:12.2f} <> {:12.2f}".format(k, s1.timers[k], s2.timers[k]))
				else:
					printWarning("  {:30s}  {:12.2f} <> {:12s}".format(k, s1.timers[k], ""))
			else:
				printWarning("  {:30s}  {:12s} <> {:12.2f}".format(k, "", s2.timers[k]))
		print "\n"
			
		return not fail
		
	
	@staticmethod
	def compareResults(dir1, dir2):
		fail = False
		
		# get files in dir1 (reference files)
		files = [ f for f in os.listdir(dir1) if os.path.isfile(os.path.join(dir1,f)) ]

		for f in files:
			fileParts = os.path.splitext(f)
			# skip files without extension
			if len(fileParts) < 2:
				continue
			# skip geometry files
			if fileParts[1] == ".d6o" or fileParts[1] == ".tsv":
				if fileParts[1] == ".tsv":
					# for tsv files we do binary comparison
					if not filecmp.cmp(os.path.join(dir1,f), os.path.join(dir2,f), shallow=False):
						printError("Mismatching content of file (byte difference) '{}'".format(f))
						fail = True
					else:
						print f
				else:
					# try to open reference and result files
					try:
						fobj1 = open(os.path.join(dir1,f), 'r')
						fobj2 = open(os.path.join(dir2,f), 'r')
					
						# read contents
						lines1 = fobj1.readlines()
						lines2 = fobj2.readlines()
						
						if len(lines1) != len(lines2):
							printError("Mismatching content of file '{}', different number of lines".format(f))
							fail = True
						else:
							for i in range(15, len(lines1)):
								if lines1[i] != lines2[i]:
									printError("Mismatching content of file '{}'".format(f))
									# print this and next lines
									printError("  '{}'".format(lines1[i].rstrip('\n')))
									printError("  '{}'".format(lines2[i].rstrip('\n')))
									fail = True
									break
							print f
					except OSError as e:
						printError("Error comparing files '{}'".format(f))
						fail = True
		
		return not fail
