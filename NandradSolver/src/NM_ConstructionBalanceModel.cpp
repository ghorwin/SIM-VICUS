/*	NANDRAD Solver Framework and Model Implementation.

	Copyright (c) 2012-today, Institut für Bauklimatik, TU Dresden, Germany

	Primary authors:
	  Andreas Nicolai  <andreas.nicolai -[at]- tu-dresden.de>
	  Anne Paepcke     <anne.paepcke -[at]- tu-dresden.de>

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


#include "NM_ConstructionSolverModel.h"
#include "NM_Constants.h"
#include "NM_ConstructionAdiabaticBCMoistureModel.h"
#include "NM_ConstructionOutsideBCMoistureModel.h"
#include "NM_KeywordList.h"

#include "WM_Types.h"

#include <NANDRAD_ConstructionInstance.h>
#include <NANDRAD_FindHelpers.h>
#include <NANDRAD_ModelInputReference.h>
#include <NANDRAD_ObjectList.h>
#include <NANDRAD_OutputGrid.h>
#include <NANDRAD_ParametrizationDefaults.h>
#include <NANDRAD_SimulationParameter.h>
#include <NANDRAD_KeywordList.h>

#include <IBK_crypt.h>

namespace NANDRAD_MODEL {


// *** Implementation of ConstructionSolverModel ***

ConstructionSolverModel::ConstructionSolverModel(unsigned int constructionSolverId, const std::string &displayName) :
	DefaultModel(constructionSolverId, displayName),
	DefaultStateDependency(SteadyState | ODE),
	OutputHandler::OutputHandler(this),
	m_moistureCalculationMode(CM_None),
	m_boundaryConditionA(nullptr),
	m_boundaryConditionB(nullptr),
	m_surfaceTemperatureA(0),
	m_surfaceTemperatureB(0),
	m_surfaceVaporPressureA(0),
	m_surfaceVaporPressureB(0),
	m_surfaceRelativeHumidityA(0),
	m_surfaceRelativeHumidityB(0),
	m_coarseGridWallModel(nullptr),
	m_coarseGridOutputHandler(nullptr),
	m_t(-1),
	m_constructionInstance(nullptr),
	m_outputGrid(nullptr)
{
}


ConstructionSolverModel::~ConstructionSolverModel() {
	if (m_coarseGridWallModel != nullptr)
		delete m_coarseGridWallModel;
	if (m_coarseGridOutputHandler != nullptr)
		delete m_coarseGridOutputHandler;
}

int ConstructionSolverModel::priorityOfModelEvaluation() const {
	// We have the following model dependency sequence at the end of evaluation stack:
	// ...
	// HeatingsLoadModel  +		WindowModel			+ ConstructionOutsideBCModel (at priorityOffsetTail)
	//		^						^								 ^
	//		|						|								 |
	//InsideBCHeatingsLoadModel + WindowsLoadModel 					 |
	//		^						^								 |
	//		|						|								 |
	//		|			InsideBCWindowsLoadModel	 				 |
	//	    |				^		^								 |
	//		|				|		|								 |
	//		|				|		InsideBCSWRadExchangeModel		 |
	//	    |				|		^								 |
	//		|			    |		|								 |
	//	    ConstructionInsideBCModel								 |
	//							^									 |
	//							|				 					 |
	//							 ConstructionSolverModel
	return AbstractStateDependency::priorityOffsetTail + 5;
}

unsigned int ConstructionSolverModel::nPrimaryStateResults() const {
	// simple calculation accepts two states for moisture
	if (m_moistureCalculationMode == CM_Average) {
		IBK_ASSERT(m_coarseGridWallModel != nullptr);
		return m_nElements + m_coarseGridWallModel->m_nElements;
	}
	// otherwise all states are registered inside wall model
	return m_n;
}


void ConstructionSolverModel::yInitial(double * y)  {

	if (m_moistureCalculationMode == CM_Average) {

		IBK_ASSERT(m_coarseGridWallModel != nullptr);
		// copy internal energy values from wall solver instance
		for (unsigned int i = 0; i < m_nElements; ++i) {
			y[m_energyBalanceEquation[i] ] = y0()[ 2 * i];
		}
		for (unsigned int i = 0; i < m_coarseGridWallModel->m_nElements; ++i) {
			y[m_moistureBalanceEquation[i]] = m_coarseGridWallModel->y0()[2 * i + 1];
		}
	}
	else {
		// copy initial valuess from wall solver instance
		std::memcpy(y, y0(), m_n * sizeof(double));
	}

}


void ConstructionSolverModel::setY(const double * y) {


	// store solution quantity
	std::memcpy(&m_y[0], y, nPrimaryStateResults() * sizeof(double));

	switch (m_moistureCalculationMode) {
		case CM_None :
			decomposeThermal(y);
			//decompose(y);
		break;

		case CM_Detailed : {
			decompose(y);
		}
		break;

		case CM_Average : {

			IBK_ASSERT(m_coarseGridWallModel != nullptr);
			// copy thermal quantities
			for (unsigned int i = 0; i < m_nElements; ++i) {
				m_tempY[2 * i] = y[m_energyBalanceEquation[i]];
			}
			for (unsigned int i = 0; i < m_coarseGridWallModel->m_nElements; ++i) {
				m_tempAv[i] = y[m_moistureBalanceEquation[i]];
			}
			// calculate moisture field:
			// we set a constant moisture density value
			// at all inside elements and extrapolate boundary values
			// with the help of external elements
			detailedFieldFromAverageField(&m_tempDet[0], &m_tempAv[0]);
			// get detaiuled values
			for (unsigned int i = 0; i < m_nElements; ++i) {
				m_tempY[2 * i + 1] = m_tempDet[i];
			}
			// update states (temperatures + moisture)
			decompose(&m_tempY[0]);

			// calculate average temperature field
			for (unsigned int i = 0; i < m_coarseGridWallModel->m_nElements; ++i) {
				m_tempYCoarse[2 * i + 1] = y[m_moistureBalanceEquation[i]];
			}
			for (unsigned int i = 0; i < m_nElements; ++i) {
				m_tempDet[i] = y[m_energyBalanceEquation[i]];
			}
			// we average temperature values at a coarse grid
			averageFieldFromDetailedField(&m_tempAv[0], &m_tempDet[0]);
			// get detaiuled values
			for (unsigned int i = 0; i < m_coarseGridWallModel->m_nElements; ++i) {
				m_tempYCoarse[2 * i]  = m_tempAv[i];
			}
			// update states (temperatures + moisture)
			m_coarseGridWallModel->decompose(&m_tempYCoarse[0]);
		}
		break;
	} // switch

	// calculate temperature on left and right sode of construction
	m_surfaceTemperatureA = TsLeft();
	m_surfaceTemperatureB = TsRight();

	// calculate vapor pressure on left and right sode of construction
	if (m_moistureCalculationMode != CM_None) {
		m_surfaceVaporPressureA = pvsLeft();
		m_surfaceVaporPressureB = pvsRight();
		m_surfaceRelativeHumidityA = RHsLeft();
		m_surfaceRelativeHumidityB = RHsRight();
	}
}


int ConstructionSolverModel::ydot(double * ydot) {
	if (m_moistureCalculationMode == CM_Average) {
		// copy thermal divergences
		for (unsigned int i = 0; i < m_nElements; ++i) {
			m_ydotAverage[m_energyBalanceEquation[i]] = divergences()[2 * i];
		}
		// copy moisture divergences
		for (unsigned int i = 0; i < m_coarseGridWallModel->m_nElements; ++i) {
			m_ydotAverage[m_moistureBalanceEquation[i]] = m_coarseGridWallModel->divergences()[2 * i + 1];
		}
		// copy vector of divergences
		std::memcpy(ydot, &m_ydotAverage[0], nPrimaryStateResults() * sizeof(double));
	}
	else {
		// store derivatives quantity
		std::memcpy(&ydot[0], divergences(), m_n * sizeof(double));
	}
	// ydot is called at the end of an update cycle
	// an non-linear iteration calls:
	// - setTime()
	// - setY()
	// - update()
	// - ydot()
	// signal success
	return 0;
}

bool ConstructionSolverModel::addOutputDefinition(const WALL_MODEL::OutputHandler::OutputDefinition &outputDef)
{
	m_outputDefs.insert(outputDef);
	return true;
}

void ConstructionSolverModel::initOutputs(const std::string & outputDir,
	const NANDRAD::OutputGrid *outputGrid, int startYear)
{
	const char * const FUNC_ID = "[ConstructionSolverModel::initOutputs]";

	// if no outputs are registered we do nothing
	if (m_outputDefs.empty())
		return;

	// Must have an output grid!
	IBK_ASSERT(outputGrid != nullptr);

	// copy output grid
	m_outputGrid = outputGrid;

	try {
		// generate unique construction ID to be used as basename for output files of this construction
		std::string construction_ID = m_constructionInstance->m_displayName;
		if (construction_ID.empty())
			construction_ID = IBK::val2string(m_constructionInstance->m_id);

		// generate hash from ID name, since we don't have access to the actual construction file

		std::string constructionHash = IBK::val2string( IBK::SuperFastHash(construction_ID) );


		WALL_MODEL::OutputHandler::initOutputs(outputDir, m_outputDefs, construction_ID, constructionHash, startYear);

		// additional outputs for coarse grid
		if (m_moistureCalculationMode == CM_Average) {
			IBK_ASSERT (m_coarseGridOutputHandler != nullptr);
			m_coarseGridOutputHandler->initOutputs(outputDir + std::string("_coarse"), m_outputDefs, construction_ID, constructionHash, startYear);
		}
	}
	catch (IBK::Exception &ex)
	{
		throw IBK::Exception(ex, IBK::FormatString("Error initializing output for Construction solver for"
			"construction instance with ID %1!").arg(constructionInstance()->m_id),
			FUNC_ID);
	}
}


void ConstructionSolverModel::writeOutputs(double t)
{
	const char * const FUNC_ID = "[ConstructionSolverModel::initOutputs]";

	// if no outputs are registered we do nothing
	if (m_outputDefs.empty())
		return;

	// for an inactive output grid time point
	// we do nothing
	if (!m_outputGrid->isActive(t))
		return;

	try {
		WALL_MODEL::OutputHandler::writeOutputs(t);

		if (m_moistureCalculationMode == CM_Average) {
			IBK_ASSERT(m_coarseGridOutputHandler != nullptr);
			m_coarseGridOutputHandler->writeOutputs(t);
		}
	}
	catch (IBK::Exception &ex)
	{
		throw IBK::Exception(ex, IBK::FormatString("Error writing output for Construction solver for"
			"construction instance with ID %1!").arg(constructionInstance()->m_id),
			FUNC_ID);
	}
}


void ConstructionSolverModel::setup(const NANDRAD::ConstructionInstance & parameters,
	const std::map<std::string, MM::Material> & materials,
	const std::map<unsigned int, DELPHIN_LIGHT::ConstructionType> &constructionTypes,
	const NANDRAD::ParametrizationDefaults &defaultPara,
	const NANDRAD::SimulationParameter &simPara,
	const WALL_MODEL::DiscretizationOptions & discOptions)
{
	const char * const FUNC_ID = "[ConstructionSolverModel::setup]";

	m_constructionInstance = &parameters;

	// retrieve wall area...
	m_wallArea = m_constructionInstance->m_para[NANDRAD::ConstructionInstance::CP_AREA].value;
	std::vector<NANDRAD::EmbeddedObject>::const_iterator embeddedObjectIt
		= m_constructionInstance->m_embeddedObjects.begin();

	// ... and correct it cutting out all windows and doors
	for( ; embeddedObjectIt != m_constructionInstance->m_embeddedObjects.end(); ++embeddedObjectIt)
	{
		m_wallArea -= embeddedObjectIt->m_para[NANDRAD::EmbeddedObject::P_Area].value;
	}
	// The correctness of the wall area is checked by the boundary condition models. Therefore, we
	// do not need an additional exception block

	// retrieve construction type (layers and materials) from data base
	unsigned int conTypeId = parameters.m_constructionTypeId;
	std::map<unsigned int, DELPHIN_LIGHT::ConstructionType>::const_iterator fit = constructionTypes.find(conTypeId);
	if (fit == constructionTypes.end()) {
		throw IBK::Exception(IBK::FormatString("Construction type with ID %1 referenced from construction "
			"instance with ID %2 not defined/referenced in project file.").arg(conTypeId).arg(parameters.m_id),
			FUNC_ID);
	}
	const DELPHIN_LIGHT::ConstructionType * conType = &fit->second;

	// copy calculation mode
	if (simPara.m_stringPara[NANDRAD::SimulationParameter::SSP_WALLMOISTUREBALANCECALCULATIONMODE].empty()) {
		throw IBK::Exception(IBK::FormatString("Missing Parameter '%1' in SimulationParameter!")
			.arg(NANDRAD::KeywordList::Keyword("SimulationParameter::stringPara_t",
				NANDRAD::SimulationParameter::SSP_WALLMOISTUREBALANCECALCULATIONMODE) ),
			FUNC_ID);
	}

	try {
		m_moistureCalculationMode = (WallMoistureBalanceCalculationMode)
			KeywordList::Enumeration("ConstructionSolverModel::WallMoistureBalanceCalculationMode",
				simPara.m_stringPara[NANDRAD::SimulationParameter::SSP_WALLMOISTUREBALANCECALCULATIONMODE]);
	}
	catch (IBK::Exception & ex) {
		throw IBK::Exception(ex, IBK::FormatString("Unknown calculation mode '%1' in 'SimulationParameter! "
			"We only accept modes '%2', '%3' and '%4'.")
			.arg(simPara.m_stringPara[NANDRAD::SimulationParameter::SSP_WALLMOISTUREBALANCECALCULATIONMODE])
			.arg(KeywordList::Keyword("ConstructionSolverModel::WallMoistureBalanceCalculationMode",
				CM_Average))
			.arg(KeywordList::Keyword("ConstructionSolverModel::WallMoistureBalanceCalculationMode",
				CM_Detailed))
			.arg(KeywordList::Keyword("ConstructionSolverModel::WallMoistureBalanceCalculationMode",
				CM_None)), FUNC_ID);
	}

	// error: missleading definitions
	if (!simPara.m_flags[NANDRAD::SimulationParameter::SF_ENABLE_MOISTURE_BALANCE].isEnabled()
		&& m_moistureCalculationMode != CM_None) {
		throw IBK::Exception(IBK::FormatString("Invalid calculation mode '%1' in 'SimulationParameter! "
			"We only accept mode '%2' for thermal calculation.")
			.arg(simPara.m_stringPara[NANDRAD::SimulationParameter::SSP_WALLMOISTUREBALANCECALCULATIONMODE])
			.arg(KeywordList::Keyword("ConstructionSolverModel::WallMoistureBalanceCalculationMode",
				CM_None)), FUNC_ID);
	}

	// for simple and none-mode we only perform detailed thermal simulation
	WALL_MODEL::WallModel::TransportMode transportMode = WALL_MODEL::WallModel::MODE_THERMAL;
	// detailed mode includes detailed moisture transport calculation
	if (m_moistureCalculationMode == CM_Detailed || m_moistureCalculationMode == CM_Average) {
		transportMode = WALL_MODEL::WallModel::MODE_HYGROTHERMAL_WITHOUT_LIQUID;
	}
	// initialise wall model, setup discretization, resize states, fluxes and sources, assign
	// material data
	try {
		init(*conType, discOptions, materials, transportMode);
	}
	catch (IBK::Exception & ex) {
		throw IBK::Exception(ex, IBK::FormatString("Error initializing construction solver for construction instance "
			"with ID %1.").arg(parameters.m_id), FUNC_ID);
	}

	// suppress average mode for minimum discretization
	if ( (discOptions.m_density == 0 || m_nElements == 2) && m_moistureCalculationMode == CM_Average) {
		m_moistureCalculationMode = CM_Detailed;
	}

	// calculate moisture layer offsets for average balance
	if (m_moistureCalculationMode == CM_Average) {
		// initialize coarse grisd wall model
		m_coarseGridWallModel = new WALL_MODEL::WallModel();
		// create a coarse grid discrteization
		WALL_MODEL::DiscretizationOptions coarseDisc(discOptions);
		coarseDisc.m_density = 0;

#ifdef FineBoundaryLayerForMixedGrid
		// setup with all data including coarse discrteization
		m_coarseGridWallModel->init(*conType, coarseDisc, materials, transportMode, &m_elements, &m_materialLayerElementOffset);
#else
		// setup with all data including coarse discrteization
		m_coarseGridWallModel->init(*conType, coarseDisc, materials, transportMode);
#endif
		// shift grid lines of boundray elements so that they match
		setupGridAveraging(m_coarseGridWallModel->m_elements);
		// initialize output handler for coarse grid
		m_coarseGridOutputHandler = new WALL_MODEL::OutputHandler(m_coarseGridWallModel);
	}

	// retrieve initial temperature from project file
	m_TInitial = 0.0;
	// check for additional initial temperature
	if (!parameters.m_para[NANDRAD::ConstructionInstance::CP_INITIAL_TEMPERATURE].name.empty()) {
		m_TInitial = parameters.m_para[NANDRAD::ConstructionInstance::CP_INITIAL_TEMPERATURE].value;
	}
	// if no initial temperature is given explicitely retrieve temperature from simulation parameter
	else if(!defaultPara.m_para[NANDRAD::ParametrizationDefaults::SP_INITIAL_TEMPERATURE].name.empty() &&
			defaultPara.m_mode == NANDRAD::ParametrizationDefaults::SM_LAZY) {
		m_TInitial = defaultPara.m_para[NANDRAD::ParametrizationDefaults::SP_INITIAL_TEMPERATURE].value;
		IBK::IBK_Message( IBK::FormatString("No initial temperature is given for construction instance with id %1. "
											"Retreiving initial temperature of %2 C from default parameter settings")
											.arg(id())
											.arg(defaultPara.m_para[NANDRAD::ParametrizationDefaults::SP_INITIAL_TEMPERATURE].get_value("C")),
											IBK::MSG_WARNING, FUNC_ID, 2);
	}
	else
		throw IBK::Exception(IBK::FormatString("Error initializing ConstructionSolverModel of ConstructionInstance with ID %1: "
			"Parameter '%2' is not defined.")
			.arg(parameters.m_id)
			.arg(NANDRAD::KeywordList::Keyword("ConstructionInstance::para_t", NANDRAD::ConstructionInstance::CP_INITIAL_TEMPERATURE)),
			FUNC_ID);

	// check validity of initial temperature value
	if (m_TInitial < 0.0 ) {
		throw IBK::Exception(IBK::FormatString("Error initializing ConstructionSolverModel of constructionInstance with ID %1: "
			"Parameter 'InitialTemperature' is smaller than zero.")
			.arg(parameters.m_id),
			FUNC_ID);
	}

	m_rHInitial = 0.0;

	// set initial relative humdity for moisture calculation
	if (m_moistureCalculationMode == CM_Average || m_moistureCalculationMode == CM_Detailed) {
		// check for additional initial temperature
		if (!parameters.m_para[NANDRAD::ConstructionInstance::CP_INITIAL_RELATIVE_HUMIDITY].name.empty()) {
			m_rHInitial = parameters.m_para[NANDRAD::ConstructionInstance::CP_INITIAL_RELATIVE_HUMIDITY].value;
		}
		// if no initial temperature is given explicitely retrieve temperature from simulation parameter
		else if (!defaultPara.m_para[NANDRAD::ParametrizationDefaults::SP_INITIAL_RELATIVE_HUMIDITY].name.empty() &&
			defaultPara.m_mode == NANDRAD::ParametrizationDefaults::SM_LAZY) {
			m_rHInitial = defaultPara.m_para[NANDRAD::ParametrizationDefaults::SP_INITIAL_RELATIVE_HUMIDITY].value;
			IBK::IBK_Message(IBK::FormatString("No initial relative humidity is given for construction instance with id %1. "
				"Retreiving initial relative humidity of %2 % from default parameter settings")
				.arg(id())
				.arg(defaultPara.m_para[NANDRAD::ParametrizationDefaults::SP_INITIAL_RELATIVE_HUMIDITY].get_value("%")),
				IBK::MSG_WARNING, FUNC_ID, 2);
		}
		else
			throw IBK::Exception(IBK::FormatString("Error initializing ConstructionSolverModel of ConstructionInstance with ID %1: "
				"Parameter '%2' is not defined.")
				.arg(parameters.m_id)
				.arg(NANDRAD::KeywordList::Keyword("ConstructionInstance::para_t", NANDRAD::ConstructionInstance::CP_INITIAL_RELATIVE_HUMIDITY)),
				FUNC_ID);

		// check validity of initial temperature value
		if (m_rHInitial < 0.0 || m_rHInitial > 1.0) {
			throw IBK::Exception(IBK::FormatString("Error initializing ConstructionSolverModel of constructionInstance with ID %1: "
				"Parameter 'InitialRelativeHumidity' is outside the interval [0,1].")
				.arg(parameters.m_id),
				FUNC_ID);
		}
	}

	// initialise wall solver instance with relative humidity of 0% (dry construction)
	WALL_MODEL::WallModel::setInitialCondition(m_TInitial, m_rHInitial);
	WALL_MODEL::WallModel::decompose(WALL_MODEL::WallModel::y0());
	// resize solution variable
	m_y.resize(nPrimaryStateResults());
	// resize temporary vector
	if (m_moistureCalculationMode == CM_Average) {
		// initialise wall solver instance with relative humidity of 0% (dry construction)
		m_coarseGridWallModel->setInitialCondition(m_TInitial, m_rHInitial);
		m_coarseGridWallModel->decompose(m_coarseGridWallModel->y0());
		// resize helper containers
		m_ydotAverage.resize(nPrimaryStateResults());
		m_tempY.resize(m_n);
		m_tempYCoarse.resize(m_coarseGridWallModel->m_n);
	}
	// copy initial conditions into m_y vector
	yInitial(&m_y[0]);
}


void ConstructionSolverModel::setBoundaryConditionA(ConstructionBoundaryCondition & boundaryCond) {
	m_boundaryConditionA = &boundaryCond;
	// set boundary condition inside surface modules: location A
	m_boundaryConditionA->setSurfaceTemperature(&m_surfaceTemperatureA);

	if (m_moistureCalculationMode != CM_None) {
		m_boundaryConditionA->setSurfaceRelativeHumidity(&m_surfaceRelativeHumidityA);
		m_boundaryConditionA->setSurfaceVaporPressure(&m_surfaceVaporPressureA);
	}
}


void ConstructionSolverModel::setBoundaryConditionB(ConstructionBoundaryCondition & boundaryCond) {
	m_boundaryConditionB = &boundaryCond;
	// set boundary condition inside surface modules: location B
	m_boundaryConditionB->setSurfaceTemperature(&m_surfaceTemperatureB);

	if (m_moistureCalculationMode != CM_None) {
		m_boundaryConditionB->setSurfaceRelativeHumidity(&m_surfaceRelativeHumidityB);
		m_boundaryConditionB->setSurfaceVaporPressure(&m_surfaceVaporPressureB);
	}
}

void ConstructionSolverModel::initResults(const std::vector<AbstractModel*> & models)
{
	// ensure that all pointers are set
	IBK_ASSERT(m_constructionInstance != nullptr);
	// resizes m_results vector from keyword list
	DefaultModel::initResults(models);

	unsigned int idIfaceA = 0;
	unsigned int idIfaceB = 0;
	// fill interface ids
	for (unsigned int i = 0; i < m_constructionInstance->m_interfaces.size(); ++i) {
		const NANDRAD::Interface &iface = m_constructionInstance->m_interfaces[i];
		// interface is at locationA
		if (iface.m_location == NANDRAD::Interface::IT_A)
			idIfaceA = iface.m_id;
		else
			idIfaceB = iface.m_id;
	}

	// calculate temperature on left and right side of construction
	m_surfaceTemperatureA = TsLeft();
	m_surfaceTemperatureB = TsRight();

	// calculate vapor pressure on left and right sode of construction
	if (m_moistureCalculationMode != CM_None) {
		m_surfaceVaporPressureA = pvsLeft();
		m_surfaceVaporPressureB = pvsRight();
		m_surfaceRelativeHumidityA = RHsLeft();
		m_surfaceRelativeHumidityB = RHsRight();
	}
}

void ConstructionSolverModel::resultValueRefs(std::vector<const double *> &res) const {
	// first seach in m_results vector
	DefaultModel::resultValueRefs(res);

	// Additionally we provide a reference to the solution quantity.
	// This reference will be accessed by the corresponding RoomStatesModel.
	const double * yPtr = &m_y[0];
	for (unsigned int i = 0; i < m_y.size(); ++i, ++yPtr) {
		// loop over all vector valued results
		res.push_back(yPtr);
	}

	// And we provide a reference to the divergences.
	const double * ydotPtr = &m_ydot[0];
	for (unsigned int i = 0; i < m_ydot.size(); ++i, ++ydotPtr) {
		// loop over all vector valued results
		res.push_back(ydotPtr);
	}

	// Add all boundary fluxes to result reference list.
	// Location A:
	if (m_boundaryConditionA != nullptr) {
		res.push_back(m_boundaryConditionA->heatConduction());
		res.push_back(m_boundaryConditionA->swRadAbsorbed());
		res.push_back(m_boundaryConditionA->lwRadBalance());

		if (m_moistureCalculationMode != CM_None) {
			res.push_back(m_boundaryConditionA->vaporDiffusion());
			res.push_back(m_boundaryConditionA->vaporEnthalpy());
		}
		if(m_boundaryConditionA->swRadOnPlane() != nullptr)
			res.push_back(m_boundaryConditionA->swRadOnPlane());
	}
	// Location B:
	if (m_boundaryConditionB != nullptr) {
		res.push_back(m_boundaryConditionB->heatConduction());
		res.push_back(m_boundaryConditionB->swRadAbsorbed());
		res.push_back(m_boundaryConditionB->lwRadBalance());

		if (m_moistureCalculationMode != CM_None) {
			res.push_back(m_boundaryConditionB->vaporDiffusion());
			res.push_back(m_boundaryConditionB->vaporEnthalpy());
		}
		if (m_boundaryConditionB->swRadOnPlane() != nullptr)
			res.push_back(m_boundaryConditionB->swRadOnPlane());
	}

	// And we provide a reference to all field fluxes.
	// Create a new input reference
	std::vector<const double*>::const_iterator valueRefIt = inputValueRefs(InputRef_FieldFlux);
	std::set<unsigned int>::const_iterator layerIt = m_activeLayerIndices.begin();

	for (; layerIt != m_activeLayerIndices.end(); ++layerIt, ++valueRefIt) {
		res.push_back(*valueRefIt);
	}
}


const double * ConstructionSolverModel::resultValueRef(const QuantityName & quantityName) const {
	// first seach in m_results vector
	const double *refValue = DefaultModel::resultValueRef(quantityName);
	if (refValue != nullptr)
		return refValue;


	// Additionally we provide a reference to the solution quantity.
	// This reference will be accessed by the corresponding ConstructionSTatesModel
	// or ConstructionStatesMoistureModel.

	if (quantityName == std::string("y"))
	{
		// complete vector
		if (quantityName.index() == -1)
			return &m_y[0];
		// wrong index size
		if ((unsigned int)quantityName.index() > m_y.size())
			return nullptr;

		return &m_y[(unsigned int)quantityName.index()];
	}
	// And we provide a reference to the divergences.
	if (quantityName == std::string("ydot")) {

		if (m_moistureCalculationMode == CM_Average) {
			// complete vector
			if (quantityName.index() == -1)
				return &m_ydotAverage[0];
			// wrong index size
			if ((unsigned int)quantityName.index() > m_ydotAverage.size())
				return nullptr;

			return &m_ydotAverage[(unsigned int)quantityName.index()];
		}
		else {
			// complete vector
			if (quantityName.index() == -1)
				return &m_ydot[0];
			// wrong index size
			if ((unsigned int)quantityName.index() > m_ydot.size())
				return nullptr;

			return &m_ydot[(unsigned int)quantityName.index()];
		}
	}
	// Offer references to all fluxes on location A and B
	if (quantityName == "HeatConductionA") {
		if (m_boundaryConditionA == nullptr)
			return nullptr;
		return m_boundaryConditionA->heatConduction();
	}
	if (quantityName == "LWRadBalanceA") {
		if (m_boundaryConditionA == nullptr)
			return nullptr;
		return m_boundaryConditionA->lwRadBalance();
	}
	if (quantityName == "SWRadAbsorbedA") {
		if (m_boundaryConditionA == nullptr)
			return nullptr;
		return m_boundaryConditionA->swRadAbsorbed();
	}
	if (quantityName == "SWRadOnPlaneA") {
		if (m_boundaryConditionA == nullptr)
			return nullptr;
		// this pointer may be nullptr
		return m_boundaryConditionA->swRadOnPlane();
	}
	if (quantityName == "VaporDiffusionA") {
		if (m_boundaryConditionA == nullptr)
			return nullptr;
		return m_boundaryConditionA->vaporDiffusion();
	}
	if (quantityName == "VaporDiffusionEnthalpyA") {
		if (m_boundaryConditionA == nullptr)
			return nullptr;
		return m_boundaryConditionA->vaporEnthalpy();
	}
	if (quantityName == "HeatConductionB") {
		if (m_boundaryConditionB == nullptr)
			return nullptr;
		return m_boundaryConditionB->heatConduction();
	}
	if (quantityName == "SWRadAbsorbedB") {
		if (m_boundaryConditionB == nullptr)
			return nullptr;
		// this pointer may be nullptr
		return m_boundaryConditionB->swRadAbsorbed();
	}
	if (quantityName == "SWRadOnPlaneB") {
		if (m_boundaryConditionB == nullptr)
			return nullptr;
		return m_boundaryConditionB->swRadOnPlane();
	}
	if (quantityName == "LWRadBalanceB") {
		if (m_boundaryConditionB == nullptr)
			return nullptr;
		return m_boundaryConditionB->lwRadBalance();
	}
	if (quantityName == "VaporDiffusionB") {
		if (m_boundaryConditionB == nullptr)
			return nullptr;
		return m_boundaryConditionB->vaporDiffusion();
	}
	if (quantityName == "VaporDiffusionEnthalpyB") {
		if (m_boundaryConditionB == nullptr)
			return nullptr;
		return m_boundaryConditionB->vaporEnthalpy();
	}

	// now check constant parameters (we provide project parameters as input references
	// field flux
	if (quantityName == std::string("FieldFlux") ) {
		// Error: wrong index format
		if (quantityName.index() < 0) {
			return nullptr;
		}
		// Error: the material layer index exceeds wall layer count
		if ((unsigned int)quantityName.index() >= m_layerSources.size()) {
			return nullptr;
		}
		// Error: we already set a field condition to the current layer
		std::set<unsigned int>::const_iterator actLayerIt =
			m_activeLayerIndices.find((unsigned int)quantityName.index());
		if (actLayerIt == m_activeLayerIndices.end()) {
			return nullptr;
		}
		// Create a new input reference
		std::vector<const double*>::const_iterator valueRefIt = inputValueRefs(InputRef_FieldFlux);
		std::set<unsigned int>::const_iterator layerIt = m_activeLayerIndices.begin();

		for (; layerIt != actLayerIt; ++layerIt) {
			++valueRefIt;
			IBK_ASSERT(valueRefIt != inputValueRefs().end());
		}
		return *valueRefIt;
	}

	// only scalar quantities are allowed any longer
	if (quantityName.index() != -1)
		return nullptr;

	// check if we request a prededined construction instance parameter
	if(NANDRAD::KeywordList::KeywordExists("ConstructionInstance::para_t",quantityName.name())
		&& !constructionInstance()->m_para[NANDRAD::KeywordList::Enumeration("ConstructionInstance::para_t",quantityName.name())].name.empty())
	{
		return &constructionInstance()->m_para[NANDRAD::KeywordList::Enumeration("ConstructionInstance::para_t",quantityName.name())].value;
	}
	return nullptr;
}


void ConstructionSolverModel::resultDescriptions(std::vector<QuantityDescription> & resDesc) const {
	// fill definitions from keyword list
	DefaultModel::resultDescriptions(resDesc);
	// resize heationgsload
	IBK_ASSERT(!m_construction.m_materialLayers.empty());
	resDesc[VVR_ThermalLoad].resize(m_construction.m_materialLayers.size());
	// offer a reference to the solution quantity
	QuantityDescription result;
	result.m_name = std::string("y");
	result.m_description = std::string("Internal Energy Density");
	result.m_unit = std::string("W/m3");
	result.m_constant = true;
	result.m_size = nPrimaryStateResults();
	resDesc.push_back(result);
	// offer a reference to the divergences
	result.m_name = std::string("ydot");
	result.m_description = std::string("Divergences");
	result.m_unit = std::string("W/m3");
	result.m_size = nPrimaryStateResults();
	resDesc.push_back(result);

	// add all boundary condition flux densities: location A
	if (m_boundaryConditionA != nullptr) {
		result.m_name = std::string("HeatConductionA");
		result.m_description = std::string("Heat conduction flux density on location A.");
		result.m_unit = std::string("W/m2");
		result.m_size = 1;
		resDesc.push_back(result);

		result.m_name = std::string("SWRadAbsorbedA");
		result.m_description = std::string("Absorbed heat flux density by short wave radiation on location A.");
		result.m_unit = std::string("W/m2");
		resDesc.push_back(result);

		result.m_name = std::string("SWRadOnPlaneA");
		result.m_description = std::string("Short wave radiation flux density at plane on location A.");
		result.m_unit = std::string("W/m2");
		resDesc.push_back(result);

		result.m_name = std::string("LWRadBalanceA");
		result.m_description = std::string("Long wave radiation balance flux density on location A.");
		result.m_unit = std::string("W/m2");
		resDesc.push_back(result);

		if (m_moistureCalculationMode != CM_None) {
			result.m_name = std::string("VaporDiffusionA");
			result.m_description = std::string("Vapor diffusion flux density on location A.");
			result.m_unit = std::string("W/m2");
			resDesc.push_back(result);

			result.m_name = std::string("VaporDiffusionEnthalpyA");
			result.m_description = std::string("Vapor diffusion enthalpy flux density on location A.");
			result.m_unit = std::string("W/m2");
			resDesc.push_back(result);
		}
	}
	// add all boundary condition flux densities: location B
	if (m_boundaryConditionB != nullptr) {
		result.m_name = std::string("HeatConductionB");
		result.m_description = std::string("Heat conduction flux density on location B.");
		result.m_unit = std::string("W/m2");
		result.m_size = 1;
		resDesc.push_back(result);

		result.m_name = std::string("SWRadAbsorbedB");
		result.m_description = std::string("Absorbed heat flux density by short wave radiation on location B.");
		result.m_unit = std::string("W/m2");
		resDesc.push_back(result);

		result.m_name = std::string("SWRadOnPlaneB");
		result.m_description = std::string("Short wave radiation flux density at plane on location B.");
		result.m_unit = std::string("W/m2");
		resDesc.push_back(result);

		result.m_name = std::string("LWRadBalanceB");
		result.m_description = std::string("Long wave radiation balance flux density on location B.");
		result.m_unit = std::string("W/m2");
		resDesc.push_back(result);

		if (m_moistureCalculationMode != CM_None) {
			result.m_name = std::string("VaporDiffusionB");
			result.m_description = std::string("Vapor diffusion flux density on location B.");
			result.m_unit = std::string("W/m2");
			resDesc.push_back(result);

			result.m_name = std::string("VaporDiffusionEnthalpyB");
			result.m_description = std::string("Vapor diffusion enthalpy flux density on location B.");
			result.m_unit = std::string("W/m2");
			resDesc.push_back(result);
		}
	}

	// add all field fluxes
	std::vector<VectorValuedQuantityIndex> indicesFieldFlux;
	std::vector<std::string> descriptionsFieldFlux;

	for (std::set<unsigned int>::const_iterator it = m_activeLayerIndices.begin();
		it != m_activeLayerIndices.end(); ++it) {
		VectorValuedQuantityIndex index;
		index.set(VectorValuedQuantityIndex::IK_Index, *it);
		indicesFieldFlux.push_back(index);
		descriptionsFieldFlux.push_back(encodeVectorValuedQuantityIndex("FieldFlux", *it));
	}

	if(!indicesFieldFlux.empty()) {
		result.m_name = std::string("FieldFlux");
		result.m_indexKeys = indicesFieldFlux;
		result.m_indexKeyDescriptions = descriptionsFieldFlux;
		result.m_description = std::string("Heat sources inside construction");
		result.m_unit = std::string("W");
		result.m_constant = true;
		resDesc.push_back(result);
	}
	// offer reference to all construction instance parameters
	for(unsigned int i = 0; i < NANDRAD::ConstructionInstance::NUM_CP; ++i) {
		// only add parameters that are really filled
		if(!constructionInstance()->m_para[i].name.empty() )
		{
			result.m_name = std::string(constructionInstance()->m_para[i].name);
			result.m_description = NANDRAD::KeywordList::Description("ConstructionInstance::para_t", i);
			result.m_unit = constructionInstance()->m_para[i].unit().name();
			resDesc.push_back(result);
		}
	}
}

void ConstructionSolverModel::inputReferenceDescriptions(std::vector<QuantityDescription> & resDesc) const {

	// set end parameter
	DefaultStateDependency::inputReferenceDescriptions(resDesc);

	// we only request necessary input references.
	std::string category = "ConstructionSolverModel::InputReferences";

	QuantityDescription &fieldFluxDesc = resDesc[InputRef_FieldFlux];
	// retreive index information from vector valued results

	if (m_activeLayerIndices.empty())
		fieldFluxDesc.clear();
	else
		fieldFluxDesc.resize(m_activeLayerIndices, VectorValuedQuantityIndex::IK_Index);
}


bool ConstructionSolverModel::registerInputReference(unsigned int sourceID,
	NANDRAD::ModelInputReference::referenceType_t referenceType,
	const QuantityName &quantity,
	const QuantityName &targetName,
	NANDRAD::ImplicitModelFeedback::operation_t operation)
{
	const char * const FUNC_ID = "[ConstructionSolverModel::registerInputReference]";

	try {
		// We only accept quantities with the target names 'FieldFlux'!
		if(targetName.name() != "FieldFlux") {
			return false;
		}
		// We only allow operation 'Add'
		if(operation != NANDRAD::ImplicitModelFeedback::IFO_ADD) {
			throw IBK::Exception(IBK::FormatString("Only operation 'Add' is accepted!"),
				FUNC_ID);
		}
		// we don't accept target names with index information
		if (targetName.index() == -1)
		{
			throw IBK::Exception(IBK::FormatString("Target name must include a vector index!"),
				FUNC_ID);
		}

		// interpret target index as material layer number
		int materialLayerIdx = targetName.index();

		// Error: wrong index format
		if(materialLayerIdx < 0) {
			throw IBK::Exception(IBK::FormatString("The requested target name is 'FieldFlux[index=...]'!")
						, FUNC_ID);
		}
		// Error: we already set a field condition to the current layer
		if(m_activeLayerIndices.find((unsigned int) materialLayerIdx) != m_activeLayerIndices.end() ) {
			throw IBK::Exception(IBK::FormatString("Material layer with index %1 is referenced twice by a field condition!")
				.arg(materialLayerIdx), FUNC_ID);
		}
		// Error: the material layer index exceeds wall layer count
		if((unsigned int) materialLayerIdx >= m_layerSources[SOURCE_HEAT_PRODUCTION_RATE].size() ) {
			throw IBK::Exception(IBK::FormatString("Material layer index %1 exceeds the number of wall layers!")
				.arg(materialLayerIdx), FUNC_ID);
		}

		// find enumeration value for target
		int targetType = decodeInputReferenceTargeType(targetName.name());
		// invalid type
		IBK_ASSERT (targetType >= 0);
		// Create a new input reference
		InputReference &inputRef = inputReference(targetType, materialLayerIdx);
		// Programming error: we already defined an input reference to the current material layer without
		// storing the index inside the m_activeLayerIndices list
		IBK_ASSERT(inputRef.m_referenceType == NANDRAD::ModelInputReference::NUM_MRT);

		inputRef.m_id = sourceID;
		inputRef.m_referenceType = referenceType;
		inputRef.m_sourceName = quantity;
		inputRef.m_targetName = targetName;

		// Store the layer index.
		m_activeLayerIndices.insert((unsigned int) materialLayerIdx);

		return true;
	}
	catch (IBK::Exception &ex) {
		throw IBK::Exception(ex, IBK::FormatString("Error adding implicit model feedback to target name '%1' and "
					"construction instance with id %2!")
					.arg(targetName.name())
					.arg(id())
					, FUNC_ID);
	}
}

void ConstructionSolverModel::stateDependencies(std::vector< std::pair<const double *, const double *> > &resultInputValueReferences) const
{
	// clear pattern
	if(!resultInputValueReferences.empty() )
		resultInputValueReferences.clear();
	// connect first and second value of ydot to
	// the boundary flux at location A

	if (m_moistureCalculationMode == CM_Detailed) {

		std::vector<std::set<const double *> > depsYdotFluxes(nPrimaryStateResults());
		std::vector<std::set<const double *> > depsThermalFluxesEner(m_nElements + 1);
		std::vector<std::set<const double *> > depsThermalFluxesMoist(m_nElements + 1);
		std::vector<std::set<const double *> > depsMoistureFluxesEner(m_nElements + 1);
		std::vector<std::set<const double *> > depsMoistureFluxesMoist(m_nElements + 1);
		std::vector<std::set<const double *> > depsEnerY(m_nElements);
		std::vector<std::set<const double *> > depsMoistY(m_nElements);

		if (m_boundaryConditionA != nullptr) {
			// heat conduction flux
			// first matrix entries (boundary conditions at side A)
			const double *inputValue = m_boundaryConditionA->heatConduction();
			IBK_ASSERT(inputValue != nullptr);
			// store the pair of adresses of result value and input
			// value inside pattern list
			depsThermalFluxesEner[0].insert(inputValue);
			// m oisture balance
			// absorbed short wave radiation flux
			inputValue = m_boundaryConditionA->swRadAbsorbed();
			IBK_ASSERT(inputValue != nullptr);
			// store the pair of adresses of result value and input
			// value inside pattern list
			depsThermalFluxesEner[0].insert(inputValue);
			// long wave radiation balance flux
			inputValue = m_boundaryConditionA->lwRadBalance();
			IBK_ASSERT(inputValue != nullptr);
			// store the pair of adresses of result value and input
			// value inside pattern list
			depsThermalFluxesEner[0].insert(inputValue);
			// add moisture boundary conditions an left construction side (location A)
			inputValue = m_boundaryConditionA->vaporDiffusion();
			IBK_ASSERT(inputValue != nullptr);
			// store the pair of adresses of result value and input
			// value inside pattern list
			depsMoistureFluxesMoist[0].insert(inputValue);
			depsMoistureFluxesEner[0].insert(inputValue);

			inputValue = m_boundaryConditionA->vaporEnthalpy();
			IBK_ASSERT(inputValue != nullptr);
			// store the pair of adresses of result value and input
			// value inside pattern list
			depsThermalFluxesMoist[0].insert(inputValue);
			depsThermalFluxesEner[0].insert(inputValue);
		}

		if (m_boundaryConditionB != nullptr) {
			// last matrix entries  (boundary conditions at side B)
			// heat conduction flux
			const double *inputValue = m_boundaryConditionB->heatConduction();
			IBK_ASSERT(inputValue != nullptr);
			// store the pair of adresses of result value and input
			// value inside pattern list
			depsThermalFluxesEner[m_nElements].insert(inputValue);
			// absorbed short wave radiation flux
			inputValue = m_boundaryConditionB->swRadAbsorbed();
			IBK_ASSERT(inputValue != nullptr);
			// store the pair of adresses of result value and input
			// value inside pattern list
			depsThermalFluxesEner[m_nElements].insert(inputValue);
			// long wave radiation balance flux
			inputValue = m_boundaryConditionB->lwRadBalance();
			IBK_ASSERT(inputValue != nullptr);
			// store the pair of adresses of result value and input
			// value inside pattern list
			depsThermalFluxesEner[m_nElements].insert(inputValue);
			// add moisture boundary conditions an right construction side (location B)
			inputValue = m_boundaryConditionB->vaporDiffusion();
			IBK_ASSERT(inputValue != nullptr);
			// store the pair of adresses of result value and input
			// value inside pattern list
			depsMoistureFluxesMoist[m_nElements].insert(inputValue);
			depsMoistureFluxesEner[m_nElements].insert(inputValue);

			inputValue = m_boundaryConditionB->vaporEnthalpy();
			IBK_ASSERT(inputValue != nullptr);
			// store the pair of adresses of result value and input
			// value inside pattern list
			depsThermalFluxesEner[m_nElements].insert(inputValue);
			depsThermalFluxesMoist[m_nElements].insert(inputValue);
		}

		// add dependencies of heat and moisture fluxes
		// to detailed temperature and mositure states
		for (unsigned int i = 0; i < m_nElements + 1; ++i)
		{
			// store dependency to right extrapolation element
			if (i == 0) {
				// internal energy
				const double *inputValue = &m_states[WALL_MODEL::WallModel::STATE_ENERGY_DENSITY][i + 1];
				depsThermalFluxesEner[i].insert(inputValue);
				depsMoistureFluxesEner[i].insert(inputValue);
				// moisture
				inputValue = &m_states[WALL_MODEL::WallModel::STATE_MOISTURE_MASS_DENSITY][i + 1];
				depsThermalFluxesMoist[i].insert(inputValue);
				depsMoistureFluxesMoist[i].insert(inputValue);
			}
			// store dependency to left neighbor element
			else {
				// internal energy
				const double *inputValue = &m_states[WALL_MODEL::WallModel::STATE_ENERGY_DENSITY][i - 1];
				depsThermalFluxesEner[i].insert(inputValue);
				depsMoistureFluxesEner[i].insert(inputValue);
				// moisture
				inputValue = &m_states[WALL_MODEL::WallModel::STATE_MOISTURE_MASS_DENSITY][i - 1];
				depsThermalFluxesMoist[i].insert(inputValue);
				depsMoistureFluxesMoist[i].insert(inputValue);
			}
			// store dependency to left extrapolation element
			if (i == m_nElements) {
				const double *inputValue = &m_states[WALL_MODEL::WallModel::STATE_ENERGY_DENSITY][i - 2];
				depsThermalFluxesEner[i].insert(inputValue);
				depsMoistureFluxesEner[i].insert(inputValue);
				// moisture
				inputValue = &m_states[WALL_MODEL::WallModel::STATE_MOISTURE_MASS_DENSITY][i - 2];
				depsThermalFluxesMoist[i].insert(inputValue);
				depsMoistureFluxesMoist[i].insert(inputValue);
			}
			// store dependency to right neighbor element
			else
			{
				// internal energy
				const double *inputValue = &m_states[WALL_MODEL::WallModel::STATE_ENERGY_DENSITY][i];
				depsThermalFluxesEner[i].insert(inputValue);
				depsMoistureFluxesEner[i].insert(inputValue);
				// moisture
				inputValue = &m_states[WALL_MODEL::WallModel::STATE_MOISTURE_MASS_DENSITY][i];
				depsThermalFluxesMoist[i].insert(inputValue);
				depsMoistureFluxesMoist[i].insert(inputValue);
			}
		}

		// add dependencies of average divergences to heat and moisture fluxes
		for (unsigned int i = 0; i < m_nElements; ++i)
		{
			// fill thermal flux dependencies
			// left side heat flux
			const double *inputValue = &m_fluxes[WALL_MODEL::WallModel::FLUX_HEAT_TOTAL][i];
			depsYdotFluxes[2 * i].insert(inputValue);
			// right side heat flux
			inputValue = &m_fluxes[WALL_MODEL::WallModel::FLUX_HEAT_TOTAL][i + 1];
			depsYdotFluxes[2 * i].insert(inputValue);

			// left side moisture flux
			inputValue = &m_fluxes[WALL_MODEL::WallModel::FLUX_MOISTURE_TOTAL][i];
			depsYdotFluxes[2 * i + 1].insert(inputValue);
			// right side moisture flux
			inputValue = &m_fluxes[WALL_MODEL::WallModel::FLUX_MOISTURE_TOTAL][i + 1];
			depsYdotFluxes[2 * i + 1].insert(inputValue);
		}

		// add thermal dependencies between detailed and average states
		for (unsigned int i = 0; i < m_nElements; ++i)
		{
			// store dependency between internal energy and y
			const double *inputValue = &m_y[2 * i];
			depsEnerY[i].insert(inputValue);
			// store dependency between moisture density and y
			inputValue = &m_y[2 * i + 1];
			depsMoistY[i].insert(inputValue);
		}
		// now fill all dependencies in a sorted way:

		// ydot -> fluxes
		const double *valueRef = &m_ydot[0];
		for (unsigned int i = 0; i < nPrimaryStateResults(); ++i)
		{
			for (std::set<const double *>::const_iterator it = depsYdotFluxes[i].begin();
				it != depsYdotFluxes[i].end(); ++it)
			{
				resultInputValueReferences.push_back(
					std::make_pair(valueRef + i, *it));
			}
		}

		// thermal fluxes -> temp
		valueRef = &m_fluxes[WALL_MODEL::WallModel::FLUX_HEAT_TOTAL][0];
		for (unsigned int i = 0; i < m_nElements + 1; ++i)
		{
			for (std::set<const double *>::const_iterator it = depsThermalFluxesEner[i].begin();
				it != depsThermalFluxesEner[i].end(); ++it)
			{
				resultInputValueReferences.push_back(
					std::make_pair(valueRef + i, *it));
			}
			for (std::set<const double *>::const_iterator it = depsThermalFluxesMoist[i].begin();
				it != depsThermalFluxesMoist[i].end(); ++it)
			{
				resultInputValueReferences.push_back(
					std::make_pair(valueRef + i, *it));
			}
		}
		// moisture fluxes -> temp
		valueRef = &m_fluxes[WALL_MODEL::WallModel::FLUX_MOISTURE_TOTAL][0];
		for (unsigned int i = 0; i < m_nElements + 1; ++i)
		{
			for (std::set<const double *>::const_iterator it = depsMoistureFluxesEner[i].begin();
				it != depsMoistureFluxesEner[i].end(); ++it)
			{
				resultInputValueReferences.push_back(
					std::make_pair(valueRef + i, *it));
			}
			for (std::set<const double *>::const_iterator it = depsMoistureFluxesMoist[i].begin();
				it != depsMoistureFluxesMoist[i].end(); ++it)
			{
				resultInputValueReferences.push_back(
					std::make_pair(valueRef + i, *it));
			}
		}
		// energy -> y
		valueRef = &m_states[WALL_MODEL::WallModel::STATE_ENERGY_DENSITY][0];
		for (unsigned int i = 0; i < m_nElements; ++i)
		{
			for (std::set<const double *>::const_iterator it = depsEnerY[i].begin();
				it != depsEnerY[i].end(); ++it)
			{
				resultInputValueReferences.push_back(
					std::make_pair(valueRef + i, *it));
			}
		}
		// moisture -> y
		valueRef = &m_states[WALL_MODEL::WallModel::STATE_MOISTURE_MASS_DENSITY][0];
		for (unsigned int i = 0; i < m_nElements; ++i)
		{
			for (std::set<const double *>::const_iterator it = depsMoistY[i].begin();
				it != depsMoistY[i].end(); ++it)
			{
				resultInputValueReferences.push_back(
					std::make_pair(valueRef + i, *it));
			}
		}
	}
	else if (m_moistureCalculationMode == CM_Average) {

		// always enforce existence of coarse grid wall model
		IBK_ASSERT(m_coarseGridWallModel != nullptr);

		std::vector<std::set<const double *> > depsYdotFluxes(nPrimaryStateResults());
		std::vector<std::set<const double *> > depsThermalFluxesEner(m_nElements + 1);
		std::vector<std::set<const double *> > depsThermalFluxesMoist(m_nElements + 1);
		std::vector<std::set<const double *> > depsMoistureFluxesEner(m_coarseGridWallModel->m_nElements + 1);
		std::vector<std::set<const double *> > depsMoistureFluxesMoist(m_coarseGridWallModel->m_nElements + 1);
		std::vector<std::set<const double *> > depsEnerDetailedY(m_nElements);
		std::vector<std::set<const double *> > depsEnerAverageY(m_coarseGridWallModel->m_nElements);
		std::vector<std::set<const double *> > depsMoistDetailedY(m_nElements);
		std::vector<std::set<const double *> > depsMoistAverageY(m_coarseGridWallModel->m_nElements);

		if (m_boundaryConditionA != nullptr) {
			// heat conduction flux
			// first matrix entries (boundary conditions at side A)
			const double *inputValue = m_boundaryConditionA->heatConduction();
			IBK_ASSERT(inputValue != nullptr);
			// store the pair of adresses of result value and input
			// value inside pattern list
			depsThermalFluxesEner[0].insert(inputValue);
			// m oisture balance
			// absorbed short wave radiation flux
			inputValue = m_boundaryConditionA->swRadAbsorbed();
			IBK_ASSERT(inputValue != nullptr);
			// store the pair of adresses of result value and input
			// value inside pattern list
			depsThermalFluxesEner[0].insert(inputValue);
			// long wave radiation balance flux
			inputValue = m_boundaryConditionA->lwRadBalance();
			IBK_ASSERT(inputValue != nullptr);
			// store the pair of adresses of result value and input
			// value inside pattern list
			depsThermalFluxesEner[0].insert(inputValue);
			// add moisture boundary conditions an left construction side (location A)
			inputValue = m_boundaryConditionA->vaporDiffusion();
			IBK_ASSERT(inputValue != nullptr);
			// store the pair of adresses of result value and input
			// value inside pattern list
			depsMoistureFluxesMoist[0].insert(inputValue);
			depsMoistureFluxesEner[0].insert(inputValue);

			inputValue = m_boundaryConditionA->vaporEnthalpy();
			IBK_ASSERT(inputValue != nullptr);
			// store the pair of adresses of result value and input
			// value inside pattern list
			depsThermalFluxesMoist[0].insert(inputValue);
			depsThermalFluxesEner[0].insert(inputValue);
		}

		if (m_boundaryConditionB != nullptr) {
			// last matrix entries  (boundary conditions at side B)
			// heat conduction flux
			const double *inputValue = m_boundaryConditionB->heatConduction();
			IBK_ASSERT(inputValue != nullptr);
			// store the pair of adresses of result value and input
			// value inside pattern list
			depsThermalFluxesEner[m_nElements].insert(inputValue);
			// absorbed short wave radiation flux
			inputValue = m_boundaryConditionB->swRadAbsorbed();
			IBK_ASSERT(inputValue != nullptr);
			// store the pair of adresses of result value and input
			// value inside pattern list
			depsThermalFluxesEner[m_nElements].insert(inputValue);
			// long wave radiation balance flux
			inputValue = m_boundaryConditionB->lwRadBalance();
			IBK_ASSERT(inputValue != nullptr);
			// store the pair of adresses of result value and input
			// value inside pattern list
			depsThermalFluxesEner[m_nElements].insert(inputValue);
			// add moisture boundary conditions an right construction side (location B)
			inputValue = m_boundaryConditionB->vaporDiffusion();
			IBK_ASSERT(inputValue != nullptr);
			// store the pair of adresses of result value and input
			// value inside pattern list
			depsMoistureFluxesMoist[m_coarseGridWallModel->m_nElements].insert(inputValue);
			depsMoistureFluxesEner[m_coarseGridWallModel->m_nElements].insert(inputValue);

			inputValue = m_boundaryConditionB->vaporEnthalpy();
			IBK_ASSERT(inputValue != nullptr);
			// store the pair of adresses of result value and input
			// value inside pattern list
			depsThermalFluxesEner[m_nElements].insert(inputValue);
			depsThermalFluxesMoist[m_nElements].insert(inputValue);
		}

		// add dependencies of heat fluxes
		// to detailed temperature and moisture states
		for (unsigned int i = 0; i < m_nElements + 1; ++i)
		{
			// store dependency to right extrapolation element
			if (i == 0) {
				// internal energy
				const double* inputValue = &m_states[WALL_MODEL::WallModel::STATE_ENERGY_DENSITY][i + 1];
				depsThermalFluxesEner[i].insert(inputValue);
				// moisture
				inputValue = &m_states[WALL_MODEL::WallModel::STATE_MOISTURE_MASS_DENSITY][i + 1];
				depsThermalFluxesMoist[i].insert(inputValue);
			}
			// store dependency to left neighbor element
			else {
				// internal energy
				const double* inputValue = &m_states[WALL_MODEL::WallModel::STATE_ENERGY_DENSITY][i - 1];
				depsThermalFluxesEner[i].insert(inputValue);
				// moisture
				inputValue = &m_states[WALL_MODEL::WallModel::STATE_MOISTURE_MASS_DENSITY][i - 1];
				depsThermalFluxesMoist[i].insert(inputValue);
			}
			// store dependency to left extrapolation element
			if (i == m_nElements) {
				const double* inputValue = &m_states[WALL_MODEL::WallModel::STATE_ENERGY_DENSITY][i - 2];
				depsThermalFluxesEner[i].insert(inputValue);
				// moisture
				inputValue = &m_states[WALL_MODEL::WallModel::STATE_MOISTURE_MASS_DENSITY][i - 2];
				depsThermalFluxesMoist[i].insert(inputValue);
			}
			// store dependency to right neighbor element
			else
			{
				// internal energy
				const double* inputValue = &m_states[WALL_MODEL::WallModel::STATE_ENERGY_DENSITY][i];
				depsThermalFluxesEner[i].insert(inputValue);
				// moisture
				inputValue = &m_states[WALL_MODEL::WallModel::STATE_MOISTURE_MASS_DENSITY][i];
				depsThermalFluxesMoist[i].insert(inputValue);
			}
		}

#ifdef FinePv
		// to detailed temperature and mositure states
		for (unsigned int i = 0; i < m_coarseGridWallModel->m_nElements + 1; ++i)
		{
			// store dependency to right extrapolation element
			if (i == 0) {
				// internal energy
				const double* inputValue = &m_coarseGridWallModel->m_states[WALL_MODEL::WallModel::STATE_ENERGY_DENSITY][i + 1];
				depsMoistureFluxesEner[i].insert(inputValue);
				// moisture
				inputValue = &m_states[WALL_MODEL::WallModel::STATE_MOISTURE_MASS_DENSITY][m_averageElementOffset[i] + 1];
				depsMoistureFluxesMoist[i].insert(inputValue);
			}
			// store dependency to left neighbor element
			else {
				// internal energy
				const double* inputValue = &m_coarseGridWallModel->m_states[WALL_MODEL::WallModel::STATE_ENERGY_DENSITY][i - 1];
				depsMoistureFluxesEner[i].insert(inputValue);
				// moisture
				inputValue = &m_states[WALL_MODEL::WallModel::STATE_MOISTURE_MASS_DENSITY][m_averageElementOffset[i] - 1];
				depsMoistureFluxesMoist[i].insert(inputValue);
			}
			// store dependency to left extrapolation element
			if (i == m_coarseGridWallModel->m_nElements) {
				const double* inputValue = &m_coarseGridWallModel->m_states[WALL_MODEL::WallModel::STATE_ENERGY_DENSITY][i - 2];
				depsMoistureFluxesEner[i].insert(inputValue);
				// moisture
				inputValue = &m_states[WALL_MODEL::WallModel::STATE_MOISTURE_MASS_DENSITY][m_averageElementOffset[i] - 2];
				depsMoistureFluxesMoist[i].insert(inputValue);
			}
			// store dependency to right neighbor element
			else
			{
				// internal energy
				const double* inputValue = &m_coarseGridWallModel->m_states[WALL_MODEL::WallModel::STATE_ENERGY_DENSITY][i];
				depsMoistureFluxesEner[i].insert(inputValue);
				// moisture
				inputValue = &m_states[WALL_MODEL::WallModel::STATE_MOISTURE_MASS_DENSITY][m_averageElementOffset[i]];
				depsMoistureFluxesMoist[i].insert(inputValue);
			}
		}
#else
		// to detailed temperature and mositure states
		for (unsigned int i = 0; i < m_coarseGridWallModel->m_nElements + 1; ++i)
		{
			// store dependency to right extrapolation element
			if (i == 0) {
				// internal energy
				const double* inputValue = &m_coarseGridWallModel->m_states[WALL_MODEL::WallModel::STATE_ENERGY_DENSITY][i + 1];
				depsMoistureFluxesEner[i].insert(inputValue);
				// moisture
				inputValue = &m_coarseGridWallModel->m_states[WALL_MODEL::WallModel::STATE_MOISTURE_MASS_DENSITY][i + 1];
				depsMoistureFluxesMoist[i].insert(inputValue);
			}
			// store dependency to left neighbor element
			else {
				// internal energy
				const double* inputValue = &m_coarseGridWallModel->m_states[WALL_MODEL::WallModel::STATE_ENERGY_DENSITY][i - 1];
				depsMoistureFluxesEner[i].insert(inputValue);
				// moisture
				inputValue = &m_coarseGridWallModel->m_states[WALL_MODEL::WallModel::STATE_MOISTURE_MASS_DENSITY][i - 1];
				depsMoistureFluxesMoist[i].insert(inputValue);
			}
			// store dependency to left extrapolation element
			if (i == m_coarseGridWallModel->m_nElements) {
				const double* inputValue = &m_coarseGridWallModel->m_states[WALL_MODEL::WallModel::STATE_ENERGY_DENSITY][i - 2];
				depsMoistureFluxesEner[i].insert(inputValue);
				// moisture
				inputValue = &m_coarseGridWallModel->m_states[WALL_MODEL::WallModel::STATE_MOISTURE_MASS_DENSITY][i - 2];
				depsMoistureFluxesMoist[i].insert(inputValue);
			}
			// store dependency to right neighbor element
			else
			{
				// internal energy
				const double* inputValue = &m_coarseGridWallModel->m_states[WALL_MODEL::WallModel::STATE_ENERGY_DENSITY][i];
				depsMoistureFluxesEner[i].insert(inputValue);
				// moisture
				inputValue = &m_coarseGridWallModel->m_states[WALL_MODEL::WallModel::STATE_MOISTURE_MASS_DENSITY][i];
				depsMoistureFluxesMoist[i].insert(inputValue);
			}
		}
#endif

		// add dependencies of average divergences to heat and moisture fluxes
		for (unsigned int i = 0; i < m_nElements; ++i)
		{
			// fill thermal flux dependencies
			// left side heat flux
			const double* inputValue = &m_fluxes[WALL_MODEL::WallModel::FLUX_HEAT_TOTAL][i];
			depsYdotFluxes[m_energyBalanceEquation[i]].insert(inputValue);
			// right side heat flux
			inputValue = &m_fluxes[WALL_MODEL::WallModel::FLUX_HEAT_TOTAL][i + 1];
			depsYdotFluxes[m_energyBalanceEquation[i]].insert(inputValue);
		}

		// add dependencies of average divergences to heat and moisture fluxes
		for (unsigned int i = 0; i < m_coarseGridWallModel->m_nElements; ++i)
		{
			// left side moisture flux
			const double* inputValue = &m_coarseGridWallModel->m_fluxes[WALL_MODEL::WallModel::FLUX_MOISTURE_TOTAL][i];
			depsYdotFluxes[m_moistureBalanceEquation[i]].insert(inputValue);
			// right side moisture flux
			inputValue = &m_coarseGridWallModel->m_fluxes[WALL_MODEL::WallModel::FLUX_MOISTURE_TOTAL][i + 1];
			depsYdotFluxes[m_moistureBalanceEquation[i]].insert(inputValue);
		}

		// add thermal dependencies between detailed and average states
		for (unsigned int i = 0; i < m_nElements; ++i)
		{
			// store dependency between temp and y
			const double* inputValue = &m_y[m_energyBalanceEquation[i]];
			depsEnerDetailedY[i].insert(inputValue);
		}

		for (unsigned int i = 0; i < m_coarseGridWallModel->m_nElements; ++i) {

			const double* inputValue = &m_y[m_moistureBalanceEquation[i]];
			depsMoistAverageY[i].insert(inputValue);
			// set dependencies to all thermalstates
			// add moisture internal conditions
			unsigned int jStart = m_averageElementOffset[i];
			unsigned int jEnd = m_averageElementOffset[i + 1];

			for (unsigned int j = jStart; j < jEnd; ++j) {
				// states reconstruction from a single value
				depsMoistDetailedY[j].insert(inputValue);
			}
			for (unsigned int j = jStart; j < jEnd; ++j) {
				inputValue = &m_y[m_energyBalanceEquation[j]];
				// states reconstruction from a single value
				depsEnerAverageY[i].insert(inputValue);
			}
		}

		// now fill all dependencies in a sorted way:

		// ydotAverage -> fluxes
		const double *valueRef = &m_ydotAverage[0];
		for (unsigned int i = 0; i < nPrimaryStateResults(); ++i)
		{
			for (std::set<const double *>::const_iterator it = depsYdotFluxes[i].begin();
				it != depsYdotFluxes[i].end(); ++it)
			{
				resultInputValueReferences.push_back(
					std::make_pair(valueRef + i, *it));
			}
		}

		// add diagonal (storage term)
		valueRef = &m_ydotAverage[0];
		const double* inputValue = &m_y[0];
		for (unsigned int i = 0; i < nPrimaryStateResults(); ++i)
		{
			resultInputValueReferences.push_back(
					std::make_pair(valueRef + i, inputValue + i));
		}

		// thermal fluxes -> temp
		valueRef = &m_fluxes[WALL_MODEL::WallModel::FLUX_HEAT_TOTAL][0];
		for (unsigned int i = 0; i < m_nElements + 1; ++i)
		{
			for (std::set<const double *>::const_iterator it = depsThermalFluxesEner[i].begin();
				it != depsThermalFluxesEner[i].end(); ++it)
			{
				resultInputValueReferences.push_back(
					std::make_pair(valueRef + i, *it));
			}
			for (std::set<const double *>::const_iterator it = depsThermalFluxesMoist[i].begin();
				it != depsThermalFluxesMoist[i].end(); ++it)
			{
				resultInputValueReferences.push_back(
					std::make_pair(valueRef + i, *it));
			}
		}

		// moisture fluxes -> temp
		valueRef = &m_coarseGridWallModel->m_fluxes[WALL_MODEL::WallModel::FLUX_MOISTURE_TOTAL][0];
		for (unsigned int i = 0; i < m_coarseGridWallModel->m_nElements + 1; ++i)
		{
			for (std::set<const double *>::const_iterator it = depsMoistureFluxesEner[i].begin();
				it != depsMoistureFluxesEner[i].end(); ++it)
			{
				resultInputValueReferences.push_back(
					std::make_pair(valueRef + i, *it));
			}
			for (std::set<const double *>::const_iterator it = depsMoistureFluxesMoist[i].begin();
				it != depsMoistureFluxesMoist[i].end(); ++it)
			{
				resultInputValueReferences.push_back(
					std::make_pair(valueRef + i, *it));
			}
		}

		// energy -> y
		valueRef = &m_states[WALL_MODEL::WallModel::STATE_ENERGY_DENSITY][0];
		for (unsigned int i = 0; i < m_nElements; ++i)
		{
			for (std::set<const double *>::const_iterator it = depsEnerDetailedY[i].begin();
				it != depsEnerDetailedY[i].end(); ++it)
			{
				resultInputValueReferences.push_back(
					std::make_pair(valueRef + i, *it));
			}
		}
		valueRef = &m_coarseGridWallModel->m_states[WALL_MODEL::WallModel::STATE_ENERGY_DENSITY][0];
		for (unsigned int i = 0; i < m_coarseGridWallModel->m_nElements; ++i)
		{
			for (std::set<const double *>::const_iterator it = depsEnerAverageY[i].begin();
				it != depsEnerAverageY[i].end(); ++it)
			{
				resultInputValueReferences.push_back(
					std::make_pair(valueRef + i, *it));
			}
		}

		// moisture -> y
		valueRef = &m_states[WALL_MODEL::WallModel::STATE_MOISTURE_MASS_DENSITY][0];
		for (unsigned int i = 0; i < m_nElements; ++i)
		{
			for (std::set<const double *>::const_iterator it = depsMoistDetailedY[i].begin();
				it != depsMoistDetailedY[i].end(); ++it)
			{
				resultInputValueReferences.push_back(
					std::make_pair(valueRef + i, *it));
			}
		}

		valueRef = &m_coarseGridWallModel->m_states[WALL_MODEL::WallModel::STATE_MOISTURE_MASS_DENSITY][0];
		for (unsigned int i = 0; i < m_coarseGridWallModel->m_nElements; ++i)
		{
			for (std::set<const double *>::const_iterator it = depsMoistAverageY[i].begin();
				it != depsMoistAverageY[i].end(); ++it)
			{
				resultInputValueReferences.push_back(
					std::make_pair(valueRef + i, *it));
			}
		}
	}
	else {
		std::vector<std::set<const double *> > deps(nPrimaryStateResults());
		IBK_ASSERT(m_nElements * m_nBalanceEquations == m_n);
		// heat conduction flux
		for (unsigned int m = 0; m < m_nBalanceEquations; ++m) {

			if (m_boundaryConditionA != nullptr) {
				// first matrix entries (boundary conditions at side A)
				const double *inputValue = m_boundaryConditionA->heatConduction();
				IBK_ASSERT(inputValue != nullptr);
				// store the pair of adresses of result value and input
				// value inside pattern list
				deps[m].insert(inputValue);
				deps[m_nBalanceEquations + m].insert(inputValue);
				// absorbed short wave radiation flux
				inputValue = m_boundaryConditionA->swRadAbsorbed();
				IBK_ASSERT(inputValue != nullptr);
				// store the pair of adresses of result value and input
				// value inside pattern list
				deps[m].insert(inputValue);
				deps[m_nBalanceEquations + m].insert(inputValue);
				// long wave radiation balance flux
				inputValue = m_boundaryConditionA->lwRadBalance();
				IBK_ASSERT(inputValue != nullptr);
				// store the pair of adresses of result value and input
				// value inside pattern list
				deps[m].insert(inputValue);
				deps[m_nBalanceEquations + m].insert(inputValue);
			}

			if (m_boundaryConditionB != nullptr) {
				// last matrix entries  (boundary conditions at side B)
			// heat conduction flux
				const double *inputValue = m_boundaryConditionB->heatConduction();
				IBK_ASSERT(inputValue != nullptr);
				// store the pair of adresses of result value and input
				// value inside pattern list
				deps[(m_nElements - 2) * m_nBalanceEquations + m].insert(inputValue);
				deps[(m_nElements - 1) * m_nBalanceEquations + m].insert(inputValue);
				// absorbed short wave radiation flux
				inputValue = m_boundaryConditionB->swRadAbsorbed();
				IBK_ASSERT(inputValue != nullptr);
				// store the pair of adresses of result value and input
				// value inside pattern list
				deps[(m_nElements - 2) * m_nBalanceEquations + m].insert(inputValue);
				deps[(m_nElements - 1) * m_nBalanceEquations + m].insert(inputValue);
				// long wave radiation balance flux
				inputValue = m_boundaryConditionB->lwRadBalance();
				IBK_ASSERT(inputValue != nullptr);
				// store the pair of adresses of result value and input
				// value inside pattern list
				deps[(m_nElements - 2) * m_nBalanceEquations + m].insert(inputValue);
				deps[(m_nElements - 1) * m_nBalanceEquations + m].insert(inputValue);
			}

			// special case: combined hygrothermal simulation
			if (m_moistureCalculationMode != CM_None) {

				if (m_boundaryConditionA != nullptr) {
					// add moisture boundary conditions an left construction side (location A)
					const double* inputValue = m_boundaryConditionA->vaporDiffusion();
					IBK_ASSERT(inputValue != nullptr);
					// store the pair of adresses of result value and input
					// value inside pattern list
					deps[m].insert(inputValue);
					deps[m_nBalanceEquations + m].insert(inputValue);
					inputValue = m_boundaryConditionA->vaporEnthalpy();
					IBK_ASSERT(inputValue != nullptr);
					// store the pair of adresses of result value and input
					// value inside pattern list
					deps[m].insert(inputValue);
					deps[m_nBalanceEquations + m].insert(inputValue);
				}

				if (m_boundaryConditionB != nullptr) {
					// add moisture boundary conditions an right construction side (location B)
					const double* inputValue = m_boundaryConditionB->vaporDiffusion();
					IBK_ASSERT(inputValue != nullptr);
					// store the pair of adresses of result value and input
					// value inside pattern list
					deps[(m_nElements - 2) * m_nBalanceEquations + m].insert(inputValue);
					deps[(m_nElements - 1) * m_nBalanceEquations + m].insert(inputValue);
					inputValue = m_boundaryConditionB->vaporEnthalpy();
					IBK_ASSERT(inputValue != nullptr);
					// store the pair of adresses of result value and input
					// value inside pattern list
					deps[(m_nElements - 2) * m_nBalanceEquations + m].insert(inputValue);
					deps[(m_nElements - 1) * m_nBalanceEquations + m].insert(inputValue);
				}
			}

			for (unsigned int n = 0; n < m_nBalanceEquations; ++n)
			{
				// connect values of ydot to y
				for (unsigned int i = 0; i < m_nElements; ++i)
				{
					// store dependency to current element
					const double* inputValue = &m_y[i * m_nBalanceEquations + n];
					deps[i * m_nBalanceEquations + m].insert(inputValue);
					// store dependency to left neighbor element
					if (i > 0)
					{
						inputValue = &m_y[(i - 1)  * m_nBalanceEquations + n];
						deps[i * m_nBalanceEquations + m].insert(inputValue);
					}
					// store dependency to right neighbor element
					if (i < m_nElements - 1)
					{
						inputValue = &m_y[(i + 1)  * m_nBalanceEquations + n];
						deps[i * m_nBalanceEquations + m].insert(inputValue);
					}
				}
			}
		}
		// now fill all dependencies in a sorted way
		const double *valueRef = &m_ydot[0];

		for (unsigned int i = 0; i < nPrimaryStateResults(); ++i)
		{
			for (std::set<const double *>::const_iterator it = deps[i].begin();
				it != deps[i].end(); ++it)
			{
				resultInputValueReferences.push_back(
					std::make_pair(valueRef + i, *it));
			}
		}
	}


	if(!m_activeLayerIndices.empty()) {

		// insert dependencies to active element heating models
		std::set<unsigned int>::const_iterator indexIt = m_activeLayerIndices.begin();
		std::vector<const double *>::const_iterator fieldFluxIt = inputValueRefs(InputRef_FieldFlux);
		for (; indexIt != m_activeLayerIndices.end(); ++indexIt, ++fieldFluxIt)
		{
			unsigned int activeLayerIdx = *indexIt;
			IBK_ASSERT(activeLayerIdx < m_layerSources[SOURCE_HEAT_PRODUCTION_RATE].size());
			// retrieve input value pointer
			IBK_ASSERT(fieldFluxIt != inputValueRefs().end());
			const double *inputValue = *fieldFluxIt;
			IBK_ASSERT(inputValue != nullptr);
			// retreive value references: elements of the currentlayer
			IBK_ASSERT(activeLayerIdx < m_materialLayerElementOffset.size());
			// rtereive minimum and maximumelement number
			unsigned int iMin = m_materialLayerElementOffset[activeLayerIdx];
			unsigned int iMax = m_materialLayerElementOffset[activeLayerIdx + 1];

			// set dependency between input model and time derivative of the source elements
			for (unsigned int i = iMin; i < iMax; ++i) {
				const double *valueRef = nullptr;
				if (m_moistureCalculationMode == CM_Average)
					valueRef = &m_ydotAverage[m_energyBalanceEquation[i]];
				else
					valueRef = &m_ydot[i * m_nBalanceEquations];

				resultInputValueReferences.push_back(
					std::make_pair(valueRef, inputValue));
			}
		}
	}

}

int ConstructionSolverModel::setTime(double t) {
	// Copy time point.
	m_t = t;
	// signal success
	return 0;
}


int ConstructionSolverModel::update() {

	// *** update field conditions ***
	if(!m_activeLayerIndices.empty()) {
		std::set<unsigned int>::const_iterator indexIt = m_activeLayerIndices.begin();
		std::vector<const double *>::const_iterator fieldFluxIt = inputValueRefs(InputRef_FieldFlux);
		std::vector<double>::iterator thermalLoadIt = m_vectorValuedResults[VVR_ThermalLoad].begin();

		unsigned int k = 0;
		for(; indexIt != m_activeLayerIndices.end(); ++indexIt, ++k, ++fieldFluxIt)
		{
			IBK_ASSERT(thermalLoadIt != m_vectorValuedResults[VVR_ThermalLoad].end());

			unsigned int activeLayerIdx = *indexIt;
			IBK_ASSERT (activeLayerIdx < m_n);

			while (activeLayerIdx > k) {
				// switch values until next active layer
				// store as result
				IBK_ASSERT(thermalLoadIt != m_vectorValuedResults[VVR_ThermalLoad].end());
				++thermalLoadIt;
				++k;
			}

			IBK_ASSERT(activeLayerIdx < m_layerSources[SOURCE_HEAT_PRODUCTION_RATE].size() );
			// retrieve input value pointer
			IBK_ASSERT(fieldFluxIt != inputValueRefs().end());
			const double *fieldFluxRef = *fieldFluxIt;
			IBK_ASSERT(fieldFluxRef != nullptr);
			const double fieldFlux = *fieldFluxRef;
			// translate into a field condition
			m_layerSources[SOURCE_HEAT_PRODUCTION_RATE][activeLayerIdx] = fieldFlux/m_wallArea;
			// store as result
			IBK_ASSERT(thermalLoadIt != m_vectorValuedResults[VVR_ThermalLoad].end());
			*thermalLoadIt = fieldFlux;
		}
		// copy values into m_sources vector
		updateSources();
	}

	// *** update boundary conditions ***

	// retrieve left side (A) and right side (B) boundary fluxes
	const double * qHeatCond_A_Ptr = nullptr;
	const double * qHeatCond_B_Ptr = nullptr;

	if (m_boundaryConditionA != nullptr)
		qHeatCond_A_Ptr = m_boundaryConditionA->heatConduction();
	if (m_boundaryConditionB != nullptr)
		qHeatCond_B_Ptr = m_boundaryConditionB->heatConduction();

	if (qHeatCond_A_Ptr != nullptr) {
		// specify heat conduction flux on the wall surface
		m_fluxes[WALL_MODEL::WallModel::FLUX_HEAT_CONDUCTION][0]  = *qHeatCond_A_Ptr;
	}
	else {
		m_fluxes[WALL_MODEL::WallModel::FLUX_HEAT_CONDUCTION][0]  = 0.0;
	}

	if (qHeatCond_B_Ptr != nullptr) {
		// specify heat conduction flux on the wall surface and revert heat flux direction
		// (positive heat flux means heat loss to the environment)
		m_fluxes[WALL_MODEL::WallModel::FLUX_HEAT_CONDUCTION][m_nElements]  = -*qHeatCond_B_Ptr;
	}
	else {
		m_fluxes[WALL_MODEL::WallModel::FLUX_HEAT_CONDUCTION][m_nElements]  = 0.0;
	}

	// same for short wave radiation
	const double * qSWRad_A_Ptr = nullptr;
	const double * qSWRad_B_Ptr = nullptr;

	if (m_boundaryConditionA != nullptr)
		qSWRad_A_Ptr = m_boundaryConditionA->swRadAbsorbed();
	if (m_boundaryConditionB != nullptr)
		qSWRad_B_Ptr = m_boundaryConditionB->swRadAbsorbed();

	if (qSWRad_A_Ptr != nullptr) {
		// specify radiation heat flux
		m_fluxes[WALL_MODEL::WallModel::FLUX_SW_RADIATION][0] = *qSWRad_A_Ptr;
	}
	else {
		m_fluxes[WALL_MODEL::WallModel::FLUX_LW_RADIATION][0] = 0.0;
	}

	if (qSWRad_B_Ptr != nullptr) {
		// specify radiation heat flux on the wall surface and revert heat flux direction
		// (positive heat flux means heat loss to the environment)
		m_fluxes[WALL_MODEL::WallModel::FLUX_SW_RADIATION][m_nElements] = -*qSWRad_B_Ptr;
	}
	else {
		m_fluxes[WALL_MODEL::WallModel::FLUX_SW_RADIATION][m_nElements] = 0;
	}

	// same for long wave radiation
	const double * qLWRad_A_Ptr = nullptr;
	const double * qLWRad_B_Ptr = nullptr;

	if (m_boundaryConditionA != nullptr)
		qLWRad_A_Ptr = m_boundaryConditionA->lwRadBalance();
	if (m_boundaryConditionB != nullptr)
		qLWRad_B_Ptr = m_boundaryConditionB->lwRadBalance();

	if (qLWRad_A_Ptr != nullptr) {
		// specify radiation heat flux
		m_fluxes[WALL_MODEL::WallModel::FLUX_LW_RADIATION][0] = *qLWRad_A_Ptr;
	}
	else {
		m_fluxes[WALL_MODEL::WallModel::FLUX_LW_RADIATION][0] = 0.0;
	}

	if (qLWRad_B_Ptr != nullptr) {
		// specify radiation heat flux on the wall surface and revert heat flux direction
		// (positive heat flux means heat loss to the environment)
		m_fluxes[WALL_MODEL::WallModel::FLUX_LW_RADIATION][m_nElements] = -*qLWRad_B_Ptr;
	}
	else {
		m_fluxes[WALL_MODEL::WallModel::FLUX_LW_RADIATION][m_nElements] = 0.0;
	}

	// special case: combined hygrothermal simulation
	switch (m_moistureCalculationMode) {
		case CM_Detailed : {
			// set vapor diffusion boundray condition
			const double * vaporDiff_A_Ptr = nullptr;
			const double * vaporDiff_B_Ptr = nullptr;

			if (m_boundaryConditionA != nullptr)
				vaporDiff_A_Ptr = m_boundaryConditionA->vaporDiffusion();
			if (m_boundaryConditionB != nullptr)
				vaporDiff_B_Ptr = m_boundaryConditionB->vaporDiffusion();

			if (vaporDiff_A_Ptr != nullptr) {
				// specify vapor diffusion flux
				m_fluxes[WALL_MODEL::WallModel::FLUX_VAPOR_DIFFUSION][0] = *vaporDiff_A_Ptr;
			}
			else {
				m_fluxes[WALL_MODEL::WallModel::FLUX_VAPOR_DIFFUSION][0] = 0.0;
			}

			if (vaporDiff_B_Ptr != nullptr) {
				// specify vapor diffusion flux on the wall surface and revert flux direction
				// (positive flux means moisture dissipation to the environment)
				m_fluxes[WALL_MODEL::WallModel::FLUX_VAPOR_DIFFUSION][m_nElements] = -*vaporDiff_B_Ptr;
			}
			else {
				m_fluxes[WALL_MODEL::WallModel::FLUX_VAPOR_DIFFUSION][m_nElements] = 0.0;
			}

			// set vapor diffusion enthalpy flux at boundaries
			const double * vaporDiffEnth_A_Ptr = nullptr;
			const double * vaporDiffEnth_B_Ptr = nullptr;

			if (m_boundaryConditionA != nullptr)
				vaporDiffEnth_A_Ptr = m_boundaryConditionA->vaporEnthalpy();
			if (m_boundaryConditionB != nullptr)
				vaporDiffEnth_B_Ptr = m_boundaryConditionB->vaporEnthalpy();

			if (vaporDiff_A_Ptr != nullptr) {
				// specify vapor diffusion flux
				m_fluxes[WALL_MODEL::WallModel::FLUX_VAPOR_DIFFUSION_ENTHALPY][0] = *vaporDiffEnth_A_Ptr;
			}
			else {
				m_fluxes[WALL_MODEL::WallModel::FLUX_VAPOR_DIFFUSION_ENTHALPY][0] = 0.0;
			}

			if (vaporDiff_B_Ptr != nullptr) {
				// specify vapor diffusion flux on the wall surface and revert flux direction
				// (positive flux means moisture dissipation to the environment)
				m_fluxes[WALL_MODEL::WallModel::FLUX_VAPOR_DIFFUSION_ENTHALPY][m_nElements] = -*vaporDiffEnth_B_Ptr;
			}
			else {
				m_fluxes[WALL_MODEL::WallModel::FLUX_VAPOR_DIFFUSION_ENTHALPY][m_nElements] = 0.0;
			}
			// complete flux calculation with the new boundary values
			updateFluxes();
		} break;

		case CM_Average : {
			// update average fluxes
			IBK_ASSERT(m_coarseGridWallModel != nullptr);

			// set vapor diffusion boundray condition
			const double * vaporDiff_A_Ptr = nullptr;
			const double * vaporDiff_B_Ptr = nullptr;

			if (m_boundaryConditionA != nullptr)
				vaporDiff_A_Ptr = m_boundaryConditionA->vaporDiffusion();
			if (m_boundaryConditionB != nullptr)
				vaporDiff_B_Ptr = m_boundaryConditionB->vaporDiffusion();

			if (vaporDiff_A_Ptr != nullptr) {
				// specify vapor diffusion flux
				m_coarseGridWallModel->m_fluxes[WALL_MODEL::WallModel::FLUX_VAPOR_DIFFUSION][0] = *vaporDiff_A_Ptr;
			}
			else {
				m_coarseGridWallModel->m_fluxes[WALL_MODEL::WallModel::FLUX_VAPOR_DIFFUSION][0] = 0.0;
			}

			if (vaporDiff_B_Ptr != nullptr) {
				// specify vapor diffusion flux on the wall surface and revert flux direction
				// (positive flux means moisture dissipation to the environment)
				m_coarseGridWallModel->m_fluxes[WALL_MODEL::WallModel::FLUX_VAPOR_DIFFUSION][m_coarseGridWallModel->m_nElements] = -*vaporDiff_B_Ptr;
			}
			else {
				m_coarseGridWallModel->m_fluxes[WALL_MODEL::WallModel::FLUX_VAPOR_DIFFUSION][m_coarseGridWallModel->m_nElements] = 0.0;
			}

			// set vapor diffusion enthalpy flux at boundaries: detailed thermnal balance
			const double * vaporDiffEnth_A_Ptr = nullptr;
			const double * vaporDiffEnth_B_Ptr = nullptr;

			if (m_boundaryConditionA != nullptr)
				vaporDiffEnth_A_Ptr = m_boundaryConditionA->vaporEnthalpy();
			if (m_boundaryConditionB != nullptr)
				vaporDiffEnth_B_Ptr = m_boundaryConditionB->vaporEnthalpy();

			if (vaporDiff_A_Ptr != nullptr) {
				// specify vapor diffusion flux
				m_fluxes[WALL_MODEL::WallModel::FLUX_VAPOR_DIFFUSION_ENTHALPY][0] = *vaporDiffEnth_A_Ptr;
			}
			else {
				m_fluxes[WALL_MODEL::WallModel::FLUX_VAPOR_DIFFUSION_ENTHALPY][0] = 0.0;
			}

			if (vaporDiff_B_Ptr != nullptr) {
				// specify vapor diffusion flux on the wall surface and revert flux direction
				// (positive flux means moisture dissipation to the environment)
				m_fluxes[WALL_MODEL::WallModel::FLUX_VAPOR_DIFFUSION_ENTHALPY][m_nElements] = -*vaporDiffEnth_B_Ptr;
			}
			else {
				m_fluxes[WALL_MODEL::WallModel::FLUX_VAPOR_DIFFUSION_ENTHALPY][m_nElements] = 0.0;
			}

#ifdef CalculateVaporPressureAtFineGrid
			if (vaporDiff_A_Ptr != nullptr) {
				// specify vapor diffusion flux
				m_fluxes[WALL_MODEL::WallModel::FLUX_VAPOR_DIFFUSION][0] = *vaporDiff_A_Ptr;
			}
			else {
				m_fluxes[WALL_MODEL::WallModel::FLUX_VAPOR_DIFFUSION][0] = 0.0;
			}
			if (vaporDiff_B_Ptr != nullptr) {
				// specify vapor diffusion flux on the wall surface and revert flux direction
				// (positive flux means moisture dissipation to the environment)
				m_fluxes[WALL_MODEL::WallModel::FLUX_VAPOR_DIFFUSION][m_nElements] = -*vaporDiff_B_Ptr;
			}
			else {
				m_fluxes[WALL_MODEL::WallModel::FLUX_VAPOR_DIFFUSION][m_nElements] = 0.0;
			}
#endif
			// complete flux calculation with the new boundary values
			updateFluxes();

#ifdef CalculateVaporPressureAtFineGrid
			// overwrite fluxes
			averageFluxFromDetailedFlux(&m_coarseGridWallModel->m_fluxes[FLUX_MOISTURE_TOTAL][0], &m_fluxes[FLUX_MOISTURE_TOTAL][0]);
#else
			// calculate all fluxes related to vapor diffusion
			m_coarseGridWallModel->updateFluxes();
#endif
			// calculate divergences for vapor tranposrt
			m_coarseGridWallModel->updateDivergences();



		} break;

		case CM_None : {
			//updateFluxes();
			updateFluxesThermal();
		} break;

	}
	// calculate divergences with the new heat fluxes
	updateDivergences();

	// signal success
	return 0;
}


unsigned int ConstructionSolverModel::decodeVectorValuedQuantityIndex(const std::string & vectorQuantity, const std::string & keyString) const
{
	const char * const FUNC_ID = "[ConstructionSolverModel::decodeVectorValuedQuantityIndex]";
	// only quantities 'FieldFlux' and 'HeatingsLoad' are allowed
	if(vectorQuantity != "FieldFlux" && vectorQuantity != "HeatingsLoad")
	{
		throw IBK::Exception(IBK::FormatString("Error decoding index: quantity "
			"%1 is not of vector valued type!")
			.arg(vectorQuantity), FUNC_ID);
	}
	// Construct a vector index element. This class contains
	// the translation utility between an index value and an identification
	// string.
	VectorValuedQuantityIndex indexKey;
	try {
		indexKey.set(keyString);
	}
	catch(IBK::Exception &ex) {
		throw IBK::Exception(ex, IBK::FormatString("Error extracting the vector index of quantity %1 from keyString '%2'.")
			.arg(vectorQuantity).arg(keyString), FUNC_ID);
	}
	// Retreive the index value from the identification string.
	if (indexKey.m_keyType == VectorValuedQuantityIndex::IK_Index) {
		return indexKey.m_keyValue;
	}
	throw IBK::Exception( IBK::FormatString("Error extracting the vector index of quantity %1 from keyString '%2'. Expected index=<number> format, "
		"other vector indices are not supported.")
		.arg(vectorQuantity).arg(keyString), FUNC_ID);
}

const std::string ConstructionSolverModel::encodeVectorValuedQuantityIndex(const std::string & vectorQuantity, unsigned int keyValue) const {
	const char * const FUNC_ID = "[ConstructionSolverModel::encodeVectorValuedQuantityIndex]";
	// only quantities 'FieldFlux' and 'HeatingsLoad' are allowed
	if(vectorQuantity != "FieldFlux" && vectorQuantity != "HeatingsLoad")
	{
		throw IBK::Exception(IBK::FormatString("Error decoding index: quantity "
			"%1 is not of vector valued type!")
			.arg(vectorQuantity), FUNC_ID);
	}
	// Construct a vector index element. This class contains
	// the translation utility between an index value and an identification
	// string.
	std::string keyString;
	VectorValuedQuantityIndex indexKey;
	try {
		indexKey.set(VectorValuedQuantityIndex::IK_Index, keyValue);
	}
	catch(IBK::Exception &ex) {
		throw IBK::Exception(ex, IBK::FormatString("Error extracting the vector index of quantity %1 from keyValue '%2'.")
			.arg(vectorQuantity).arg(keyValue), FUNC_ID);
	}
	// Retreive the key string that is used in the quantity name later.
	try {
		keyString = indexKey.toString();
	}
	catch(IBK::Exception &ex) {
		throw IBK::Exception( ex, IBK::FormatString("Error extracting the vector index of quantity %1 from keyValue '%2'.")
			.arg(vectorQuantity).arg(keyValue), FUNC_ID);
	}
	return keyString;
}


void  ConstructionSolverModel::setupGridAveraging(
	std::vector<WALL_MODEL::Element> &elems) {

#ifdef FineBoundaryLayerForMixedGrid
	// create offsets and grid for special discrtizatiuon case
	unsigned int equnOffset = 0;
	for(unsigned int e = 0; e < m_materialLayerElementOffset[1]; ++e) {
		m_averageElementOffset.push_back(e);
		m_energyBalanceEquation.push_back(equnOffset);
		m_moistureBalanceEquation.push_back(equnOffset + 1);
		equnOffset +=2;
	}

	for(unsigned int l = 1; l < m_materialLayerElementOffset.size() - 2; ++l) {
		m_averageElementOffset.push_back(m_materialLayerElementOffset[1]);
		for(unsigned int k = m_materialLayerElementOffset[l]; k < m_materialLayerElementOffset[l + 1]; ++k, ++equnOffset)
			m_energyBalanceEquation.push_back(equnOffset);
		m_moistureBalanceEquation.push_back(equnOffset);
		++equnOffset;
	}

	if (m_materialLayerElementOffset.size() > 2) {
		for (unsigned int e = m_materialLayerElementOffset[m_materialLayerElementOffset.size() - 2]; e < m_nElements; ++e) {
			m_averageElementOffset.push_back(e);
			m_energyBalanceEquation.push_back(equnOffset);
			m_moistureBalanceEquation.push_back(equnOffset + 1);
			equnOffset += 2;
		}
	}

	m_averageElementOffset.push_back(m_nElements);

#else
	// ensure that coarse grid boundaries match element boundaries of detailed grid
	// and modify if necessary

	// indexes for start position of
	// detailed element counter
	unsigned int jStart = 0;
	// width covered by detailed elements until counter jStart
	double detailedWidth = 0.0;

	// width coverd by by coarse grid elements
	double width = 0.0;
	// start position
	m_averageElementOffset.push_back(0);

	for (unsigned int i = 0; i < elems.size(); ++i) {
		WALL_MODEL::Element &elem = elems[i];
		// now find all detailed elements inside
		// current element
		unsigned int j = jStart;
		for ( ; j < m_nElements; ++j) {
			const WALL_MODEL::Element &detailedElem =
				m_elements[j];
			// update width covered by detailed grid
			detailedWidth += detailedElem.dx;
			if (detailedWidth >= width + elem.dx - 1e-08)
				break;
		}

		// correct element position
		if (i < elems.size() - 1) {
			// create new element position
			double dNew = detailedWidth - width;
			// correct next element
			WALL_MODEL::Element &nextElem = elems[i + 1];
			nextElem.dx = nextElem.dx + elem.dx - dNew;
			nextElem.x = detailedWidth + 0.5 * nextElem.dx;
			// correct current element
			elem.dx = dNew;
			elem.x = width + 0.5 * dNew;
			// update width
			width = detailedWidth;
		}
		else {
			// update width
			width += elem.dx;
		}

		// store moisture average index offset
		m_averageElementOffset.push_back(j + 1);

		// store equation index
		for (unsigned int k = jStart; k < j + 1; ++k) {
			m_energyBalanceEquation.push_back((unsigned int) m_moistureBalanceEquation.size() + k);
		}
		// set moisture element
		m_moistureBalanceEquation.push_back((unsigned int) m_moistureBalanceEquation.size() + j + 1);

		// correct index
		jStart = j + 1;
	}
#endif

	// resize missing quantities
	m_tempDet.resize(m_nElements, 0.0);
	m_tempAv.resize(elems.size(), 0.0);
}


void ConstructionSolverModel::averageFluxFromDetailedFlux(double *fluxAverage, const double *fluxDetailed) const {

	// 2 element discretization automatically enables detailed calculation mode
	IBK_ASSERT(m_coarseGridWallModel != nullptr);
	unsigned int nElements = (unsigned int)m_coarseGridWallModel->m_nElements;

	// integrate moisture density for internal layers
	for (unsigned int i = 0; i <= nElements; ++i) {
		unsigned int j = m_averageElementOffset[i];

		fluxAverage[i] = fluxDetailed[j];
	}
}

void ConstructionSolverModel::averageFieldFromDetailedField(double *vAverage, const double *vDetailed) const {

	// 2 element discretization automatically enables detailed calculation mode
	IBK_ASSERT(m_coarseGridWallModel != nullptr);
	unsigned int nElements = (unsigned int)m_coarseGridWallModel->m_nElements;

	// integrate moisture density for internal layers
	for (unsigned int i = 0; i < nElements; ++i) {

		// integrate internal states state
		double d = m_coarseGridWallModel->m_elements[i].dx;
		double v = 0.0;

		unsigned int jStart = m_averageElementOffset[i];
		unsigned int jEnd = m_averageElementOffset[i + 1];

		for (unsigned int j = jStart; j < jEnd; ++j) {
			const WALL_MODEL::Element & elem = m_elements[j];
			v += elem.dx * vDetailed[j];
		}
		IBK_ASSERT(d > 0);
		v /= d;

		// store average state
		vAverage[i] = v;
	}
}

void ConstructionSolverModel::detailedFieldFromAverageField(double *vDetailed, const double *vAverage) const {

	// 2 element discretization automatically enables detailed calculation mode
	IBK_ASSERT(m_coarseGridWallModel != nullptr);
	// we set a constant value
	// at all inside elements and extrapolate boundary values
	// with the help of external elements
	unsigned int nElements = (unsigned int)m_coarseGridWallModel->m_nElements;
	// now copy values for all layers
	for (unsigned int i = 0; i < nElements; ++i) {
		// find out start and end indexess
		unsigned int jStart = m_averageElementOffset[i];
		unsigned int jEnd = m_averageElementOffset[i + 1];

		for (unsigned int j = jStart; j < jEnd; ++j) {
			vDetailed[j] = vAverage[i];
		}
	}
}


} // namespace NANDRAD_MODEL

