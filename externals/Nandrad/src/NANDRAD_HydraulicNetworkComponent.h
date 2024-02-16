/*	The NANDRAD data model library.

	Copyright (c) 2012-today, Institut für Bauklimatik, TU Dresden, Germany

	Primary authors:
	  Andreas Nicolai  <andreas.nicolai -[at]- tu-dresden.de>
	  Anne Paepcke     <anne.paepcke -[at]- tu-dresden.de>

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

#ifndef NANDRAD_HydraulicNetworkComponentH
#define NANDRAD_HydraulicNetworkComponentH

#include <IBK_Parameter.h>

#include "NANDRAD_CodeGenMacros.h"
#include "NANDRAD_Constants.h"
#include "NANDRAD_LinearSplineParameter.h"
#include "NANDRAD_DataTable.h"


namespace NANDRAD {

/*! Parameters for a hydraulic component for the network. */
class HydraulicNetworkComponent {
public:

	/*! The various types (equations) of the hydraulic component. */
	enum ModelType {
		MT_SimplePipe,						// Keyword: SimplePipe						'Pipe with a single fluid volume'
		MT_DynamicPipe,						// Keyword: DynamicPipe						'Pipe with discretized fluid volume'
		MT_ConstantPressurePump,			// Keyword: ConstantPressurePump			'Pump with defined pressure head'
		MT_ConstantMassFluxPump,			// Keyword: ConstantMassFluxPump			'Pump with defined mass flux'
		MT_ControlledPump,					// Keyword: ControlledPump					'Pump with controlled pressure head'
		MT_VariablePressurePump,			// Keyword: VariablePressurePump			'Pump with linear pressure head curve'
		MT_HeatExchanger,					// Keyword: HeatExchanger					'Simple heat exchanger'
		MT_HeatPumpVariableIdealCarnotSourceSide,	// Keyword: HeatPumpVariableIdealCarnotSourceSide	'Heat pump installed at source side, based on carnot efficiency'
		MT_HeatPumpVariableIdealCarnotSupplySide,	// Keyword: HeatPumpVariableIdealCarnotSupplySide	'Heat pump installed at supply side, based on carnot efficiency'
		MT_HeatPumpVariableSourceSide,		// Keyword: HeatPumpVariableSourceSide		'Heat pump installed at source side, based on polynom for COP'
		MT_HeatPumpOnOffSourceSide,			// Keyword: HeatPumpOnOffSourceSide			'Heat pump (on-off) at source side, based on polynoms for heating power and el. power'
//		xMT_HeatPumpOnOffSourceSideWithBuffer,	// xKeyword: HeatPumpOnOffSourceSideWithBuffer			'On-off-type heat pump based on polynoms for heating power and el. power, installed at source side'
		MT_ControlledValve,					// Keyword: ControlledValve					'Controlled valve'
		MT_ConstantPressureLossValve,		// Keyword: ConstantPressureLossValve		'Valve with constant pressure loss'
		MT_IdealHeaterCooler,				// Keyword: IdealHeaterCooler				'Ideal heater with predefined supply temperature'
		MT_PressureLossElement,				// Keyword: PressureLossElement				'Adiabatic element with pressure loss defined by zeta-value'
		NUM_MT
	};

	/*! Parameters for the component. */
	enum para_t {
		P_HydraulicDiameter,					// Keyword: HydraulicDiameter					[mm]	'Only used for pressure loss calculation with PressureLossCoefficient (NOT for pipes)'
		P_PressureLossCoefficient,				// Keyword: PressureLossCoefficient				[---]	'Pressure loss coefficient for the component (zeta-value)'
		P_PressureHead,							// Keyword: PressureHead						[Bar]	'Pump predefined pressure head'
		P_MassFlux,								// Keyword: MassFlux							[kg/s]	'Pump predefined mass flux'
		P_PumpMaximumEfficiency,				// Keyword: PumpMaximumEfficiency				[---]	'Pump maximum efficiency'
		P_FractionOfMotorInefficienciesToFluidStream,	// Keyword: FractionOfMotorInefficienciesToFluidStream	[---]	'Fraction of pump heat loss due to inefficiency that heats up the fluid'
		P_MaximumPressureHead,					// Keyword: MaximumPressureHead					[Bar]	'Pump maximum pressure head at point of minimal mass flow of pump'
		P_PumpMaximumElectricalPower,			// Keyword: PumpMaximumElectricalPower			[W]		'Pump maximum electrical power at point of optimal operation'
		P_DesignPressureHead,					// Keyword: DesignPressureHead					[Bar]	'Design pressure head of VariablePressureHeadPump'
		P_DesignMassFlux,						// Keyword: DesignMassFlux						[kg/s]	'Design mass flux of VariablePressureHeadPump'
		P_PressureHeadReductionFactor,			// Keyword: PressureHeadReductionFactor			[---]	'Factor to reduce pressure head of VariablePressureHeadPump'
		P_Volume,								// Keyword: Volume								[m3]	'Water or air volume of the component'
		P_PipeMaxDiscretizationWidth,			// Keyword: PipeMaxDiscretizationWidth			[m]		'Maximum width/length of discretized volumes in pipe'
		P_CarnotEfficiency,						// Keyword: CarnotEfficiency					[---]	'Carnot efficiency eta'
		P_MaximumHeatingPower,					// Keyword: MaximumHeatingPower					[W]		'Maximum heating power'
		P_MaximumCoolingPower,					// Keyword: MaximumCoolingPower					[W]		'Maximum cooling power'
		P_PressureLoss,							// Keyword: PressureLoss						[Bar]	'Pressure loss for valve'
		P_MinimumOutletTemperature,				// Keyword: MinimumOutletTemperature			[C]		'Minimum outlet temperature of heat exchanger, used for clipping of heat extraction'
		P_HeatingPowerB0W35,					// Keyword: HeatingPowerB0W35					[W]		'Heat pump heating power at B0/W35
		P_HeatingBufferSupplyTemperature,		// Keyword: HeatingBufferSupplyTemperature		[C]		'Heat pump supply temperature for heating buffer storage'
		P_HeatingBufferReturnTemperature,		// Keyword: HeatingBufferReturnTemperature		[C]		'Heat pump return temperature for heating buffer storage'
		P_DHWBufferSupplyTemperature,			// Keyword: DHWBufferSupplyTemperature			[C]		'Heat pump supply temperature for DHW buffer storage'
		P_DHWBufferReturnTemperature,			// Keyword: DHWBufferReturnTemperature			[C]		'Heat pump return temperature for DHW buffer storage'
		P_HeatingBufferVolume,					// Keyword: HeatingBufferVolume					[m3]	'Heat pump heating buffer storage volume'
		P_DHWBufferVolume,						// Keyword: DHWBufferVolume						[m3]	'Heat pump DHW buffer storage volume'		'
		NUM_P
	};


	// *** PUBLIC MEMBER FUNCTIONS ***

	NANDRAD_READWRITE
	NANDRAD_COMPARE_WITH_ID
	NANDRAD_COMP(HydraulicNetworkComponent)

	/*! Compares two component definitions by parameters only, without comparing ID. */
	bool sameParametersAs(const HydraulicNetworkComponent & other) const;

	/*! Checks for valid and required parameters (value ranges).
		\param networkModelType Type of network calculation model (HydraulicNetwork::ModelType).
	*/
	void checkParameters(int networkModelType);

	// *** PUBLIC MEMBER VARIABLES ***

	/*! Unique ID for this component. */
	unsigned int					m_id			= NANDRAD::INVALID_ID;				// XML:A:required

	/*! Display name. */
	std::string						m_displayName;										// XML:A

	/*! Model type. */
	ModelType						m_modelType		= NUM_MT;							// XML:A:required

	/*! Parameters of the flow component. */
	IBK::Parameter					m_para[NUM_P];										// XML:E

	/*! Array parameters of the flow component */
	DataTable						m_polynomCoefficients;								// XML:E

	// *** STATIC FUNCTIONS ***

	/*! Needed both in user interface and for valid parameter checking in solver.
		\param networkModelType Identifies network model (HydraulicNetwork::ModelType).
	*/
	static std::vector<unsigned int> requiredParameter(const ModelType modelType, int networkModelType);

	static std::vector<std::string> requiredScheduleNames(const ModelType modelType);

	/*! Helper function that implements specific rules for testing a single parameter.
		This is useful if the same parameter is used by several models and we want to avoid implementing
		the same checking rule multiple times.
		Is used in Nandrad as well as in the graphical user interface.
	*/
	static void checkModelParameter(const IBK::Parameter &para, const unsigned int numPara);

};

} // namespace NANDRAD

#endif // NANDRAD_HydraulicNetworkComponentH
