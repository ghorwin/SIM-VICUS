#ifndef VICUS_WindowDividerH
#define VICUS_WindowDividerH

#include "VICUS_CodeGenMacros.h"
#include "VICUS_Constants.h"
#include "VICUS_AbstractDBElement.h"
#include "VICUS_MaterialLayer.h"

#include <QString>
#include <QColor>

namespace VICUS {

/*! Window divider is a child of Window, but not a stand-alone
	database element.
*/
class WindowDivider {
public:

	/*! Basic parameters. */
	enum para_t {
		P_Thickness,				// Keyword: Thickness			[m]		'Divider material thickness.'
		NUM_P
	};


	// *** PUBLIC MEMBER FUNCTIONS ***

	VICUS_READWRITE
	VICUS_COMPARE_WITH_ID

	/*! Comparison operator. */
	bool operator!=(const WindowDivider &other)const {
//		for(unsigned int i=0; i< NUM_P; ++i)
//			if(m_para[i] != other.m_para[i])
//				return true;

		if( m_id != other.m_id ||
				m_idMaterial != other.m_idMaterial||
				m_dataSource != other.m_dataSource ||
				m_displayName != other.m_displayName ||
				m_notes != other.m_notes)
			return true;

		for(unsigned int i=0; i<NUM_P; ++i)
			if(m_para[i] != other.m_para[i])
				return true;

		return false;
	}

	bool isValid(){
		if(m_id == INVALID_ID ||
				m_idMaterial == INVALID_ID)
			return false;

		for(unsigned int i=0; i<NUM_P; ++i)
			if(m_para[i].empty())
				return false;

		return true;
	}

	/*! Comparison operator. */
	bool operator==(const WindowDivider &other) const { return !(*this != other);}

	// *** PUBLIC MEMBER VARIABLES ***

	/*! Unique ID of divider. */
	unsigned int					m_id = INVALID_ID;			// XML:A:required

	/*! Display name of divider. */
	QString							m_displayName;				// XML:A

	/*! Notes. */
	QString							m_notes;					// XML:E

	/*! Data source. */
	QString							m_dataSource;				// XML:E

	/*! Material ID. */
	unsigned int					m_idMaterial;				// XML:A

	/*! Parameters. */
	IBK::Parameter					m_para[NUM_P];				// XML:E


};

} // namespace VICUS


#endif // VICUS_WindowDividerH
