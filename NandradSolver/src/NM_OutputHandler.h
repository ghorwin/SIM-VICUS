/*	NANDRAD Solver Framework and Model Implementation.

	Copyright (c) 2012-today, Institut für Bauklimatik, TU Dresden, Germany

	Primary authors:
	  Andreas Nicolai  <andreas.nicolai -[at]- tu-dresden.de>
	  Anne Paepcke     <anne.paepcke -[at]- tu-dresden.de>

	This program is part of SIM-VICUS (https://github.com/ghorwin/SIM-VICUS)

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.
*/

#ifndef NM_OutputHandlerH
#define NM_OutputHandlerH

#include <vector>
#include <map>

#include <IBK_Unit.h>
#include <IBK_Path.h>

#include "NM_QuantityDescription.h"

namespace IBK {
	class StopWatch;
}

namespace NANDRAD {
	class Project;
}

namespace NANDRAD_MODEL {

class OutputFile;
class AbstractModel;

/*! Handles all output file initialization and the process of caching and writing
	outputs.

	During simulation, this class is called to store/append outputs whenever an output time point has been
	reached in one of the ouput grids.

	The output handler then notifies all output files to cache new output values (when respective grid is active).
	After some real time (or when output cache exceeds a limit), the output handler requests the output files
	to flush their caches.

	Output files are created on first call.

	\note Note that output handler operates with three time points (within writeOutputs()):
	- \a t_out - Output time point in [s] in simulation time (i.e. starts with 0 when solver starts). \a t_out is
		needed for interpolating integral values and is passed to output files when writing outputs (to cache).
	- \a t_secondsOfYear - Time point in [s] relative to Midnight January 1st of start year (t_out + startTime offset).
		\a t_timeOfYear is written in first column of output files and is also passed to output files
	- \a t_timeOfYear - Time point in [<output unit>] converted from t_secondsOfYear.
		\a t_secondsOfYear is used to check if an output grid is active.

*/
class OutputHandler {
public:

	/*! Different names for auto-generated files based on quantity type. */
	enum OutputFileNames {
		OFN_States,				// Keyword: states
		OFN_Loads,				// Keyword: loads
		OFN_LoadIntegrals,		// Keyword: load_integrals
		OFN_Fluxes,				// Keyword: fluxes
		OFN_FluxIntegrals,		// Keyword: flux_integrals
		OFN_Network,			// Keyword: network
		OFN_NetworkElements,	// Keyword: network_elements
		OFN_Misc,				// Keyword: misc
		NUM_OFN
	};

	/*! D'tor. */
	~OutputHandler();

	/*! Main initialization function.

		\note During init, the OutputDefinitions in the project are modified such that the quick-access pointers
			  to grids and object lists are created. The vectors themselves remain unchanged (no invalidation of
			  previously obtained pointers).

		\param restart If true, simulation was restarted and output files shall be continued, rather than re-created.
		\param prj The project data structure with all input data.
		\param outputPath Path to directory where output files should be created in.
	*/
	void setup(bool restart, NANDRAD::Project & prj, const IBK::Path & outputPath);

	/*! Appends outputs to files.
		Actually, this function only caches current output values. Only when a certain
		time has passed (or cached output data exceeds a certain limit), the data is actually written to file.

		\param t_out Output time point (in seconds, simulation time).
		\param t_secondsOfYear Output time point as offset to Midnight January 1st in the start year.
		\param varSubstitutionMap map containing substitutions for header labels in files
	*/
	void writeOutputs(double t_out, double t_secondsOfYear, const std::map<std::string, std::string> & varSubstitutionMap);

	/*! When called, asks all output files to flush their cached data to file. */
	void flushCache();


	/*! Vector with output file objects.
		Objects are not owned by us, but rather by NandradModel, since the output file objects
		are also model objects and are inserted into the model container.
	*/
	std::vector<OutputFile*>					m_outputFiles;

	/*! If true, solver is restarted.
		This flag is used when the first output is being written to decide whether to
		re-open files or to create files from scratch and writing headers.
	*/
	bool										m_restart;

	/*! Cached flag if using binary files or not. */
	bool										m_binaryFiles;

	/*! Unit to be used for time points in output files. */
	IBK::Unit									m_timeUnit;

	/*! A stop watch that monitors real time to check when outputs need to be written.
		Object will be created on first writeOutputs() call, and is owned by this object.
	*/
	IBK::StopWatch								*m_outputTimer = nullptr;

	/*! Pointer to path with output files (not owned). */
	const IBK::Path								*m_outputPath = nullptr;

	/*! Number of bytes to hold in cache before flushing the cache. */
	unsigned int								m_outputCacheLimit;

	/*! Number of seconds to wait before before flushing the cache. */
	double										m_realTimeOutputDelay;
};


} // namespace NANDRAD_MODEL

#endif // NM_OutputHandlerH
