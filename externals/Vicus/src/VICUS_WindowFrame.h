#ifndef VICUS_WindowFrameH
#define VICUS_WindowFrameH

#include "VICUS_CodeGenMacros.h"
#include "VICUS_Constants.h"
#include "VICUS_AbstractDBElement.h"
#include "VICUS_KeywordList.h"

#include <QString>
#include <QColor>

namespace VICUS {

/*! Window frame is a child of Window, but not a stand-alone
	database element.
*/
class WindowFrame {
public:

	/*! Basic parameters. */
	enum para_t {
		P_Thickness,				// Keyword: Thickness			[m]		'Frame material thickness.'
		NUM_P
	};


	// *** PUBLIC MEMBER FUNCTIONS ***

	VICUS_READWRITE
	VICUS_COMPARE_WITH_ID

	/// TODO Dirk add a function to get the circumference
	/// need polygon ...

	/*! Comparison operator. */
	bool operator!=(const WindowFrame &other)const {
		if( m_id != other.m_id ||
				m_dataSource != other.m_dataSource ||
				m_idMaterial != other.m_idMaterial ||
				m_notes != other.m_notes)
			return true;

		for(unsigned int i=0; i<NUM_P; ++i)
			if(m_para[i] != other.m_para[i])
				return true;

		return false;
	}

	bool isValid() const {
		if(m_id == INVALID_ID ||
				m_idMaterial == INVALID_ID)
			return false;

		m_para[P_Thickness].checkedValue(KeywordList::Keyword("WindowFrame::para_t", P_Thickness),
										 "m", "m", 0, false, 1, true, nullptr);

		return true;
	}

	/*! Comparison operator. */
	bool operator==(const WindowFrame &other) const { return !(*this != other);}

	// *** PUBLIC MEMBER VARIABLES ***

	/*! Unique ID of frame. */
	unsigned int					m_id = INVALID_ID;					// XML:A:required

	/*! Display name of frame. */
	QString							m_displayName;						// XML:A

	/*! Notes. */
	QString							m_notes;							// XML:E

	/*! Data source. */
	QString							m_dataSource;						// XML:E

	/*! Material ID. */
	unsigned int					m_idMaterial;						// XML:A

	/*! Parameters. */
	IBK::Parameter					m_para[NUM_P];						// XML:E


};

} // namespace VICUS


#endif // VICUS_WindowFrameH
