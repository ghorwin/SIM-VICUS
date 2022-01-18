import os
import sys

class CCDFile(object):
	"""Class for reading CCDFiles."""

	def read(self,directory, fname):
		"""Reads the CCD file with the filename fname."""
		print ("Reading '{0}'".format(fname))
		# initializing member variables
		self.location = ""
		self.description = ""
		self.key = ""	# the climate type
		self.height = "" 
		self.longitude = ""
		self.latitude = ""
		self.startdate = ""
		self.tp = []
		self.values = []

		# try to load the file
		try :
			# create file object
			fobj = open(directory + fname, 'r')
			# read all lines
			lines = fobj.readlines()
		except IOError:
			print ("Can't read CCD file.")
			return 1
		# close file, we keep the data in 'lines'
		fobj.close

		# read line by line
		linecount = 0
		for line in lines:
			linecount = linecount + 1
			# skip comments
			pos = line.find('#')
			if pos != -1:
				# remove everything past and including the #
				line = line[:pos]
			# remove leading or trailing whitespaces
			line = line.strip(' \t\n')
			# skip completely empty lines
			if len(line) == 0:
				continue
			
			# try to extract header line
			parts = line.split('=')
			if len(parts) == 2:
				kw = parts[0].strip()
				value = parts[1].strip()
				if kw == 'LOCATION':
					self.location = value
				elif kw == 'DESCRIPTION':
					self.description = value
				elif kw == 'TYPE':
					self.key = value
				elif kw == 'HEIGHT':
					self.height = value
				elif kw == 'LONGITUDE':
					self.longitude = value
				elif kw == 'LATITUDE':
					self.latitude = value
				elif kw == 'STARTDATE':
					self.startdate = value
				else:
					print ('Warning: Invalid or unregognized keyword: ',kw)
			else:
				# replace ':' with ' '
				line = line.replace(':', ' ')
				# try to read data values
				vals = line.split();
				if len(vals) == 5:
					day = int(vals[0])
					h = int(vals[1])
					min = int(vals[2])
					sec = int(vals[3])
					val = float(vals[4])
					self.tp.append(day*3600*24 + h*3600 + min*60 + sec)
					self.values.append(val)

	def write(self,directory, fname):
		"""Writes the CCD file with the filename fname."""
		# try to create the file
		try :
			# create file object
			fobj = open(directory + fname, 'w')

			print ("Writing '{0}'".format(fname))

			fobj.write("LOCATION 			= {0}\n".format(self.location))
			fobj.write("DESCRIPTION 		= {0}\n".format(self.description))
			fobj.write("TYPE 				= {0}\n".format(self.key))
			fobj.write("HEIGHT 				= {0}\n".format(self.height))
			fobj.write("LONGITUDE 			= {0}\n".format(self.longitude))
			fobj.write("LATITUDE 			= {0}\n".format(self.latitude))
			fobj.write("STARTDATE 			= {0}\n".format(self.startdate))

			fobj.write("\n");
			for i in range(0, len(self.tp)):
				sec = self.tp[i] % 60
				min = (self.tp[i] // 60) % 60
				h = (self.tp[i] // 3600) % 24
				day = self.tp[i] // int(3600*24)
				fobj.write("{0:>5.0f} {1:>5.0f} {2:>2.0f} {3:>2.0f} {4:>8}\n".format(day, h, min, sec, self.values[i]))
		except IOError:
			print ("Can't write CCD file.")
			return 1
		# close file, we keep the data in 'lines'
		fobj.close
