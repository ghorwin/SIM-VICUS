#include "VICUS_Window.h"
#include "VICUS_KeywordList.h"


namespace VICUS {

AbstractDBElement::ComparisonResult Window::equal(const AbstractDBElement *other) const {
	const Window * otherWin = dynamic_cast<const Window*>(other);
	if (otherWin == nullptr)
		return Different;

	//first check critical data

	//check parameters
	if (m_idGlazingSystem != otherWin->m_idGlazingSystem ||
			m_frame != otherWin->m_frame ||
			m_divider != otherWin->m_divider)
		return Different;

	//check meta data

	if (m_displayName != otherWin->m_displayName ||
			m_dataSource != otherWin->m_dataSource ||
			m_notes != otherWin->m_notes ||
			m_color != otherWin->m_color)
		return OnlyMetaDataDiffers;

	return Equal;
}


bool Window::isValid() const {
	// TODO : Dirk
}

} // namespace VICUS
