/*	The SIM-VICUS data model library.

	Copyright (c) 2020-today, Institut für Bauklimatik, TU Dresden, Germany

	Primary authors:
	  Andreas Nicolai  <andreas.nicolai -[at]- tu-dresden.de>
	  ... all the others from the SIM-VICUS team ... :-)

	This library is part of SIM-VICUS (https://github.com/ghorwin/SIM-VICUS)

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 3 of the License, or (at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	Lesser General Public License for more details.
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
		MT_SimplePipe,						// Keyword: SimplePipe					'Pipe with a single fluid volume and with heat exchange'
		MT_DynamicPipe,						// Keyword: DynamicPipe					'Pipe with a discretized fluid volume and heat exchange'
		MT_ConstantPressurePump,			// Keyword: ConstantPressurePump		'Pump with constant pressure'
		MT_HeatExchanger,					// Keyword: HeatExchanger				'Simple heat exchanger with given heat flux'
		MT_HeatPumpIdealCarnot,				// Keyword: HeatPumpIdealCarnot			'Heat pump with variable heating power based on carnot efficiency'
		MT_HeatPumpReal,					// Keyword: HeatPumpReal				'On-off-type heat pump with based on manufacturer data sheet'

		// models below not supported yet

//		MT_GasBoiler,						// xKeyword: GasBoiler					'Gas boiler'
//		MT_ControlValve,					// xKeyword: ControlValve				'Control valve'
//		MT_WaterStorage,					// xKeyword: WaterStorage				'Water storage'
//		MT_ComponentConditionSystem,		// xKeyword: ComponentConditionSystem	'Component conditioning system is a system for heating or cooling of components'
//		MT_Radiator,						// xKeyword: Radiator					'Radiator'
//		MT_Mixer,							// xKeyword: Mixer						'Mixer component'
//		MT_FMU,								// xKeyword: FMU						'Flow characteristics provided by FMU'
		NUM_MT
	};

	/*! Parameters for the component. */
	enum para_t {
		P_HydraulicDiameter,					// Keyword: HydraulicDiameter					[mm]	'Only used for pressure loss calculation with PressureLossCoefficient (NOT for pipes)'
		P_PressureLossCoefficient,				// Keyword: PressureLossCoefficient				[-]		'Pressure loss coefficient for the component (zeta-value)'
		P_PressureHead,							// Keyword: PressureHead						[Pa]	'Pressure head form a pump'
		P_PumpEfficiency,						// Keyword: PumpEfficiency						[---]	'Pump efficiency'
		P_Volume,								// Keyword: Volume								[m3]	'Water or air volume of the component'
		P_PipeMaxDiscretizationWidth,			// Keyword: PipeMaxDiscretizationWidth			[m]		'Maximum width of discretized volumes in pipe'
		P_CarnotEfficiency,						// Keyword: CarnotEfficiency					[---]	'Carnot efficiency eta'
		P_MaximumHeatingPower,					// Keyword: MaximumHeatingPower					[W]		'Maximum heating power'
		P_HeatPumpNominalTemperatureDifference,	// Keyword: HeatPumpNominalTemperatureDifference	[K]		'Nominal temperature difference at condenser or evaporator'
		NUM_P
	};


	// *** PUBLIC MEMBER FUNCTIONS ***

	VICUS_READWRITE
	VICUS_COMPARE_WITH_ID
	VICUS_COMP(NetworkComponent)

	/*! Checks if all parameters are valid. */
	bool isValid() const;

	/*! Comparison operator */
	ComparisonResult equal(const AbstractDBElement *other) const;

	// *** PUBLIC MEMBER VARIABLES ***

	/*! Unique ID for this component. */
	unsigned int					m_id				= VICUS::INVALID_ID;			// XML:A:required

	/*! Model type. */
	ModelType						m_modelType			= MT_SimplePipe;				// XML:A:required

	/*! Parameters of the flow component. */
	IBK::Parameter					m_para[NUM_P];										// XML:E

	/*! Display name. */
	IBK::MultiLanguageString		m_displayName;										// XML:A

	/*! False color. */
	QColor							m_color;											// XML:A

	/*! Notes. */
	IBK::MultiLanguageString		m_notes;											// XML:E

	/*! Manufacturer. */
	IBK::MultiLanguageString		m_manufacturer;										// XML:E

	/*! Data source. */
	IBK::MultiLanguageString		m_dataSource;										// XML:E
};

} // namespace VICUS


#endif // VICUS_NetworkComponentH
