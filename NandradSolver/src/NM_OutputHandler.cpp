#include "NM_OutputHandler.h"

#include <algorithm>
#include <array>

#include <IBK_StringUtils.h>
#include <IBK_messages.h>
#include <IBK_Exception.h>

#include <NANDRAD_Project.h>

#include "NM_OutputFile.h"
#include "NM_QuantityName.h"
#include "NM_KeywordList.h"

const char * const STATE_QUANTITIES[] = {
	"Temperature"
};

const char * const FLUX_QUANTITIES[] = {
	"ConvectiveHeatFluxDensity"
};

namespace NANDRAD_MODEL {

OutputHandler::~OutputHandler() {
	for (OutputFile * of : m_outputFiles)
		delete of;
	m_outputFiles.clear();
}


void OutputHandler::init(bool restart, NANDRAD::Project & prj) {
	FUNCID(OutputHandler::init);

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
		if (od.m_objectListRef->m_referenceType == NANDRAD::ModelInputReference::MRT_LOCATION ||
			od.m_objectListRef->m_referenceType == NANDRAD::ModelInputReference::MRT_SENSOR)
		{
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

	for (auto filegrp : targetFileMap) {
		// automatically named files are skipped
		if (filegrp.first.empty()) continue;

		OutputFile * of = new OutputFile;
		// take ownership of the created file
		m_outputFiles.push_back(of);

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

			// now we can create the file
			OutputFile * of = new OutputFile;
			// take ownership of the created file
			m_outputFiles.push_back(of);

			// set filename
			of->m_filename = filename;
			// add file extension (tsv or btf see below)
			if (prj.m_outputs.m_binaryFormat.isEnabled())
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
	for (OutputFile * of : m_outputFiles) {
		// request each output file to generate input references for its output variables
//		of->createInputReferences()

	}
}

} // namespace NANDRAD_MODEL
