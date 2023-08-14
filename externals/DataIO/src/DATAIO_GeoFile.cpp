/*	DataIO library
	Copyright (c) 2001-2016, Institut fuer Bauklimatik, TU Dresden, Germany

	Written by A. Nicolai, St. Vogelsang
	All rights reserved.

	Redistribution and use in source and binary forms, with or without modification,
	are permitted provided that the following conditions are met:

	1. Redistributions of source code must retain the above copyright notice, this
	   list of conditions and the following disclaimer.

	2. Redistributions in binary form must reproduce the above copyright notice,
	   this list of conditions and the following disclaimer in the documentation
	   and/or other materials provided with the distribution.

	3. Neither the name of the copyright holder nor the names of its contributors
	   may be used to endorse or promote products derived from this software without
	   specific prior written permission.

	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
	ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
	WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
	DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
	ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
	(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
	LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
	ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
	(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
	SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "DATAIO_GeoFile.h"


#include <numeric>
#include <fstream>
#include <iterator>
#include <sstream>
#include <algorithm>

#include <IBK_InputOutput.h>
#include <IBK_NotificationHandler.h>
#include <IBK_FileReader.h>
#include <IBK_crypt.h>
#include <IBK_Version.h>
#include <IBK_assert.h>
#include <IBK_messages.h>
#include <IBK_Constants.h>

#include "DATAIO_DataIO.h"
#include "DATAIO_Constants.h"

namespace DATAIO {

GeoFile::GeoFile() {
	clear();
}
// ----------------------------------------------------------------------------


void GeoFile::read(const IBK::Path &fname, IBK::NotificationHandler * notify){
	clear();

	FUNCID(GeoFile::read);
	// make sure the sizes are correct
	IBK_STATIC_ASSERT(sizeof(unsigned int) == 4);
	IBK_STATIC_ASSERT(sizeof(int) == 4);
	IBK_STATIC_ASSERT(sizeof(double) == 8);

	// open file in binary mode and check if successful
	std::ifstream in;
	if (!IBK::open_ifstream(in, fname, std::ios_base::binary))
		throw IBK::Exception( IBK::FormatString("Couldn't open geometry file '%1'.").arg(fname), FUNC_ID);

	try {
		try {
			unsigned int minorFileVersionDummy;
			IBK::Version::read(in, MAGIC_NUMBER_BINARY_GEOFILE, SECOND_MAGIC_NUMBER, MAGIC_NUMBER_ASCII_GEOFILE, SECOND_MAGIC_NUMBER,
							   m_isBinary, m_majorFileVersion, minorFileVersionDummy);
		}
		catch (IBK::Exception & ex) {
			throw IBK::Exception(ex, IBK::FormatString("Couldn't read magic file header, invalid or unrecognized file format."), FUNC_ID);
		}

		// call the actual read functions
		if (m_isBinary) {

			switch (m_majorFileVersion) {

				// delphin 6 version string
				case 0x06 :
				case 0x07 :
					// call read binary method
					try {
						readBinaryGeometryData( in );
						m_constructionLines.generate(*this);
						// close stream
						in.close();
					}
					catch (IBK::Exception &ex) {
						throw IBK::Exception(ex, "Error reading binary geometry data.", FUNC_ID);
					}
					return;

				default:
					throw IBK::Exception("Unknown major file version, not able to read this format!", FUNC_ID);
			}

		}
		else { // if (m_isBinary)

			switch (m_majorFileVersion) {

				// delphin 6 version string
				case 0x07 :
				case 0x06 : {

					in.close(); // close stream, FileReader opens the file for reading again
					std::vector< std::string > geoLines;

					if (IBK::FileReader::readAll(fname, geoLines, std::vector<std::string>(),
												 4*sizeof(unsigned int), notify) == -1)
					{
						throw IBK::Exception("Error reading ASCII geometry file!", FUNC_ID);
					}

					// call text read method
					try {
						parseGeometryData( geoLines );
						m_constructionLines.generate(*this);
					}
					catch (IBK::Exception &ex) {
						throw IBK::Exception(ex, "", FUNC_ID);
					}
					return;
				}

				default:
					throw IBK::Exception("Unknown file version, not able to read this format!", FUNC_ID);
			}

		}

	}
	catch (IBK::Exception &ex ) {
		throw IBK::Exception(ex, IBK::FormatString("Error reading geometry file '%1'.").arg(fname), FUNC_ID);
	}

}
// ----------------------------------------------------------------------------


void GeoFile::adjustFileName() {
	FUNCID(GeoFile::adjustFileName);

	if (!m_filename.isValid())
		throw IBK::Exception("Missing or invalid filename.", FUNC_ID);

	// substitute extension if not specified
	std::string ext = m_filename.extension();
	// only remove extension if d6b or d6o is used
	IBK::Path fname_wo_ext = m_filename;
	if (ext == "g6a" || ext == "g6b")
		fname_wo_ext = m_filename.withoutExtension();
	if (m_isBinary)		fname_wo_ext.addExtension(".g6b");
	else				fname_wo_ext.addExtension(".g6a");

	m_filename = fname_wo_ext;
}
// ----------------------------------------------------------------------------


void GeoFile::write(IBK::NotificationHandler * notify) const {
	FUNCID(GeoFile::write);

	// check that our filename is set correctly
	// substitute extension if not specified
	std::string ext = m_filename.extension();
	// only remove extension if d6b or d6o is used
	IBK::Path fname_wo_ext = m_filename;
	if (ext == "g6a" || ext == "g6b")
		fname_wo_ext = m_filename.withoutExtension();
	if (m_isBinary)		fname_wo_ext.addExtension(".g6b");
	else				fname_wo_ext.addExtension(".g6a");
	if (fname_wo_ext.filename() != m_filename.filename()) {
		throw IBK::Exception(IBK::FormatString("Invalid filename (extension) of geometry file '%1', should have been '%2'!\n")
			.arg(m_filename).arg(fname_wo_ext), FUNC_ID);
	}

	// open file
	std::ofstream out;
	if (!IBK::open_ofstream(out, m_filename, std::ios_base::binary))
		throw IBK::Exception(IBK::FormatString("Couldn't open file '%1'.").arg(m_filename), FUNC_ID);

	// write magic header and version number
	IBK::Version::write(out, MAGIC_NUMBER_BINARY_GEOFILE, SECOND_MAGIC_NUMBER, MAGIC_NUMBER_ASCII_GEOFILE, SECOND_MAGIC_NUMBER,
						m_isBinary, MAJOR_FILE_VERSION, MINOR_FILE_VERSION);

	if (m_isBinary) {
		try {
			writeBinaryGeometryData(out);
		}
		catch (IBK::Exception & ex) {
			throw IBK::Exception(ex, IBK::FormatString("Error writing binary geometry file '%1'.").arg(m_filename), FUNC_ID);
		}
		if (notify) notify->notify(1);
	}
	else {
		// do we need to reopen the stream in ASCII mode
		out.close();
#if defined(_WIN32) && !defined(__MINGW32__)
		out.open( fname_wo_ext.wstr().c_str(), std::ios_base::app|std::ios_base::out );
#else
		out.open( fname_wo_ext.str().c_str(), std::ios_base::app|std::ios_base::out );
#endif
		// check if successful
		if (!out)
			throw IBK::Exception(IBK::FormatString("Couldn't open file '%1'.").arg(m_filename), FUNC_ID);

		try {
			writeGeometryData( out, notify);
		}
		catch (IBK::Exception & ex) {
			throw IBK::Exception(ex, IBK::FormatString("Error writing geometry file '%1'.").arg(m_filename), FUNC_ID);
		}
		if (notify) notify->notify(1);
	}
}


void GeoFile::parseGeometryData(const std::vector<std::string> & geoLines) {

	FUNCID(GeoFile::parseGeometryData);
	try {
		unsigned int current_line = 0;

		int current = -1; // index of currently read table
		for (std::vector<std::string>::const_iterator lineIt = geoLines.begin();
			 lineIt != geoLines.end(); ++lineIt, ++current_line)
		{
			const std::string & line = *lineIt;

			// skip empty lines or lines with comments
			if (line.empty() || line[0] == '#') continue;

			if (line[0] == 'T' && line.find("TABLE") != std::string::npos) {

				if (line.find("MATERIALS")!=std::string::npos)
					current = 0;
				else if (line.find("GRID")!=std::string::npos)
					current = 1;
				else if (line.find("ELEMENT_GEOMETRY")!=std::string::npos)
					current = 2;
				else if (line.find("SIDES_GEOMETRY")!=std::string::npos)
					current = 3;
				else current = -1;

			}
			else { // if (line[0] = std::string geometry_hashcode == 'T' && line.find("TABLE") != std::string::npos) {

				std::stringstream lstrm(line);
				switch (current) {
					case -1 : continue;

					// Material data
					case 0 : {
						Material tmp;
						unsigned int coltemp;
						lstrm >> tmp.ID >> coltemp;
						if (m_majorFileVersion < 6) {
							tmp.color = IBK::Color::fromTColor(coltemp);
						}
						else {
							tmp.color = IBK::Color::fromQRgb(coltemp);
						}
						std::string tstr;
						while (lstrm >> tstr)
							tmp.name += tstr + " ";
						m_matDefs.push_back(tmp);
					} break;

					// Grid spacings
					case 1 : {

						double tmp;

						// read x widths
						if (m_grid.x_gridCoords.empty()) {

							m_grid.x_gridCoords.push_back(0);
							while (lstrm >> tmp) {
								if (tmp == 0.0) {
									throw IBK::Exception("Zero element width in x direction", FUNC_ID);
								}
								m_grid.xwidths.push_back(tmp);
								m_grid.x_coords.push_back(m_grid.x_gridCoords.back() + tmp*0.5);
								m_grid.x_gridCoords.push_back(m_grid.x_gridCoords.back() + tmp);
							}

							if (m_grid.xwidths.empty()) {
								throw IBK::Exception("Missing element widths in x direction!", FUNC_ID);
							}

						}
						else if (m_grid.y_gridCoords.empty()) {

							// read y widths
							m_grid.y_gridCoords.push_back(0);
							while (lstrm >> tmp) {
								if (tmp == 0.0) {
									throw IBK::Exception("Zero element width in y direction", FUNC_ID);
								}
								m_grid.ywidths.push_back(tmp);
								m_grid.y_coords.push_back(m_grid.y_gridCoords.back() + tmp*0.5);
								m_grid.y_gridCoords.push_back(m_grid.y_gridCoords.back() + tmp);
							}

							if (m_grid.ywidths.empty()) {
								throw IBK::Exception("Missing element widths in y direction!", FUNC_ID);
							}

						}
						else if (m_grid.z_gridCoords.empty()) {

							// read z widths
							double z_total = 0;
							while (lstrm >> tmp) {
								if ( (tmp == 0.0) && !m_grid.zwidths.empty() ) {
									throw IBK::Exception("Zero element width in z direction!", FUNC_ID);
								}
								m_grid.zwidths.push_back(tmp);
								z_total += tmp;
							}

							// calculate grid coordinates
							m_grid.z_gridCoords.push_back(z_total);
							for (std::vector<double>::iterator it = m_grid.zwidths.begin(); it != m_grid.zwidths.end(); ++it) {
								m_grid.z_coords.push_back(m_grid.z_gridCoords.back() - *it*0.5);
								m_grid.z_gridCoords.push_back(m_grid.z_gridCoords.back() - *it);
							}

							// correct the last grid coordinate to zero
							m_grid.z_gridCoords.back() = 0;
							if (m_grid.zwidths.empty()) {
								throw IBK::Exception("Missing element widths in z direction!", FUNC_ID);
							}

						}

						// determine construction type based in data in m_grid
						m_grid.updateConstructionType();

					} break;

					// Element geometry
					case 2 : {
						Element tmp;
						// check if we have a 3D case
						if ( m_grid.zwidths.size() > 1 ) {
							/// \todo Remove redundant output of element/side number
							lstrm >> tmp.n >> tmp.x >> tmp.y >> tmp.z >> tmp.i >> tmp.j >> tmp.k >> tmp.matnr;
						} else {
							lstrm >> tmp.n >> tmp.x >> tmp.y >> tmp.i >> tmp.j >> tmp.matnr;
							tmp.k = 0;
							tmp.z = 0;
						}
						if (tmp.n != m_elementsVec.size()) {
							throw IBK::Exception("Currupt element table!", FUNC_ID);
						}
						m_elementsVec.push_back(tmp);
					} break;

					// Sides geometry
					case 3 : {

						Side tmp;
						// check if we have 3D
						if ( m_grid.zwidths.size() > 1 ) {
							/// \todo Remove redudant output of element/side number
							lstrm >> tmp.n >> tmp.x >> tmp.y >> tmp.z >> tmp.i >> tmp.j >> tmp.k >> tmp.orientation;
						}	else {
							lstrm >> tmp.n >> tmp.x >> tmp.y >> tmp.i >> tmp.j >> tmp.orientation;
							tmp.k = 0;
							tmp.z = 0;
						}
						if (tmp.n != m_sidesVec.size()) {
							throw IBK::Exception("Currupt sides table!", FUNC_ID);
						}
						m_sidesVec.push_back(tmp);

					} break;

				}
			}
		}

		// for file versions < 7, fix column index
		fixYCoordinates();
	}
	catch (IBK::Exception &ex) {
		throw IBK::Exception(ex, "Error on parse geometry data.", FUNC_ID);
	}
}


void GeoFile::writeGeometryData(std::ostream& out, IBK::NotificationHandler * notify) const {
	FUNCID(GeoFile::writeGeometryData);

	// write material tabe
	if (m_matDefs.empty())
		throw IBK::Exception("Missing data in materials table.", FUNC_ID);

	out << "\nTABLE  MATERIALS" << '\n';
	for (std::vector<Material>::const_iterator it = m_matDefs.begin(); it != m_matDefs.end(); ++it)
		out << std::setw(10) << std::left << it->ID << " "
			<< std::setw(10) << std::left << it->color.toQRgb() << " "
			<< it->name << '\n';
	out << '\n';
	if (notify) notify->notify(0.1);
	// write grid table
	if (m_grid.xwidths.empty())
		throw IBK::Exception("Missing x-grid discretization data.", FUNC_ID);
	if (m_grid.ywidths.empty())
		throw IBK::Exception("Missing y-grid discretization data.", FUNC_ID);
	if (m_grid.zwidths.empty())
		throw IBK::Exception("Missing z-grid discretization data.", FUNC_ID);

	out << "TABLE  GRID" << '\n';
	std::copy(m_grid.xwidths.begin(), m_grid.xwidths.end(), std::ostream_iterator<double>(out, " "));
	out << '\n';
	std::copy(m_grid.ywidths.begin(), m_grid.ywidths.end(), std::ostream_iterator<double>(out, " "));
	out << '\n';
	if (notify) notify->notify(0.2);

	// do we have 3D here?
	if ( m_grid.zwidths.size() > 1 ) {
		std::copy(m_grid.zwidths.begin(), m_grid.zwidths.end(), std::ostream_iterator<double>(out, " "));
		out << '\n';
	} else {
		out << m_grid.zwidths.front() << '\n';
	}

	// write element geometry table
	if (m_elementsVec.empty())
		throw IBK::Exception("Missing element geometry data.", FUNC_ID);

	out << "\nTABLE  ELEMENT_GEOMETRY" << '\n';
	unsigned int percentage=20;
	for (unsigned int i=0; i<m_elementsVec.size(); ++i) {
		unsigned int perc = 20 + (unsigned int)(i*40.0/m_elementsVec.size());
		if (notify && perc != percentage) {
			notify->notify((percentage+1)/100.0);
			percentage = perc;
		}
		const Element & E = m_elementsVec[i];

		// do we have 3D grid here?
		if ( m_grid.zwidths.size() > 1 ) {

			/// \todo Remove redudant output of element/side number
			out << std::setw(8)  << std::left << E.n << ' ';
			std::stringstream nicestrm;
			nicestrm.precision(12);
			nicestrm	<< std::setw(9) << std::left << E.x << " "
						<< std::setw(9) << std::left << E.y << " "
						<< std::setw(9) << std::left << E.z;

			out << std::setw(30) << std::left << nicestrm.str() << "\t ";
			out << std::setw(5) << std::left << E.i << " "
				<< std::setw(5) << std::left << E.j << " "
				<< std::setw(5) << std::left << E.k << " ";
			out << E.matnr << '\n';

		} else {

			out << std::setw(8)  << std::left << E.n << ' ';
			std::stringstream nicestrm;
			nicestrm.precision(12);
			nicestrm << std::setw(9) << std::left << E.x << " " << E.y;
			out << std::setw(20) << std::left << nicestrm.str() << "\t ";
			out << std::setw(5) << std::left << E.i << " "
				<< std::setw(5) << std::left << E.j << " ";
			out << E.matnr << '\n';

		}

	}

	// write sides geometry table
	if (m_sidesVec.empty()) {
		IBK::IBK_Message("Missing side geometry data (no sides specified, flux outputs won't be possible).",
						 IBK::MSG_WARNING, FUNC_ID);
	}
	else {

		out << "\nTABLE  SIDES_GEOMETRY" << '\n';
		for (unsigned int i=0; i<m_sidesVec.size(); ++i) {
			unsigned int perc = 60 + (unsigned int)(i*40.0/m_sidesVec.size());
			if (notify && perc != percentage) {
				notify->notify((percentage+1)/100.0);
				percentage = perc;
			}
			const Side & S = m_sidesVec[i];

			/// \todo Remove redudant output of element/side number
			out << std::setw(8)  << std::left << S.n << ' ';
			std::stringstream nicestrm;
			nicestrm.precision(12);

			// do we have 3D grid here?
			if ( m_grid.zwidths.size() > 1 ) {

				nicestrm	<< std::setw(9) << std::left << S.x << " "
							<< std::setw(9) << std::left << S.y << " "
							<< std::setw(9) << std::left << S.z;
				out << std::setw(20) << std::left << nicestrm.str() << "\t "
					<< std::setw(6) << std::left << S.i << " "
					<< std::setw(6) << std::left << S.j << " "
					<< std::setw(6) << std::left << S.k << " "
					<< std::setw(5) << std::left << S.orientation << '\n';

			} else {

				nicestrm << std::setw(9) << std::left << S.x << " "
						 << std::setw(9) << std::left << S.y;
				out << std::setw(20) << std::left << nicestrm.str() << "\t "
					<< std::setw(6) << std::left << S.i << " "
					<< std::setw(6) << std::left << S.j << " "
					<< std::setw(5) << std::left << S.orientation << '\n';

			} // else if 3D

		}

		out << std::endl;
	}

	if (!out.good())
		throw IBK::Exception("Error writing geometry data to file.", FUNC_ID);
}


void GeoFile::readBinaryGeometryData(std::istream& in) {
	FUNCID(GeoFile::readBinaryGeometryData);

	IBK_ASSERT(m_majorFileVersion != 0);

	try {
		// read materials section
		unsigned int s;
		in.read(reinterpret_cast<char *>(&s), sizeof(unsigned int));

		if (s>100000) {
			throw IBK::Exception("Number of materials in materials table > 100000, binary file probably corrupt!", FUNC_ID);
		}

		m_matDefs.resize(s);

		for (std::vector<Material>::iterator it = m_matDefs.begin(); it != m_matDefs.end(); ++it) {
			in.read(reinterpret_cast<char *>(&it->ID), sizeof(unsigned int));
			unsigned int colQRgb;
			in.read(reinterpret_cast<char *>(&colQRgb), sizeof(unsigned int));
			it->color = IBK::Color::fromQRgb(colQRgb);
			IBK::read_string_binary(in, it->name, 10000);
		}

		// read grid section
		IBK::read_vector_binary(in, m_grid.xwidths, 10000);
		IBK::read_vector_binary(in, m_grid.ywidths, 10000);
		IBK::read_vector_binary(in, m_grid.zwidths, 10000);


		// calculate x-grid coordinates
		m_grid.x_gridCoords.resize(m_grid.xwidths.size()+1);
		m_grid.x_gridCoords[0] = 0;
		m_grid.x_coords.resize(m_grid.xwidths.size());

		for (unsigned int i=0; i<m_grid.xwidths.size(); ++i) {
			if (m_grid.xwidths[i] <= 0.0)
				throw IBK::Exception("Zero or negative element width in x direction", FUNC_ID);

			m_grid.x_coords[i] = m_grid.x_gridCoords[i] + m_grid.xwidths[i]*0.5;
			m_grid.x_gridCoords[i+1] = m_grid.x_gridCoords[i] + m_grid.xwidths[i];
		}


		// calculate y-grid coordinates
		m_grid.y_gridCoords.resize(m_grid.ywidths.size()+1);
		m_grid.y_gridCoords[0] = 0;
		m_grid.y_coords.resize(m_grid.ywidths.size());

		for (unsigned int i=0; i<m_grid.ywidths.size(); ++i) {
			if (m_grid.ywidths[i] <= 0.0)
				throw IBK::Exception("Zero or negative element width in y direction",FUNC_ID);

			m_grid.y_coords[i] = m_grid.y_gridCoords[i] + m_grid.ywidths[i]*0.5;
			m_grid.y_gridCoords[i+1] = m_grid.y_gridCoords[i] + m_grid.ywidths[i];
		}


		// calculate z-grid coordinates
		m_grid.z_gridCoords.resize(m_grid.zwidths.size()+1);
		m_grid.z_gridCoords[0] = 0;
		m_grid.z_coords.resize(m_grid.zwidths.size());

		for (unsigned int i=0; i<m_grid.zwidths.size(); ++i) {
			if (m_grid.zwidths[i] <= 0.0)
				throw IBK::Exception("Zero or negative element width in z direction", FUNC_ID);

			m_grid.z_coords[i] = m_grid.z_gridCoords[i] + m_grid.zwidths[i]*0.5;
			m_grid.z_gridCoords[i+1] = m_grid.z_gridCoords[i] + m_grid.zwidths[i];
		}


		// read element and side geometry
		if (m_majorFileVersion < 6) {
			// D5 output files used 2D records in binary files, therefore we
			// first read binary 2D records and then copy them over into our
			// 3D data records.
			std::vector<Element2D> elementsVec;
			std::vector<Side2D> sidesVec;
			IBK::read_vector_binary(in, elementsVec, 10000000);
			IBK::read_vector_binary(in, sidesVec, 50000000);
			// convert to 3D vectors, using zwidth[0] parameter for z-coordinate calculation
			double z2;
			if (m_grid.zwidths.empty())
				z2 = 0;
			else
				z2 = m_grid.zwidths[0] / 2.0;
			m_elementsVec.resize(elementsVec.size());
			m_sidesVec.resize(sidesVec.size());
			for (unsigned int i = 0; i < sidesVec.size(); ++i) {
				Side & S = m_sidesVec[i];
				Side2D & Sold = sidesVec[i];
				S.n = Sold.n;
				S.x = Sold.x;
				S.y = Sold.y;
				S.z = z2;
				S.i = Sold.i;
				S.j = Sold.j;
				S.k = 0;
				S.orientation = Sold.vertical ? 1 : 0;
			}
			for (unsigned int i = 0; i < elementsVec.size(); ++i) {
				Element & E = m_elementsVec[i];
				Element2D & Eold = elementsVec[i];
				E.n = Eold.n;
				E.x = Eold.x;
				E.y = Eold.y;
				E.z = z2;
				E.i = Eold.i;
				E.j = Eold.j;
				E.k = 0;
				E.matnr = Eold.matnr;
			}
		}
		else {
			IBK::read_vector_binary(in, m_elementsVec, 10000000);
			IBK::read_vector_binary(in, m_sidesVec, 50000000);
		}

		// determine construction type based in data in m_grid
		m_grid.updateConstructionType();

		// for file versions < 7, fix column index
		fixYCoordinates();
	}
	catch (IBK::Exception &ex) {

		throw IBK::Exception(ex, "Error on read binary geometry data.", FUNC_ID);
	}
}


void GeoFile::writeBinaryGeometryData(std::ostream& out) const {
	FUNCID(GeoFile::writeBinaryGeometryData);

	// write material tabe
	if (m_matDefs.empty())
		throw IBK::Exception("Missing data in materials table.", FUNC_ID);

	// write number of entries
	unsigned int s = static_cast<unsigned int>(m_matDefs.size());
	out.write(reinterpret_cast<char *>(&s), sizeof(unsigned int));

	// write data
	for (std::vector<Material>::const_iterator it = m_matDefs.begin(); it != m_matDefs.end(); ++it) {
		out.write(reinterpret_cast<const char *>(&it->ID), sizeof(unsigned int));
		unsigned int colQRgb = it->color.toQRgb();
		out.write(reinterpret_cast<const char *>(&colQRgb), sizeof(unsigned int));
		IBK::write_string_binary(out,it->name);
	}

	// write grid table
	if (m_grid.xwidths.empty())
		throw IBK::Exception("Missing x-grid discretization data.", FUNC_ID);
	if (m_grid.ywidths.empty())
		throw IBK::Exception("Missing y-grid discretization data.", FUNC_ID);
	if (m_grid.zwidths.empty())
		throw IBK::Exception("Missing z-grid discretization data.", FUNC_ID);

	IBK::write_vector_binary(out, m_grid.xwidths);
	IBK::write_vector_binary(out, m_grid.ywidths);
	IBK::write_vector_binary(out, m_grid.zwidths);

	// write elements vector
	if (m_elementsVec.empty())
		throw IBK::Exception("Missing element geometry data.", FUNC_ID);
	IBK::write_vector_binary(out, m_elementsVec);

	// write sides geometry table
	if (m_sidesVec.empty())
		throw IBK::Exception("Missing side geometry data.", FUNC_ID);

	IBK::write_vector_binary(out, m_sidesVec);

	if (!out.good())
		throw IBK::Exception("Error writing geometry data to file.", FUNC_ID);
}


unsigned int GeoFile::hashCode() {
	FUNCID(GeoFile::hashCode);

	// write everything into a string stream
	try {
		if (m_isBinary) {
			std::stringbuf buf;
			std::iostream iostrm(&buf);
			writeBinaryGeometryData(iostrm);
			iostrm.flush();
			//iostrm.seekg(std::ios_base::end);
			std::streampos endPos = iostrm.tellp(); // size of stream
			std::vector<char> buffer( (unsigned int)endPos);
			std::streamsize receivedSize = iostrm.rdbuf()->sgetn( &buffer[0], buffer.size() );
			// get char array for it and return the hash value
			return IBK::SuperFastHash( &buffer[0], (int)receivedSize);
		}
		else {
			// create string with all data
			std::stringstream	stream;
			writeGeometryData(stream, nullptr);
			// get char array for it and return the hash value
			return IBK::SuperFastHash( stream.str() );
		}
	}
	catch (IBK::Exception & ex) {
		throw IBK::Exception(ex, "Error writing geometry data.", FUNC_ID);
	}
}


void GeoFile::checkForConsistantMaterialIndexes() const {
	for (unsigned int i=0; i<m_elementsVec.size(); ++i) {
		if (m_elementsVec[i].matnr >= m_matDefs.size())
			throw IBK::Exception(IBK::FormatString("Invalid material index in element definition #%1.").arg(i),
								 "[GeoFile::checkForConsistantMaterialIndexes]");
	}
}


void GeoFile::clear() {

	m_filename.clear();
	m_projectFileHashCode.clear();

	m_matDefs.clear();
	m_grid = Grid();
	m_elementsVec.clear();
	m_sidesVec.clear();

	m_majorFileVersion = 0; // undefined
}


void GeoFile::Grid::updateConstructionType() {
	// determine construction type
	if (zwidths.size() > 1 ) {
		// full 3D structure
		constructionType = 3;
	}
	else if (zwidths.size() == 1 && zwidths.back() == 0) {
		// Rotation symmetric 3D structure
		constructionType = 2;
	}
	else if (xwidths.size() > 1 && ywidths.size() > 1 ) {
		// "Plane 2D structure"
		constructionType = 1;
	}
	else {
		// "Plane 1D structure"
		constructionType = 0;
	}
}


void GeoFile::fixYCoordinates() {
	// fix y-grid coordinates and element indexes for files with version less than 7
	if (m_majorFileVersion < 7 && !m_grid.ywidths.empty()) {
		// maximum row index corresponds to maximum y grid cell count - 1
		unsigned int maxJ = (unsigned int)m_grid.ywidths.size() - 1;
		// adjust j row index
		for (unsigned int i = 0; i < (unsigned int)m_elementsVec.size(); ++i) {
			Element & E = m_elementsVec[i];
			E.j = maxJ - E.j;
		}

		// also reverse y-grid coordinates
		std::reverse(m_grid.ywidths.begin(), m_grid.ywidths.end());
		m_grid.y_gridCoords.resize(m_grid.ywidths.size()+1);
		m_grid.y_coords.resize(m_grid.ywidths.size());
		m_grid.y_gridCoords[0] = 0;
		for (unsigned int i=0; i<m_grid.ywidths.size(); ++i) {
			m_grid.y_gridCoords[i+1] = m_grid.y_gridCoords[i] + m_grid.ywidths[i];
			m_grid.y_coords[i] = m_grid.y_gridCoords[i] + m_grid.ywidths[i]*0.5;
		}
	}
}






} // namespace DATAIO
