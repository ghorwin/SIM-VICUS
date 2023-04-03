#ifndef VICUS_StructuralUnitH
#define VICUS_StructuralUnitH


#include "VICUS_CodeGenMacros.h"
#include "VICUS_Constants.h"
#include "VICUS_Object.h"

#include <QColor>


namespace VICUS {

/*! A structural unit combines several VICUS:Rooms for
	acoustic calculation and several post-processings. */
class StructuralUnit : public VICUS::Object {
public:
	VICUS_READWRITE
	VICUS_COMPARE_WITH_ID

	StructuralUnit() {}

	/*! Type-info string. */
	const char * typeinfo() const override { return "StructuralUnit"; }

	// *** PUBLIC MEMBER VARIABLES ***

	//:inherited	unsigned int					m_id = INVALID_ID;		// XML:A:required
	//:inherited	QString							m_displayName;			// XML:A

	/*! Stores the room ids of this structural unit. */
	std::set<unsigned int>							m_roomIds;				// XML:E

	/*! Color of structural unit. */
	QColor											m_color;				// XML:E

};
}
#endif // VICUS_StructuralUnitH
