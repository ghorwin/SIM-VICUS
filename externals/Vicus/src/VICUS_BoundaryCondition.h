#ifndef VICUS_BoundaryConditionH
#define VICUS_BoundaryConditionH

#include <QString>
#include <QColor>
#include <QCoreApplication>

#include <vector>

#include <IBK_Parameter.h>
#include <IBK_MultiLanguageString.h>

#include <NANDRAD_InterfaceHeatConduction.h>
#include <NANDRAD_InterfaceLongWaveEmission.h>
#include <NANDRAD_InterfaceSolarAbsorption.h>
#include <NANDRAD_InterfaceVaporDiffusion.h>
#include <NANDRAD_InterfaceAirFlow.h>

#include "VICUS_CodeGenMacros.h"
#include "VICUS_Constants.h"
#include "VICUS_AbstractDBElement.h"


namespace VICUS {

/*! This is a template for boundary conditions, to be referenced by components. */
class BoundaryCondition : public AbstractDBElement {
	Q_DECLARE_TR_FUNCTIONS(BoundaryCondition)
public:

	// *** PUBLIC MEMBER FUNCTIONS ***

	VICUS_READWRITE
	VICUS_COMPARE_WITH_ID


	/*! Checks if all parameters are valid. */
	bool isValid() const;

	/*! Generates a short html-description of the defined parameters. */
	QString htmlDescription() const;

	// *** PUBLIC MEMBER VARIABLES ***

	/*! Unique ID of boundary condition. */
	unsigned int							m_id = INVALID_ID;					// XML:A:required

	/*! Display name of boundary condition. */
	IBK::MultiLanguageString				m_displayName;						// XML:A

	/*! Identification color. */
	QColor									m_color;							// XML:E

	// Boundary condition parameters
	/*! Model for convective heat transfer. */
	NANDRAD::InterfaceHeatConduction		m_heatConduction;					// XML:E
	/*! Model for solar absorption. */
	NANDRAD::InterfaceSolarAbsorption		m_solarAbsorption;					// XML:E
	/*! Model for long wave emissivity. */
	NANDRAD::InterfaceLongWaveEmission		m_longWaveEmission;					// XML:E
	/*! Model for vapor diffusion (requires enabled moisture balance). */
	NANDRAD::InterfaceVaporDiffusion		m_vaporDiffusion;					// XML:E
	/*! Model for air flow calculation. */
	NANDRAD::InterfaceAirFlow				m_airFlow;							// XML:E

};

} // namespace VICUS


#endif // VICUS_BoundaryConditionH
