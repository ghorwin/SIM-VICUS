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

#include "DATAIO_Utils.h"

#include <IBK_messages.h>
#include <IBK_StringUtils.h>

#include "DATAIO_GeoFile.h"

namespace DATAIO {


void listDataIOHeader(const IBK::Path &inputFilePath) {

	DATAIO::DataIO dFile;
	dFile.read(inputFilePath);

	// compose path to geofile
	DATAIO::GeoFile gFile;
	try {
		if (dFile.m_geoFileName.isValid()) {
			IBK::Path geoFilePath = inputFilePath.parentPath() / dFile.m_geoFileName;
			gFile.read(geoFilePath);
		}
	}
	catch(IBK::Exception&) {
		IBK::IBK_Message("Cannot read geofile from " + inputFilePath.str(), IBK::MSG_WARNING, "DataIO::listDataIOHeader");
	}

	listDataIOHeader(dFile, gFile);
}


void listDataIOHeader( const DataIO & dFile, GeoFile & gFile) {

	std::string output;
	// output type info
	switch (dFile.m_type) {
		case DataIO::T_FIELD  :
			output = "Output type    : FIELD\n"; break;
		case DataIO::T_FLUX  :
			output = "Output type    : FLUX\n"; break;
		case DataIO::T_REFERENCE  :
			output = "Output type    : SINGLE\n"; break;
		default :
			output = "Output type    : <...>\n";
	}
	IBK::IBK_Message(output,IBK::MSG_PROGRESS);
	IBK::IBK_Message("Quantity       : " + dFile.m_quantity + "\n",IBK::MSG_PROGRESS);
	// output space type info
	switch (dFile.m_spaceType) {
		case DataIO::ST_SINGLE 	:
			output = "Space type     : SINGLE\n"; break;
		case DataIO::ST_MEAN 	:
			output = "Space type     : MEAN\n"; break;
		case DataIO::ST_INTEGRAL 	:
			output = "Space type     : INTEGRAL\n"; break;
		default 	:
			output = "Space type     : <...>\n";
	}
	IBK::IBK_Message(output,IBK::MSG_PROGRESS);
	// output time type info
	switch (dFile.m_timeType) {
		case DataIO::TT_NONE 	:
			output = "Time type      : NONE\n"; break;
		case DataIO::TT_MEAN 	:
			output = "Time type      : MEAN\n"; break;
		case DataIO::TT_INTEGRAL 	:
			output = "Time type      : INTEGRAL\n"; break;
		default :
			output = "Time type      : <...>\n";
	}
	IBK::IBK_Message(output,IBK::MSG_PROGRESS);
	IBK::IBK_Message("Value unit     : " + dFile.m_valueUnit + "\n",IBK::MSG_PROGRESS);
	IBK::IBK_Message("Time unit      : " + dFile.m_timeUnit + "\n",IBK::MSG_PROGRESS);

	IBK::IBK_Message("Start year     : " + IBK::val2string(dFile.m_startYear) + "\n",IBK::MSG_PROGRESS);

	// print data format type
	DATAIO::DataIO::dataFormat_t ot = dFile.dataFormat();
	output = "Data format    : " + DataIO::dataFormat2String(ot) + "\n";
	IBK::IBK_Message(output,IBK::MSG_PROGRESS);

	// set elements/sides info
	switch (dFile.m_type) {
		case DataIO::T_FIELD  :
			output = "Elements       : "; break;
		case DataIO::T_FLUX  :
			output = "Sides          : "; break;
		case DataIO::T_REFERENCE  :
			output = "Reference      : "; break;
		default :
			output = "Elements/Sides : ";
	}

	if ( dFile.m_nums.size() > 0 && dFile.m_type != DataIO::NUM_T )
		output = output + IBK::val2string( dFile.m_nums.size() ) + "\n";
	else
		output = output + "<...>\n";
	IBK::IBK_Message(output,IBK::MSG_PROGRESS);

	// add geometry info
	if (gFile.m_matDefs.empty())
		output = "Materials      : <...>\n";
	else
		output = "Materials      : " + IBK::val2string(gFile.m_matDefs.size()) + "\n";
	IBK::IBK_Message(output,IBK::MSG_PROGRESS);
	switch (gFile.m_grid.constructionType) {
		case 0 :
			output = "Geometry       : Plane 1D structure\n"; break;
		case 1 :
			output = "Geometry       : Plane 2D structure"; break;
		case 2 :
			output = "Geometry       : Rotation symmetric 3D structure"; break;
		case 3 :
			output = "Geometry       : 3D structure"; break;
		default :
			output = "Geometry       : <...>\n"; break;
	}
	IBK::IBK_Message(output,IBK::MSG_PROGRESS);
	output = std::string();
	if (gFile.m_grid.constructionType >= 0) {
		if (!gFile.m_grid.xwidths.empty() && !gFile.m_grid.ywidths.empty()) {
			output = ", " + IBK::val2string(gFile.m_grid.xwidths.size()) + (gFile.m_grid.xwidths.size()==1 ? " column, " : " columns, ")
				+ IBK::val2string(gFile.m_grid.ywidths.size()) + (gFile.m_grid.ywidths.size()==1 ? " row\n" : " rows\n");
		}
		IBK::IBK_Message(output,IBK::MSG_PROGRESS);
	}
	output = "Time points    : " + IBK::val2string(dFile.m_timepoints.size()) + "\n";
	IBK::IBK_Message(output,IBK::MSG_PROGRESS);

}


} // namespace DATAIO

