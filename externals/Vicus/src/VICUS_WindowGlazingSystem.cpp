#include "VICUS_WindowGlazingSystem.h"
#include "VICUS_KeywordList.h"


namespace VICUS {

AbstractDBElement::ComparisonResult WindowGlazingSystem::equal(const AbstractDBElement *other) const {
	const WindowGlazingSystem * otherWindow = dynamic_cast<const WindowGlazingSystem*>(other);
	if (otherWindow == nullptr)
		return Different;

	//first check critical data

	//check parameters
	for(unsigned int i=0; i<NUM_P; ++i){
		if(m_para[i] != otherWindow->m_para[i])
			return Different;
	}

	for(unsigned int i=0; i<NUM_SP; ++i){
		if(m_splinePara[i] != otherWindow->m_splinePara[i])
			return Different;
	}

	if(m_layers != otherWindow->m_layers)
		return Different;

	//check meta data

	if(m_displayName != otherWindow->m_displayName ||
			m_color != otherWindow->m_color ||
			m_dataSource != otherWindow->m_dataSource ||
			m_manufacturer != otherWindow->m_manufacturer ||
			m_notes != otherWindow->m_notes)
		return OnlyMetaDataDiffers;

	return Equal;
}

} // namespace VICUS
