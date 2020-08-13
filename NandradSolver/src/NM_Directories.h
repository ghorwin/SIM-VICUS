/*	NANDRAD Solver Framework and Model Implementation.

	Copyright (c) 2012-today, Institut für Bauklimatik, TU Dresden, Germany

	Primary authors:
	  Andreas Nicolai  <andreas.nicolai -[at]- tu-dresden.de>
	  Anne Paepcke     <anne.paepcke -[at]- tu-dresden.de>

	This library is part of SIM-VICUS (https://github.com/ghorwin/SIM-VICUS)

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 3 of the License, or (at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	Lesser General Public License for more details.
*/

#ifndef NM_DirectoriesH
#define NM_DirectoriesH

#include <string>

#include <IBK_Path.h>

namespace NANDRAD_MODEL {

/*!	Wraps the model's directory handling.
	\todo Check if all directories/paths in this class are really needed.
*/
class Directories {
public:
	/*! Creates the directory structure for a given project.
		Throws an exception if the directories cannot be created.
		After successful completion, the member variables contain the
		absolute paths to the individual subdirectories.
		\param projectpath The path to the project file.
		\param projectfile The project file.
	*/
	void create(const IBK::Path &projectRootPath);

	/*! Creates additional directory for FMU slaves output.
	Must be calles after create and will beoptionally called if
	FMU slaves are registered.
	Throws an exception if the directories cannot be created.
	*/
	void createFMUSlavesDirectory();

	IBK::Path		m_rootDir;					///< Project root directory (without trailing slash).
	IBK::Path		m_logDir;					///< Project log directory (without trailing slash).
	IBK::Path		m_varDir;					///< Project var directory (without trailing slash).
	IBK::Path		m_resultsDir;				///< Project output directory (without trailing slash).
	IBK::Path		m_slavesDir;				///< Project output directory for fmu slaves (without trailing slash).

	IBK::Path		m_heatingDesignRootDir;		///< Root directory for heating design calculation (without trailing slash).
	IBK::Path		m_coolingDesignRootDir;		///< Root directory for cooling design calculation (without trailing slash).
	IBK::Path		m_heatingDesignResultsDir;	///< Results directory for heating design calculation (without trailing slash).
	IBK::Path		m_coolingDesignResultsDir;	///< Results directory for cooling design calculation (without trailing slash).
};


} // namespace NANDRAD_MODEL

#endif // NM_DirectoriesH
