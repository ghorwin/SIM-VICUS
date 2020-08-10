#ifndef NM_OutputHandlerH
#define NM_OutputHandlerH

#include <vector>

namespace NANDRAD {
	class Project;
}

namespace NANDRAD_MODEL {

class OutputFile;

/*! Handles all output file initialization and the process of caching and writing
	outputs.

	During simulation, this class is called to store/append outputs whenever an output time point has been
	reached in one of the ouput grids.

	The output handler then notifies all output files to cache new output values (when respective grid is active).
*/
class OutputHandler {
public:

	/*! Different names for auto-generated files based on quantity type. */
	enum OutputFileNames {
		OFN_States,				// Keyword: states
		OFN_Loads,				// Keyword: loads
		OFN_Fluxes,				// Keyword: fluxes
		OFN_FluxIntegrals,		// Keyword: flux_integrals
		OFN_Misc,				// Keyword: misc
		NUM_OFN
	};

	/*! Main initialization function.

		\note
		During init, the OutputDefinitions in the project are modified such that the quick-access pointers
		to grids and object lists are created. The vectors themselves remain unchanged (no invalidation of
		previously obtained pointers).

		\param restart If true, simulation was restarted and output files shall be continued, rather than re-created.
		\param prj The project data structure with all input data.
	*/
	void init(bool restart, NANDRAD::Project & prj);


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
};


} // namespace NANDRAD_MODEL

#endif // NM_OutputHandlerH
