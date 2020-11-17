#ifndef VICUS_ViewSettingsH
#define VICUS_ViewSettingsH

#include <IBK_Flag.h>

#include <QColor>

#include "VICUS_CodeGenMacros.h"

namespace VICUS {

class ViewSettings {
	VICUS_READWRITE_PRIVATE
public:

	enum Flags {
		F_GridVisible,		// Keyword: GridVisible			'Grid is visible'
		NUM_F
	};

	// *** PUBLIC MEMBER FUNCTIONS ***

	VICUS_READWRITE_IFNOTEMPTY(ViewSettings)
	VICUS_COMP(ViewSettings)

	// *** PUBLIC MEMBER VARIABLES ***

	/*! Holds grid spacing in [m] */
	double								m_gridSpacing	= 10;			// XML:E
	/*! Holds width dimension of grid in [m] */
	double								m_gridWidth		= 100;			// XML:E

	QString								m_gridColor		= "#7f7fb2";	// XML:E

	IBK::Flag							m_flags[NUM_F];					// XML:E
};


inline bool ViewSettings::operator!=(const ViewSettings & other) const {
	if (m_gridSpacing != other.m_gridSpacing) return true;
	if (m_gridWidth != other.m_gridWidth) return true;
	for (unsigned int i=0; i<NUM_F; ++i)
		if (m_flags[i] != other.m_flags[i]) return true;

	return false;
}

} // namespace VICUS


#endif // VICUS_ViewSettingsH
