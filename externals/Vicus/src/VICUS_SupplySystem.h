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

#ifndef VICUS_SupplySystemH
#define VICUS_SupplySystemH

#include <QString>
#include <QCoreApplication>

#include <vector>

#include <IBK_Parameter.h>
#include <IBK_MultiLanguageString.h>

#include "VICUS_AbstractDBElement.h"
#include "VICUS_Constants.h"
#include "VICUS_CodeGenMacros.h"
#include "VICUS_Database.h"


namespace VICUS {

class SubNetwork;
class NetworkComponent;
class NetworkController;
class Schedule;

/*! Defines the base structure of a generic network:
	A generic network may be generated for surface heating FMU export. We bundle
	all heatings by a hydraulically balanced network and support FMI-coupling
	at a single demand network branch.
*/
class SupplySystem : public AbstractDBElement {
	Q_DECLARE_TR_FUNCTIONS(SupplySystem)
public:

	/*! Supplier supported by the generic network model. */
	enum SupplyType {
		ST_StandAlone,			// Keyword: StandAlone					'Stand-alone mode with given mass flux and suppply temperature'
		ST_SubNetwork,			// Keyword: SubNetwork					'User defined sub network'
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


	// *** PUBLIC MEMBER FUNCTIONS ***

	void readXML(const TiXmlElement * element) override;
	TiXmlElement * writeXML(TiXmlElement * parent) const override;

	VICUS_COMPARE_WITH_ID

	/*! Checks if the network definition is valid. */
	bool isValid(const Database<SubNetwork> &subNetworkDB, const Database<VICUS::NetworkComponent> & compDB, const Database<VICUS::NetworkController> & ctrlDB, const Database<VICUS::Schedule> & scheduleDB) const;

	/*! Comparison operator */
	ComparisonResult equal(const AbstractDBElement *other) const override;

	// *** PUBLIC MEMBER VARIABLES ***

	//:inherited	unsigned int					m_id = INVALID_ID;		// XML:A:required
	//:inherited	IBK::MultiLanguageString		m_displayName;			// XML:A
	//:inherited	QColor							m_color;				// XML:A

	/*! Network supply type. */
	SupplyType				m_supplyType = ST_StandAlone;		// XML:A:required
	/*! Parameters for stand alone mode. */
	IBK::Parameter			m_para[NUM_P];						// XML:E
	/*! FMU path for Database FMU mode. */
	QString					m_supplyFMUPath;					// XML:E
	/*! Id for database FMU. */
	IDType					m_idSupplyFMU = VICUS::INVALID_ID;	// XML:E
	/*! Id of vicus sub network */
	IDType					m_idSubNetwork = VICUS::INVALID_ID;	// XML:E

	// *** runtime variables ***
	const mutable	VICUS::SubNetwork		* m_subNetwork = nullptr;
};



} // Namespace VICUS


#endif // VICUS_SupplySystemH
