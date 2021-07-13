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

#ifndef VICUS_NetworkComponentH
#define VICUS_NetworkComponentH

#include <IBK_MultiLanguageString.h>
#include <IBK_Parameter.h>

#include <QColor>

#include "VICUS_CodeGenMacros.h"
#include "VICUS_Constants.h"
#include "VICUS_AbstractDBElement.h"
#include "VICUS_Database.h"
#include "VICUS_Schedule.h"

#include <NANDRAD_HydraulicNetworkComponent.h>

namespace VICUS {

/*! Data model object for network components, basically the same as NANDRAD::HydraulicNetworkComponent with
	data members needed in the user interface.

	We cannot use inheritence here, but since both objects are very similar it should be easy to copy/paste back
	and forth between objects.
*/
class NetworkComponent : public AbstractDBElement {
public:

	/*! The various types (equations) of the hydraulic component. */
	enum ModelType {
		MT_SimplePipe,						// Keyword: SimplePipe						'Pipe with a single fluid volume and with heat exchange'
		MT_DynamicPipe,						// Keyword: DynamicPipe						'Pipe with a discretized fluid volume and heat exchange'
		MT_ConstantPressurePump,			// Keyword: ConstantPressurePump			'Pump with constant/externally defined pressure'
		MT_ConstantMassFluxPump,			// Keyword: ConstantMassFluxPump			'Pump with constant/externally defined mass flux'
		MT_ControlledPump,					// Keyword: ControlledPump					'Pump with pressure head controlled based on flow controller'
		MT_HeatExchanger,					// Keyword: HeatExchanger					'Simple heat exchanger with given heat flux'
		MT_HeatPumpIdealCarnotSourceSide,	// Keyword: HeatPumpIdealCarnotSourceSide	'Heat pump with variable heating power based on carnot efficiency, installed at source side (collector cycle)'
		MT_HeatPumpIdealCarnotSupplySide,	// Keyword: HeatPumpIdealCarnotSupplySide	'Heat pump with variable heating power based on carnot efficiency, installed at supply side'
		MT_HeatPumpRealSourceSide,			// Keyword: HeatPumpRealSourceSide			'On-off-type heat pump based on polynoms, installed at source side'
		MT_ControlledValve,					// Keyword: ControlledValve					'Valve with associated control model'
		MT_IdealHeaterCooler,				// Keyword: IdealHeaterCooler				'Ideal heat exchange model that provides a defined supply temperature to the network and calculates the heat loss/gain'
		NUM_MT
	};

	/*! Parameters for the component. */
	enum para_t {
		P_HydraulicDiameter,					// Keyword: HydraulicDiameter					[mm]	'Only used for pressure loss calculation with PressureLossCoefficient (NOT for pipes)'
		P_PressureLossCoefficient,				// Keyword: PressureLossCoefficient				[---]	'Pressure loss coefficient for the component (zeta-value)'
		P_PressureHead,							// Keyword: PressureHead						[Pa]	'Pressure head for a pump'
		P_MassFlux,								// Keyword: MassFlux							[kg/s]	'Pump predefined mass flux'
		P_PumpEfficiency,						// Keyword: PumpEfficiency						[---]	'Pump efficiency'
		P_FractionOfMotorInefficienciesToFluidStream,	// Keyword: FractionOfMotorInefficienciesToFluidStream	[---]	'Fraction of pump heat loss due to inefficiency that heats up the fluid'
		P_Volume,								// Keyword: Volume								[m3]	'Water or air volume of the component'
		P_PipeMaxDiscretizationWidth,			// Keyword: PipeMaxDiscretizationWidth			[m]		'Maximum width/length of discretized volumes in pipe'
		P_CarnotEfficiency,						// Keyword: CarnotEfficiency					[---]	'Carnot efficiency eta'
		P_MaximumHeatingPower,					// Keyword: MaximumHeatingPower					[W]		'Maximum heating power'
		NUM_P
	};

	// *** PUBLIC MEMBER FUNCTIONS ***

	VICUS_READWRITE
	VICUS_COMP(NetworkComponent)
	VICUS_COMPARE_WITH_ID

	/*! Checks if all parameters are valid. */
	bool isValid(const Database<Schedule> &scheduleDB) const;

	/*! Comparison operator */
	ComparisonResult equal(const AbstractDBElement *other) const override;

	// *** PUBLIC MEMBER VARIABLES added for VICUS ***

	/*! Display name. */
	IBK::MultiLanguageString			m_displayName;										// XML:A

	/*! False color. */
	QColor								m_color;											// XML:A

	/*! Notes. */
	IBK::MultiLanguageString			m_notes;											// XML:E

	/*! Manufacturer. */
	IBK::MultiLanguageString			m_manufacturer;										// XML:E

	/*! Data source. */
	IBK::MultiLanguageString			m_dataSource;										// XML:E

	/*! Schedules for this component */
	std::vector<unsigned int>			m_scheduleIds;										// XML:E


	// *** PUBLIC MEMBER VARIABLES from NANDRAD::HydraulicNetworkComponent (without m_displayName) ***

	/*! Unique ID for this component. */
	unsigned int					m_id			= NANDRAD::INVALID_ID;				// XML:A:required

	/*! Model type. */
	ModelType						m_modelType		= NUM_MT;							// XML:A:required

	/*! Parameters of the flow component. */
	IBK::Parameter					m_para[NUM_P];										// XML:E

	/*! Array parameters of the flow component */
	NANDRAD::DataTable				m_polynomCoefficients;								// XML:E

};

} // namespace VICUS


#endif // VICUS_NetworkComponentH
