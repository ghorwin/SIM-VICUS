#include "VICUS_NetworkBuriedPipeProperties.h"

#include "VICUS_KeywordList.h"


namespace VICUS {


NetworkBuriedPipeProperties::NetworkBuriedPipeProperties()
{
	// set default values
	KeywordList::setParameter(m_para, "NetworkBuriedPipeProperties::para_t", P_PipeDepth, 0.8);
	KeywordList::setParameter(m_para, "NetworkBuriedPipeProperties::para_t", P_PipeSpacing, 0.5);
}


} // namespace VICUS


