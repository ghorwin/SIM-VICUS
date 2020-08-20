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

#include "NM_OutputHandler.h"

#include <algorithm>
#include <array>
#include <memory>

#include <IBK_StringUtils.h>
#include <IBK_messages.h>
#include <IBK_Exception.h>
#include <IBK_StopWatch.h>
#include <IBK_UnitList.h>

#include <NANDRAD_Project.h>

#include "NM_OutputFile.h"
#include "NM_QuantityName.h"
#include "NM_KeywordList.h"

const char * const STATE_QUANTITIES[] = {
	"AirTemperature"
};

const char * const FLUX_QUANTITIES[] = {
	"ConvectiveHeatFluxDensity"
};

namespace NANDRAD_MODEL {

OutputHandler::~OutputHandler() {
	delete m_outputTimer;
	// Note: Objects m_outputFiles are owned by NandradModel (stored in m_modelContainer).
}


void OutputHandler::setup(bool restart, NANDRAD::Project & prj, const IBK::Path & outputPath) {
	FUNCID(OutputHandler::setup);

	// cache parameters needed to create output files
	m_restart = restart; // store restart info flag
	m_outputPath = &outputPath;
	m_binaryFiles = prj.m_outputs.m_binaryFormat.isEnabled();
	m_timeUnit = prj.m_outputs.m_timeUnit;
	if (m_timeUnit.base_id() != IBK_UNIT_ID_SECONDS) {
		throw IBK::Exception( IBK::FormatString("Output time unit '%1' is not a valid time unit.").arg(m_timeUnit.name()), FUNC_ID);
	}

	m_outputCacheLimit = 100000000; // ~100 Mb for starters
	m_realTimeOutputDelay = 10; // wait a few seconds second simulation time before flushing the cache

	// initialize and check output grids
	{
		IBK::IBK_Message("Initializing output grids\n", IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
		IBK_MSG_INDENT;

		std::vector<NANDRAD::OutputGrid> usedGrids;
		for (NANDRAD::OutputGrid & og : prj.m_outputs.m_grids) {
			// search output definitions until we find one that uses this grid
			for (const NANDRAD::OutputDefinition & od : prj.m_outputs.m_outputDefinitions) {
				if (od.m_gridName == og.m_name) {
					usedGrids.push_back(og);
					break;
				}
			}
		}
		if (usedGrids.size() < prj.m_outputs.m_grids.size())
			IBK::IBK_Message(IBK::FormatString("Removing %1 unused output grids").arg(prj.m_outputs.m_grids.size() - usedGrids.size()),
							 IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
		prj.m_outputs.m_grids.swap(usedGrids);
		for (NANDRAD::OutputGrid & og : prj.m_outputs.m_grids) {
			IBK::IBK_Message(IBK::FormatString("Checking output grid '%1'\n").arg(og.m_name),
							 IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);

			try {
				og.checkIntervalDefinition();
			}
			catch (IBK::Exception & ex) {
				throw IBK::Exception(ex, IBK::FormatString("Error in definition of output grid '%1'.").arg(og.m_name), FUNC_ID);
			}
			og.setupIntervals();
		}
	}

	// we create groups of output definitions, first based on target file name

	// in the initial loop, we also create and store pointer references to output grids and object lists referenced
	// from output definitions - in case of invalid definitions, we bail out with an error message
	std::map<std::string, std::vector<NANDRAD::OutputDefinition> > targetFileMap;
	for (unsigned int i=0; i<prj.m_outputs.m_outputDefinitions.size(); ++i) {

		NANDRAD::OutputDefinition & od = prj.m_outputs.m_outputDefinitions[i];

		// sanity checks are done on the fly

		// ** resolve output grid
		if (IBK::trim_copy(od.m_gridName).empty())
			throw IBK::Exception(IBK::FormatString("Output definition #%1 has empty/invalid output grid name.").arg(i), FUNC_ID);

		std::vector<NANDRAD::OutputGrid>::const_iterator grid_it = std::find(prj.m_outputs.m_grids.begin(),
																			 prj.m_outputs.m_grids.end(),
																			 od.m_gridName);
		if (grid_it == prj.m_outputs.m_grids.end())
			throw IBK::Exception(IBK::FormatString("Output definition #%1 references unknown/undefined output grid '%2'").arg(i).arg(od.m_gridName), FUNC_ID);

		// store pointer link to output grid
		od.m_gridRef = &(*grid_it);


		// ** resolve object list

		if (IBK::trim_copy(od.m_objectListName).empty())
			throw IBK::Exception(IBK::FormatString("Output definition #%1 has empty/invalid object list name.").arg(i), FUNC_ID);

		std::vector<NANDRAD::ObjectList>::const_iterator oblst_it = std::find(prj.m_objectLists.begin(),
																			  prj.m_objectLists.end(),
																			  od.m_objectListName);
		if (oblst_it == prj.m_objectLists.end())
			throw IBK::Exception(IBK::FormatString("Output definition #%1 references unknown/undefined object list '%2'").arg(i).arg(od.m_objectListName), FUNC_ID);

		// store pointer link to output grid
		od.m_objectListRef = &(*oblst_it);


		// ** defaults

		if (od.m_timeType == NANDRAD::OutputDefinition::NUM_OTT)
			od.m_timeType = NANDRAD::OutputDefinition::OTT_NONE;

		// ** look for specified filename

		std::string outputFname = IBK::trim_copy(od.m_filename);
		// if output filename is not empty, make sure that all output definitions in same file use the
		// same output grid
		if (!outputFname.empty() && !targetFileMap[outputFname].empty()) {
			if (targetFileMap[outputFname].back().m_gridName != od.m_gridName) {
				throw IBK::Exception(IBK::FormatString("Output definition #%1 requests output file '%2' and references output grid '%3'. However, previous "
													   "output definitions for same output file use a different output grid '%4', which is not allowed.")
									 .arg(i).arg(od.m_filename).arg(od.m_gridName).arg(targetFileMap[outputFname].back().m_gridName), FUNC_ID);
			}
		}

		// store object list definition in map; those without filename are stored with key = string() = ""
		targetFileMap[outputFname].push_back(od);
	}

	// now process all unnamed output definitions and group them by output grid name
	const std::vector<NANDRAD::OutputDefinition> & odlist = targetFileMap[""]; // may be empty

	// we now create for each output grid a list of output file types:
	// - states
	// - (climatic) loads
	// - fluxes  (all quantities with /s as unit)
	// - flux_integrals (integrals of flux quantities)
	// - misc (all others)

	// we store the files in a map of array, so that we can access the data via:
	//   groupMap("hourly_grid")[OFN_States] -> vector with all output definitions using grid "hourly_grid"
	//   and state-quantities
	std::map<std::string, std::array<std::vector<NANDRAD::OutputDefinition>, NUM_OFN> > groupMap;

	for (const NANDRAD::OutputDefinition & od : odlist) {
		// determine quantity type

		// anything related to climatic loads, will get into OFN_Loads
		if (od.m_objectListRef->m_referenceType == NANDRAD::ModelInputReference::MRT_LOCATION) {
			groupMap[od.m_gridName][OFN_Loads].push_back(od);
			continue;
		}

		// strip brackets from quantity name, if given
		QuantityName qn;
		try {
			qn.fromEncodedString(od.m_quantity);
		}
		catch (IBK::Exception & ex) {
			throw IBK::Exception(ex, IBK::FormatString("Invalid quantity format string '%1' in output definition.").arg(od.m_quantity), FUNC_ID);
		}

		// check if quantity is a state quantity
		bool found = false;
		for (const char * const quantityName : STATE_QUANTITIES) {
			if (qn.m_name == quantityName) {
				found = true;
				break;
			}
		}
		if (found) {
			groupMap[od.m_gridName][OFN_States].push_back(od);
			continue;
		}

		// flux quantity?
		found = false;
		for (const char * const quantityName : FLUX_QUANTITIES) {
			if (qn.m_name == quantityName) {
				found = true;
				break;
			}
		}
		if (found) {
			// time integral values?
			if (od.m_timeType == NANDRAD::OutputDefinition::OTT_INTEGRAL)
				groupMap[od.m_gridName][OFN_FluxIntegrals].push_back(od);
			else
				groupMap[od.m_gridName][OFN_Fluxes].push_back(od);
			continue;
		}

		// all others go to misc
		groupMap[od.m_gridName][OFN_Misc].push_back(od);
	}


	// now we have grouped all output definitions into files, and we can create output file objects for
	// all non-empty groups

	// we first create the output files in shared ptr containers, so that memory is properly
	// cleaned up in case of exceptions. At the end of the function, the objects
	// are moved to m_outputFiles before they are copied into NandradModel::m_modelContainer (which
	// owns them).
	std::vector<std::unique_ptr<OutputFile> >					tmpOutputFiles;

	for (auto filegrp : targetFileMap) {
		// automatically named files are skipped
		if (filegrp.first.empty()) continue;

		tmpOutputFiles.push_back( std::unique_ptr<OutputFile>(new OutputFile));
		OutputFile * of = tmpOutputFiles.back().get();

		// set filename
		of->m_filename = filegrp.first;
		// add file extension (tsv or btf see below)
		if (prj.m_outputs.m_binaryFormat.isEnabled())
			of->m_filename += ".btf";
		else
			of->m_filename += ".tsv";

		// now set all output definitions
		of->m_outputDefinitions = filegrp.second;

		// store convenience pointer in file
		of->m_gridRef = of->m_outputDefinitions.back().m_gridRef;

		IBK::IBK_Message(IBK::FormatString("Output file '%1' registered with grid '%2' and %3 output definitions.\n")
						 .arg(of->m_filename).arg(of->m_gridRef->m_name).arg(of->m_outputDefinitions.size()),
						 IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_INFO);
	}

	// now all the auto-generated files
	for (auto filegrp : groupMap) {
		// process all types
		for (unsigned int i=0; i<NUM_OFN; ++i) {
			// skip file types with empty lists
			if (filegrp.second[i].empty()) continue;

			// now create the file with the file name pattern:
			// <filetype>_<gridname>.tsv or
			// <filetype>_<gridname>.btf   (bt = binary table format)
			//
			// also: special handling: if only a single output grid is used, i.e. size of the groupMap is 1, skip
			// the gridname suffix

			std::string filename = NANDRAD_MODEL::KeywordList::Keyword("OutputHandler::OutputFileNames", (int)i);
			// add suffix
			if (groupMap.size() != 1) {
				filename += "_" + filegrp.first;
			}

			// now we can create the file object
			tmpOutputFiles.push_back( std::unique_ptr<OutputFile>(new OutputFile));
			OutputFile * of = tmpOutputFiles.back().get();

			// set filename
			of->m_filename = filename;
			// add file extension (tsv or btf see below)
			if (m_binaryFiles)
				of->m_filename += ".btf";
			else
				of->m_filename += ".tsv";

			// now set all output definitions
			of->m_outputDefinitions = filegrp.second[i];

			// store convenience pointer in file
			of->m_gridRef = of->m_outputDefinitions.back().m_gridRef;

			IBK::IBK_Message(IBK::FormatString("Output file '%1' registered with grid '%2' and %3 output definitions.\n")
							 .arg(of->m_filename).arg(of->m_gridRef->m_name).arg(of->m_outputDefinitions.size()),
							 IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_INFO);
		}
	}


	// *** create input references

	// process all output files
	for (std::unique_ptr<OutputFile> & of : tmpOutputFiles) {
		// request each output file to generate input references for its output variables
		of->createInputReferences(); // may throw exception
	}

	// *** transfer pointer

	for (std::unique_ptr<OutputFile> & of : tmpOutputFiles) {
		m_outputFiles.push_back(of.release());
	}

}


void OutputHandler::writeOutputs(double t_out, double t_secondsOfYear) {
	FUNCID(OutputHandler::writeOutputs);

	// On first call to writeOutputs() we can finally re-open/create the output files
	// because only now we have the inputReferences resolved and have access to the
	// result quantity descriptions. In setup() we do not yet have this info and
	// therefore cannot create the output files there.
	if (m_outputTimer == nullptr) {

		// *** create/re-open output files

		if (m_restart)
			IBK::IBK_Message("Re-opening output files:\n", IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
		else
			IBK::IBK_Message("Creating output files:\n", IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
		IBK_MSG_INDENT;

		// compose time header and convert output time to output unit
		std::string timeColumnHeader = "Time [" + m_timeUnit.name() + "]";

		for (OutputFile * of : m_outputFiles) {
			try {
				of->createFile(m_restart, m_binaryFiles, timeColumnHeader, m_outputPath);
			} catch (IBK::Exception & ex) {
				throw IBK::Exception(ex, IBK::FormatString("Error creating output file '%1'.").arg(of->m_filename), FUNC_ID);
			}
		}
		// Note: in createFile() also all integral quantities are initialized

		// *** initialize output flush timer

		// now create timer (becomes owned by us)
		m_outputTimer = new IBK::StopWatch; // timer starts automatically

	}

	// convert to output time unit
	double t_timeOfYear = t_secondsOfYear;
	IBK::UnitList::instance().convert(IBK::Unit(IBK_UNIT_ID_SECONDS), m_timeUnit, t_timeOfYear);

	// now pass on the call to each file to cache current results
	unsigned int storedBytes = 0;
	for (OutputFile * of : m_outputFiles) {
		// check if output grid is active and only write outputs if this is the case
		if (of->m_gridRef->isActive(t_secondsOfYear))
			of->cacheOutputs(t_out, t_timeOfYear);
		storedBytes += of->cacheSize();
	}

	// flush cache to file once cached limit
	if (m_outputTimer->difference()/1000.0 > m_realTimeOutputDelay ||
		storedBytes > m_outputCacheLimit)
	{
		if (storedBytes > m_outputCacheLimit)
			IBK::IBK_Message("Flushing output cache (cache limit exceeded).\n", IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_DETAILED);
		else
			IBK::IBK_Message( IBK::FormatString("Flushing output cache (time delay reached, cache size = %1 Mb).\n").arg(storedBytes/1024./1024.), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_DETAILED);
		flushCache();
		// restart timer
		m_outputTimer->start();
	}
}


void OutputHandler::flushCache() {
	for (OutputFile * of : m_outputFiles)
		of->flushCache();
}


} // namespace NANDRAD_MODEL
