#ifndef SupplySystem_H
#define SupplySystem_H

#include <QString>
#include <QCoreApplication>

#include <vector>

#include <IBK_Parameter.h>
#include <IBK_MultiLanguageString.h>

#include "VICUS_AbstractDBElement.h"
#include "VICUS_Constants.h"
#include "VICUS_CodeGenMacros.h"


namespace VICUS {

/*! Defines the base structure of a generic network:
	A generic network may be generated for surface heating FMU export. We bundle
	all heatings by a hydraulically balanced network and support FMI-coupling
	at a single demand network branch.
*/
class SupplySystem : public AbstractDBElement {
	Q_DECLARE_TR_FUNCTIONS(SupplySystem)
public:

	/*! Supplier supported by the generic network model. */
	enum supplyType_t {
		ST_StandAlone,			// Keyword: StandAlone					'Stand-alone mode with given mass flux and suppply temperature'
		ST_SubNetwork,			// Keyword: SubNetwork					'VIVUS sub network loaded from a database and parametrized by the user'
		ST_DatabaseFMU,			// Keyword: DatabaseFMU					'Supply FMU loaded from a database and parametrized by the user'
		ST_UserDefinedFMU,		// Keyword: UserDefinedFMU				'User defined supply FMU'
		NUM_ST
	};

	/*! Network parameters.
	*/
	enum para_t {
		P_MaximumMassFlux,			// Keyword: MaximumMassFlux			[kg/s]	'Maximum mass flux into the network, needed for pump control'
		P_SupplyTemperature,		// Keyword: SupplyTemperature		[C]		'Constant supply temeprature'
		P_MaximumMassFluxFMU,		// Keyword: MaximumMassFluxFMU		[kg/s]	'Maximum mass flux towards the building.'
		P_HeatingPowerFMU,			// Keyword: HeatingPowerFMU			[W]		'Procuder heating power'
		NUM_P
	};
	/*! FMU parameters.
	*/
	enum supplyPara_t {
		NUM_SP
	};

	// *** PUBLIC MEMBER FUNCTIONS ***
	VICUS_READWRITE_OVERRIDE
	VICUS_COMPARE_WITH_ID

	/*! Checks if the network definition is valid. */
	bool isValid() const;

	/*! Comparison operator */
	ComparisonResult equal(const AbstractDBElement *other) const override;

	// *** PUBLIC MEMBER VARIABLES ***

	//:inherited	unsigned int		m_id = INVALID_ID;			// XML:A:required
	//:inherited	QString				m_displayName;				// XML:A
	//:inherited	bool				m_visible = true;			// XML:A

	/*! Network supply type. */
	supplyType_t			m_supplyType = NUM_ST;				// XML:A:required
	/*! Parameters for stand alone mode. */
	IBK::Parameter			m_para[NUM_P];						// XML:E
	/*! FMU path for Database FMU mode. */
	QString					m_supplyFMUPath;					// XML:E
	/*! Id for database FMU. */
	unsigned int			m_supplyFMUId = VICUS::INVALID_ID;	// XML:E
};



} // Namespace VICUS


#endif // SupplySystem_H
