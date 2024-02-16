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
#include <IBK_IntPara.h>

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

	NetworkComponent() = default;

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
		// from NANDRAD::HydraulicNetworkComponent
		P_HydraulicDiameter,					// Keyword: HydraulicDiameter					[mm]	'Only used for pressure loss calculation with PressureLossCoefficient (NOT for pipes)'
		P_PressureLossCoefficient,				// Keyword: PressureLossCoefficient				[---]	'Pressure loss coefficient for the component (zeta-value)'
		P_PressureHead,							// Keyword: PressureHead						[Bar]	'Pump predefined pressure head'
		P_MassFlux,								// Keyword: MassFlux							[kg/s]	'Pump predefined mass flux'
		P_PumpMaximumEfficiency,				// Keyword: PumpMaximumEfficiency				[---]	'Pump efficiency in optimal operation point'
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
		// additional parameters
		P_PipeLength,							// Keyword: PipeLength							[m]		'Length of pipe'
		NUM_P
	};

	/*! Whole number parameters. */
	enum intPara_t {
		IP_NumberParallelPipes,					// Keyword: NumberParallelPipes				[---]	'Number of parallel pipes in ground heat exchanger'
		IP_NumberParallelElements,				// Keyword: NumberParallelElements			[---]	'Number of parallel elements'
		NUM_IP
	};

	// *** PUBLIC MEMBER FUNCTIONS ***

	VICUS_READWRITE_OVERRIDE
	VICUS_COMP(NetworkComponent)
	VICUS_COMPARE_WITH_ID

	/*! Checks if all parameters are valid. */
	bool isValid(const Database<Schedule> &scheduleDB) const;

	/*! Comparison operator */
	ComparisonResult equal(const AbstractDBElement *other) const override;

	/*! returns the NANDRAD::HydraulicNetworkComponent parameters which may deviates from the VICUS one as we use the VICUS model
	 * for GUI and preprocessing */
	void nandradNetworkComponentParameter(IBK::Parameter *para) const;


	// *** Static Functions

	/*! returns the NANDRAD::HydraulicNetworkComponent ModelType which may deviates from the VICUS one as we use the VICUS model
	 * for GUI and preprocessing */
	static NANDRAD::HydraulicNetworkComponent::ModelType nandradNetworkComponentModelType(ModelType modelType);

	/*! returns additional required parameters, not included in NANDRAD::HydraulicNetworkComponent */
	static std::vector<unsigned int> additionalRequiredParameter(const ModelType modelType);

	/*! returns optional parameters for the given model type */
	static std::vector<unsigned int> optionalParameter(const ModelType modelType);

	/*! returns additional required parameters */
	static std::vector<unsigned int> requiredIntParameter(const ModelType modelType);

	/*! checks additional required parameters, not included in NANDRAD::HydraulicNetworkComponent */
	static void checkAdditionalParameter(const IBK::Parameter &para, const unsigned int numPara);

	/*! checks required integer parameters */
	static void checkIntParameter(const IBK::IntPara &para, const unsigned int numPara);

	static bool hasPipeProperties(const ModelType modelType);

	// *** PUBLIC MEMBER VARIABLES added for VICUS ***

	//:inherited	unsigned int					m_id = INVALID_ID;					// XML:A:required
	//:inherited	IBK::MultiLanguageString		m_displayName;						// XML:A
	//:inherited	QColor							m_color;							// XML:A

	/*! Notes. */
	IBK::MultiLanguageString			m_notes;										// XML:E

	/*! Manufacturer. */
	IBK::MultiLanguageString			m_manufacturer;									// XML:E

	/*! Data source. */
	IBK::MultiLanguageString			m_dataSource;									// XML:E

	/*! Schedules for this component */
	std::vector<unsigned int>			m_scheduleIds;									// XML:E

	/*! Integer parameters. */
	IBK::IntPara						m_intPara[NUM_IP];								// XML:E

	/*! Reference id fo pipe properties for e.g. the GroundHeatExchanger model */
	IDType								m_pipePropertiesId = INVALID_ID;				// XML:E


	// *** PUBLIC MEMBER VARIABLES from NANDRAD::HydraulicNetworkComponent (without m_displayName) ***


	/*! Model type. */
	ModelType							m_modelType		= NUM_MT;						// XML:A:required

	/*! Parameters of the flow component. */
	IBK::Parameter						m_para[NUM_P];									// XML:E

	/*! Array parameters of the flow component */
	NANDRAD::DataTable					m_polynomCoefficients;							// XML:E

};

} // namespace VICUS


#endif // VICUS_NetworkComponentH
