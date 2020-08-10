#ifndef NM_OutputHandlerH
#define NM_OutputHandlerH

#include <vector>

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


	/*! Vector with output file objects. */
	std::vector<OutputFile*>								m_outputFiles;


};


} // namespace NANDRAD_MODEL

#endif // NM_OutputHandlerH
