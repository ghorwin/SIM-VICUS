#ifndef NM_OutputFileH
#define NM_OutputFileH

#include <string>

namespace NANDRAD_MODEL {

/*! Handles writing of a single tsv output file.
	This class implements the model interface and requests input references for
	all output quantities stored in the output file managed by this class.
*/
class OutputFile {
public:


	std::string	m_filename;
};

} // namespace NANDRAD_MODEL


#endif // NM_OutputFileH
