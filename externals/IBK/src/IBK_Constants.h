/*	Copyright (c) 2001-2017, Institut für Bauklimatik, TU Dresden, Germany

	Written by A. Nicolai, H. Fechner, St. Vogelsang, A. Paepcke, J. Grunewald
	All rights reserved.

	This file is part of the IBK Library.

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


	This library contains derivative work based on other open-source libraries.
	See OTHER_LICENCES and source code headers for details.

*/

#ifndef IBK_ConstantsH
#define IBK_ConstantsH

namespace IBK {

/*! used to specify a unused or invalid item index. */
extern const unsigned int INVALID_ELEMENT;

/*! The library version (full version number). */
extern const char * const VERSION;

/*! The seconds in each of the twelve months. */
extern unsigned int SECONDS_PER_MONTH[12];

/*! The seconds up to the given month. */
extern unsigned int SECONDS_UNTIL_MONTH[12];

/*! The abbreviated names of the months for date In/Output. */
extern const char * const MONTH_NAMES[12];

/*! Seconds in a regular year. */
extern const unsigned int SECONDS_PER_YEAR;

/*! Seconds per day. */
extern const unsigned int SECONDS_PER_DAY;

/*! Current start id for user space in the various databases. */
extern const unsigned int USER_ID_START;

/*! ID used to identify VOID types, for example VOID materials.
	\warning Changing this constant may cause problems with older project/data files that store
	the old IDs and new software versions would not recognize these materials as VOIDs.
*/
extern const unsigned int VOID_ID;


/*! \todo rename to something related to materials. */
enum materialDBTypes {
	MATDB_TYPE_DEFAULT,
	MATDB_TYPE_ALL,
	MATDB_TYPE_COND,
	MATDB_TYPE_DELPHIN5,
	MATDB_TYPE_DELPHIN6
};

/*! Default placeholder name to be used to reference the project directory. */
extern const char * const PLACEHOLDER_PROJECT_DIR;
/*! Default placeholder name to be used to reference the installation location of the GUI/solver binary. */
extern const char * const PLACEHOLDER_INSTALL_DIR;
/*! Default placeholder name to be used to reference the material database directory. */
extern const char * const PLACEHOLDER_MATERIALS_DIR;
/*! Default placeholder name to be used to reference the climate database directory. */
extern const char * const PLACEHOLDER_CLIMATE_DIR;

} // namespace IBK

/*! \file IBK_Constants.h
	\brief Program and library constants.
*/

#endif // IBK_ConstantsH
