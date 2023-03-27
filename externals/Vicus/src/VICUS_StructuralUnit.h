#ifndef VICUS_STRUCTURALUNIT_H
#define VICUS_STRUCTURALUNIT_H


#include "VICUS_CodeGenMacros.h"
#include "VICUS_Constants.h"
#include "VICUS_Object.h"


#include <QColor>


namespace VICUS {
class StructuralUnit : public VICUS::Object
{
public:
	VICUS_READWRITE
	VICUS_COMPARE_WITH_ID

	StructuralUnit();

	/*! Type-info string. */
	const char * typeinfo() const override { return "StructuralUnit"; }

	// *** PUBLIC MEMBER VARIABLES ***

	//:inherited	unsigned int					m_id = INVALID_ID;		// XML:A:required
	//:inherited	QString							m_displayName;			// XML:A

	/*! store the room ids of this structural unit*/
	std::set<unsigned int>						m_roomIds;				// XML:E

	//TODO Anton: save color here, since its not a abstractDBelement
	// ! displayname ist auch keine MultiLang String
	QColor											m_color;				// XML:E

};
}
#endif // VICUS_STRUCTURALUNIT_H
