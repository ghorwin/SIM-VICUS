/*	The SIM-VICUS data model library.

	Copyright (c) 2020-today, Institut für Bauklimatik, TU Dresden, Germany

	Primary authors:
	  Andreas Nicolai  <andreas.nicolai -[at]- tu-dresden.de>
	  Dirk Weiss  <dirk.weiss -[at]- tu-dresden.de>
	  Stephan Hirth  <stephan.hirth -[at]- tu-dresden.de>
	  Hauke Hirsch  <hauke.hirsch -[at]- tu-dresden.de>

	  ... all the others from the SIM-VICUS team ... :-)

	This library is part of SIM-VICUS (https://github.com/ghorwin/SIM-VICUS)

	This library is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.
*/

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
			m_methodDivider!= otherWin->m_methodDivider||
			m_methodFrame!= otherWin->m_methodFrame ||
			m_frame != otherWin->m_frame ||
			m_divider != otherWin->m_divider)
		return Different;

	for(unsigned int i=0; i<NUM_P; ++i){
		if(m_para[i] != otherWin->m_para[i])
			return Different;
	}

	//check meta data

	if (m_displayName != otherWin->m_displayName ||
			m_dataSource != otherWin->m_dataSource ||
			m_notes != otherWin->m_notes ||
			m_color != otherWin->m_color)
		return OnlyMetaDataDiffers;

	return Equal;
}


bool Window::isValid() const {
	if(m_id == INVALID_ID ||
			m_idGlazingSystem == INVALID_ID)
		return false;


	switch (m_methodFrame) {
		case VICUS::Window::M_None:

		break;
		case VICUS::Window::M_Fraction:{
			try {
				m_para[P_FrameFraction].checkedValue(KeywordList::Keyword("Window::para_t", P_FrameFraction),
													 "---", "---", 0, true, 1, true, nullptr);
				if(!m_frame.isValid())
					return false;
			}  catch (...) {
				return false;
			}

		}
		break;
		case VICUS::Window::M_ConstantWidth:{
			try {
				m_para[P_FrameWidth].checkedValue(KeywordList::Keyword("Window::para_t", P_FrameWidth),
												  "m", "m", 0, false, 2, true, nullptr);
				if(!m_frame.isValid())
					return false;
			}  catch (...) {
				return false;
			}

		}
		break;
		case VICUS::Window::NUM_M:
		return false;
	}
	switch (m_methodDivider) {
		case VICUS::Window::M_None:

		break;
		case VICUS::Window::M_Fraction:{
			try {
				m_para[P_DividerFraction].checkedValue(KeywordList::Keyword("Window::para_t", P_DividerFraction),
													 "---", "---", 0, true, 1, true, nullptr);
				if(!m_divider.isValid())
					return false;
			}  catch (...) {
				return false;
			}
		}
		break;
		case VICUS::Window::M_ConstantWidth:{
			try {
				m_para[P_DividerWidth].checkedValue(KeywordList::Keyword("Window::para_t", P_DividerWidth),
												  "m", "m", 0, false, 2, true, nullptr);
				if(!m_divider.isValid())
					return false;
			}  catch (...) {
				return false;
			}
		}
		break;
		case VICUS::Window::NUM_M:
		return false;

	}



	return true;
}


bool Window::calculateUValue(double & UValue, const VICUS::Database<Material> & materials,
							 const VICUS::Database<WindowGlazingSystem> & glazingSystem,
							 double ri, double re) const
{
	UValue = -1;
	return true;
}

} // namespace VICUS
