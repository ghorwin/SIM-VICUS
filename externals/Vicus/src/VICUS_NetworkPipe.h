#ifndef VICUS_NetworkPipeH
#define VICUS_NetworkPipeH

#include <string>
#include <vector>

#include "VICUS_CodeGenMacros.h"
#include "VICUS_Constants.h"
#include "VICUS_AbstractDBElement.h"

#include <IBK_MultiLanguageString.h>

#include <QColor>

namespace VICUS {

class NetworkPipe: public AbstractDBElement {
public:

	// *** PUBLIC MEMBER FUNCTIONS ***

	VICUS_READWRITE
	VICUS_COMPARE_WITH_ID

	double diameterInside() const{
		return m_diameterOutside - 2 * m_wallThickness;
	}

	/*! Checks if all parameters are valid. */
	bool isValid() const;

	/*! Comparison operator */
	ComparisonResult equal(const AbstractDBElement *other) const;

	// *** PUBLIC MEMBER VARIABLES ***

	/*! Unique id number. */
	unsigned int						m_id = INVALID_ID;				// XML:A:required
	/*! Display name of fluid. */
	IBK::MultiLanguageString			m_displayName;					// XML:A
	/*! Identification color. */
	QColor								m_color;						// XML:A

	///TODO Hauke bitte die variablen in ein array m_para[...] umbauen

	/*! Outside diameter pipe in [mm]. */
	double								m_diameterOutside;				// XML:A:required
	/*! Wall thickness pipe in [mm]. */
	double								m_wallThickness;				// XML:A:required
	/*! Pipe wall thermal conductivity in [W/mK] */
	double								m_lambdaWall;					// XML:A:required
	/*! Pipe wall roughness in [mm] */
	double								m_roughness;					// XML:A:required
	/*! outisde diameter of insulation in [mm] */
	double								m_insulationThickness = 0.0;	// XML:A
	/*! thermal conductivity of insulation in [W/mK] */
	double								m_lambdaInsulation = 0.0;		// XML:A

};


} // namespace VICUS

#endif // VICUS_NetworkPipeH
