/*	NANDRAD Solver Framework and Model Implementation.

	Copyright (c) 2012-today, Institut für Bauklimatik, TU Dresden, Germany

	Primary authors:
	  Andreas Nicolai  <andreas.nicolai -[at]- tu-dresden.de>
	  Anne Paepcke     <anne.paepcke -[at]- tu-dresden.de>

	This program is part of SIM-VICUS (https://github.com/ghorwin/SIM-VICUS)

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.
*/

#include "NM_NandradModel.h"

#if defined(_OPENMP)
#include <omp.h>
#endif // _OPENMP

#include <memory>

#include <IBK_Version.h>
#include <IBK_messages.h>
#include <IBK_FormatString.h>
#include <IBK_FileUtils.h>

#include <CCM_Constants.h>

#include <NANDRAD_ArgsParser.h>
#include <NANDRAD_Constants.h>
#include <NANDRAD_Project.h>
#include <NANDRAD_KeywordList.h>
#include <NANDRAD_FMIVariableDefinition.h>

#include <SOLFRA_IntegratorSundialsCVODE.h>
#include <SOLFRA_IntegratorExplicitEuler.h>
#include <SOLFRA_IntegratorImplicitEuler.h>
#include <SOLFRA_JacobianSparseCSR.h>
#include <SOLFRA_Constants.h>

#include <sundials/sundials_config.h>
#include <sundials/sundials_timer.h>

#include <SOLFRA_LESGMRES.h>
#include <SOLFRA_LESBiCGStab.h>
#include <SOLFRA_LESDense.h>
#include <SOLFRA_LESKLU.h>
#include <SOLFRA_PrecondILU.h>
#include <SOLFRA_PrecondILUT.h>

#ifdef IBK_STATISTICS
#define NANDRAD_TIMER_TIMEDEPENDENT 11
#define NANDRAD_TIMER_STATEDEPENDENT 12
#define NANDRAD_TIMER_SETTIME 13
#define NANDRAD_TIMER_SETY 14
#define NANDRAD_TIMER_UPDATE_ODEMODELS 15
#define NANDRAD_TIMER_UPDATE_MODELGROUPS 16
#define NANDRAD_TIMER_UPDATE_MODELS 17
#define NANDRAD_TIMER_YDOT 18
#endif

// Models

#include "NM_Loads.h"
#include "NM_Schedules.h"
#include "NM_RoomStatesModel.h"
#include "NM_RoomBalanceModel.h"
#include "NM_ConstantZoneModel.h"
#include "NM_OutputFile.h"
#include "NM_StateModelGroup.h"
#include "NM_FMIInputOutput.h"
#include "NM_OutputHandler.h"
#include "NM_ConstructionStatesModel.h"
#include "NM_ConstructionBalanceModel.h"
#include "NM_NaturalVentilationModel.h"
#include "NM_InternalLoadsModel.h"
#include "NM_InternalMoistureLoadsModel.h"
#include "NM_WindowModel.h"
#include "NM_RoomRadiationLoadsModel.h"
#include "NM_HydraulicNetworkModel.h"
#include "NM_ShadingControlModel.h"
#include "NM_ThermostatModel.h"
#include "NM_HeatLoadSummationModel.h"
#include "NM_IdealHeatingCoolingModel.h"
#include "NM_IdealPipeRegisterModel.h"
#include "NM_IdealSurfaceHeatingCoolingModel.h"
#include "NM_NetworkInterfaceAdapterModel.h"

#include "NM_ThermalNetworkStatesModel.h"
#include "NM_ThermalNetworkBalanceModel.h"
#include "NM_ThermalComfortModel.h"

namespace NANDRAD_MODEL {

NandradModel::NandradModel() :
	m_project(new NANDRAD::Project)
{
}


NandradModel::~NandradModel() {
	// final flush of outputs - only needed in case of solver crash or manual abort
	if (m_outputHandler != nullptr)
		m_outputHandler->flushCache();

	// free memory of owned instances
	delete m_project;
	delete m_lesSolver;
	delete m_jacobian;
	delete m_preconditioner;
	delete m_integrator;

	//	delete m_FMU2ModelDescription;

	for (std::vector<AbstractModel*>::iterator it = m_modelContainer.begin();
		it != m_modelContainer.end(); ++it)
	{
		delete *it;
	}
	for (std::set<StateModelGroup*>::iterator it = m_stateModelGroups.begin();
		it != m_stateModelGroups.end(); ++it)
	{
		delete *it;
	}

	delete m_schedules;
	delete m_outputHandler;
	m_outputHandler = nullptr;
	// note: m_loads is handled just as any other model and cleaned up as part of the m_modelContainer cleanup above

	delete m_progressLog;
}


void NandradModel::init(const NANDRAD::ArgsParser & args) {
	FUNCID(NandradModel::init);

	// *** Write Information about project file and relevant directories ***

	IBK::IBK_Message( IBK::FormatString("Executable path:      '%1'\n").arg(args.m_executablePath), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_INFO);
	IBK::IBK_Message( IBK::FormatString("Project file:         '%1'\n").arg(args.m_projectFile), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
	IBK::IBK_Message( IBK::FormatString("Project directory:    '%1'\n").arg(m_projectFilePath.parentPath()), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_INFO);

	IBK::IBK_Message( IBK::FormatString("Output root dir:      '%1'\n").arg(m_dirs.m_rootDir), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
	{
		IBK_MSG_INDENT;
		IBK::IBK_Message( IBK::FormatString("log directory:      '%1'\n").arg(m_dirs.m_logDir), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_INFO);
		IBK::IBK_Message( IBK::FormatString("var/data directory: '%1'\n").arg(m_dirs.m_varDir), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_INFO);
		IBK::IBK_Message( IBK::FormatString("results directory:  '%1'\n").arg(m_dirs.m_resultsDir), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_INFO);
	}

	// *** Create physical model implementation object and initialize with project. ***

	// initialize project data structure with default values
	// (these values may be overwritten by project data and command line options)
	m_project->initDefaults();

	// read input data from file
	IBK::IBK_Message( IBK::FormatString("Reading project file\n"), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
	m_project->readXML(args.m_projectFile);

	// *** Print Out Placeholders ***
	IBK::IBK_Message( IBK::FormatString("Path Placeholders\n"), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_INFO);
	{
		IBK_MSG_INDENT;
		for (std::map<std::string, IBK::Path>::const_iterator it = m_project->m_placeholders.begin();
			it != m_project->m_placeholders.end(); ++it)
		{
			std::stringstream strm;
			strm << std::setw(25) << std::left << it->first;
			IBK::IBK_Message( IBK::FormatString("%1 -> %2\n").arg(strm.str()).arg(it->second), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_INFO);
		}
	}

	// *** Remove duplicate construction IDs ***
	m_project->mergeSameConstructions();

	// Now, the data model (i.e. m_project) is unmodified structurally in memory and persistant pointers can be stored
	// to data entries for fast access during simulation.

	IBK::IBK_Message( IBK::FormatString("Initializing model\n"), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
	IBK_MSG_INDENT;

	// *** Initialize solver parameters (and apply command line overrides) ***
	initSolverParameter(args);
	// *** Initialize simulation parameters ***
	initSimulationParameter();
	// *** Initialize Climatic Loads ***
	initClimateData();
	// *** Initialize Schedules ***
	initSchedules();
	// *** Initialize RoomBalanceModels and ConstantZoneModels ***
	initZones();
	// *** Initialize Wall/Construction Modules ***
	initWallsAndInterfaces();
	// *** Initialize Networks ***
	initNetworks();
	// *** Initialize ModelGroups ***
//	initModelGroups();
	// *** Initialize all internal fmus ***
//	initFMUComponents();
	// *** Initialize all models ***
	initModels();
	// *** Initialize FMI model ***
	if (!args.m_executablePath.isValid()) // empty executable path = we are inside an FMU -> initFMI()
		initFMI();
	// *** Initialize Object Lists ***
	initObjectLists();
	// *** Initialize outputs ***
	initOutputs(args.m_restart || args.m_restartFrom);

	// Here, *all* model objects must be created and stored in m_modelContainer !!!

	// *** Setup model dependencies ***
	initModelDependencies();
	// *** Setup states model graph and generate model groups ***
	initModelGraph();
	// *** Initialize list with output references ***
	initOutputReferenceList();
	// *** Initialize Global Solver ***
	initSolverVariables();
	// *** Initialize sparse solver matrix ***
	initSolverMatrix();
	// *** Init statistics/feedback output ***
	initStatistics(this, args.m_restart);
}


void NandradModel::setupDirectories(const NANDRAD::ArgsParser & args) {
	FUNCID(NandradModel::setupDirectories);

	m_projectFilePath = args.m_projectFile;
	try {
		// ***** Check if project file exists *****
		if (!m_projectFilePath.isFile())
			throw IBK::Exception( IBK::FormatString("Project file '%1' does not exist (or access denied).").arg(m_projectFilePath), FUNC_ID);

		// ***** Create directory structure for solver log and output files *****

		if (args.hasOption(IBK::SolverArgsParser::GO_OUTPUT_DIR)) {
			m_dirs.create( IBK::Path(args.option(IBK::SolverArgsParser::GO_OUTPUT_DIR)) );
		}
		else {
			m_dirs.create(m_projectFilePath.withoutExtension());
		}
	}
	catch (IBK::Exception & ex) {
		throw IBK::Exception(ex, IBK::FormatString("Initialization of Nandrad Model for project '%1' failed.")
			.arg(m_projectFilePath), FUNC_ID);
	}
	catch (std::exception & ex) {
		throw IBK::Exception(IBK::FormatString("Initialization of Nandrad Model for project '%1' failed "
			"with error message:%2\n").arg(m_projectFilePath).arg(ex.what()), FUNC_ID);
	}
}



/*** Functions re-implemented from SOLFRA::ModelInterface. ***/

double NandradModel::dt0() const {
	return m_project->m_solverParameter.m_para[NANDRAD::SolverParameter::P_InitialTimeStep].value;
}


SOLFRA::ModelInterface::CalculationResult NandradModel::setTime(double t) {
	if (m_t == t)
		return SOLFRA::ModelInterface::CalculationSuccess; // model state already at time t, no need to update

	// cache time point
	m_t = t;
	// only update time-dependent variables when a new time-point is set
	m_tChanged = true;

	// all successful
	return SOLFRA::ModelInterface::CalculationSuccess;
}


SOLFRA::ModelInterface::CalculationResult NandradModel::setY(const double * y) {
	// copy memory
	std::memcpy(&m_y[0], y, m_n*sizeof(double));

	// mark model as outdated
	m_yChanged = true;

	// fluxes and divergences are computed in ydot()
	return SOLFRA::ModelInterface::CalculationSuccess;
}


SOLFRA::ModelInterface::CalculationResult NandradModel::ydot(double * ydot) {
	FUNCID(NandradModel::ydot);
	try {
		int calculationResultFlag = 0;
		// only update if necessary
		// we re-compute the solution if either the time point or the solution variables
		// have changed.
		if (m_tChanged) {
			// we must assume that the balance equation update algorithm depends on
			// time
#ifdef IBK_STATISTICS
			SUNDIALS_TIMED_FUNCTION(NANDRAD_TIMER_TIMEDEPENDENT,
				calculationResultFlag |= updateTimeDependentModels()
			);
			++m_nTimeFunctionEvals;
#else
			calculationResultFlag |= updateTimeDependentModels();
			calculationResultFlag |= updateStateDependentModels();
#endif
		}
		// y changed
		else if (m_yChanged) {
			// we must assume that the balance equation update algorithm depends on states
			// and thus update all elements
#ifdef IBK_STATISTICS
			SUNDIALS_TIMED_FUNCTION(NANDRAD_TIMER_STATEDEPENDENT,
				calculationResultFlag |= updateStateDependentModels()
			);
			++m_nStateFunctionEvals;
#else
			calculationResultFlag |= updateStateDependentModels();
#endif
		}
		if (calculationResultFlag != 0) {
			if (calculationResultFlag & 2)
				return SOLFRA::ModelInterface::CalculationAbort;
			else
				return SOLFRA::ModelInterface::CalculationRecoverableError;
		}

		// stop if no target space is given (we only needed to update the model state, no outputs are requested)
		if (ydot == nullptr)
			return SOLFRA::ModelInterface::CalculationSuccess;

		// *** store ydot ***
		std::memcpy(ydot, &m_ydot[0], m_n*sizeof(double));

		// *** feedback to user ***
		IBK_FastMessage(IBK::VL_DEVELOPER)(IBK::FormatString("    ydot: t=%1 [%2]\n")
							.arg(m_t).arg(IBK::Time(2000,m_t).toTOYFormat()),
							IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_DEVELOPER);
		m_feedback.writeFeedbackFromF(m_t);
	}
	catch (IBK::Exception & ex) {
		throw IBK::Exception(ex, "Error retrieving divergences!", FUNC_ID);
	}

	return SOLFRA::ModelInterface::CalculationSuccess;
}


void NandradModel::writeOutputs(double t_out, const double * y_out) {
	//IBK::IBK_Message(IBK::FormatString("Writing output at t_out = %1 s\n").arg(t_out));
	// update state of model to output time and variables
	// finally write the output
	setTime(t_out);
	setY(y_out);
	ydot(nullptr);

	// move (relative) simulation time to absolute time (offset to midnight, January 1st of the start year)
	double t_secondsOfYear = t_out + m_project->m_simulationParameter.m_interval.m_para[NANDRAD::Interval::P_Start].value;

	m_outputHandler->writeOutputs(t_out, t_secondsOfYear, m_varSubstitutionMap);

	// write feedback to user
	IBK_ASSERT(m_t == t_out);
	m_feedback.writeFeedback(t_out, false);
}


void NandradModel::writeFinalOutputs() {
	// guard against call if we haven't even initialized our model
	if (m_outputHandler == nullptr)
		return;
	SUNDIALS_TIMED_FUNCTION( SUNDIALS_TIMER_WRITE_OUTPUTS,
		m_outputHandler->flushCache();
	);
}


std::string NandradModel::simTime2DateTimeString(double t) const {
	// add start time offset to t and then call parent function
	int startYear = m_project->m_simulationParameter.m_intPara[NANDRAD::SimulationParameter::IP_StartYear].value;
	t += m_project->m_simulationParameter.m_interval.m_para[NANDRAD::Interval::P_Start].value;
	return IBK::Time(startYear, t).toShortDateFormat();
}


void NandradModel::stepCompleted(double t, const double * /*y*/ ) {

	// tell all modules that need to remember the last integration step, that they
	// can cache a new state
	for (std::vector<AbstractTimeDependency*>::iterator it = m_timeModelContainer.begin();
		it != m_timeModelContainer.end(); ++it)
	{
		(*it)->stepCompleted(t);
	}
}


SOLFRA::LESInterface * NandradModel::lesInterface() {
	FUNCID(NandradModel::lesInterface);

	if (m_lesSolver != nullptr)
		return m_lesSolver;
	IBK_ASSERT(m_preconditioner == nullptr);
	IBK_ASSERT(m_jacobian == nullptr);

	if (m_project->m_solverParameter.m_integrator == NANDRAD::SolverParameter::I_ExplicitEuler) {
		IBK::IBK_Message("Linear Equation Solver Modules not needed for Explicit Euler.\n", IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
		return nullptr;
	}

	IBK::IBK_Message("\n", IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
	IBK::IBK_Message("Creating Linear Equation Solver Modules\n", IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
	IBK_MSG_INDENT;
	IBK::IBK_Message( IBK::FormatString("Number of unknowns: %1\n").arg(m_n), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);

	// create LES solver based on selected setting
	switch (m_project->m_solverParameter.m_lesSolver) {

		// Dense
		case NANDRAD::SolverParameter::LES_Dense : {
			m_lesSolver = new SOLFRA::LESDense;
			IBK_Message( IBK::FormatString("Using generic Dense solver!\n"),
				IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
			return m_lesSolver;
		}

		// KLU
		case NANDRAD::SolverParameter::LES_KLU: {
			SOLFRA::JacobianSparseCSR *jacSparse = new SOLFRA::JacobianSparseCSR(n(), nnz(), &m_ia[0], &m_ja[0],
				&m_iaT[0], &m_jaT[0]);
			m_jacobian = jacSparse;
			// create KLU solver
			m_lesSolver = new SOLFRA::LESKLU;
			IBK_Message(IBK::FormatString("Using generic KLU solver!\n"),
				IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
			return m_lesSolver;
		}

		// GMRES
		case NANDRAD::SolverParameter::LES_GMRES : {
			m_lesSolver = new SOLFRA::LESGMRES;
			IBK_Message(IBK::FormatString("Using GMRES solver\n"),  IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
		} break;

		// BiCGStab
		case NANDRAD::SolverParameter::LES_BiCGStab : {
			m_lesSolver = new SOLFRA::LESBiCGStab;
			IBK_Message(IBK::FormatString("Using BiCGStab solver\n"),  IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
		} break;

		default:
			throw IBK::Exception("Unknown or undefined LES solver.", FUNC_ID);
	}

	// must be an iterative solver, so configure preconditioners and jacobian matrix generators
	std::string precondName = "no preconditioner";

	// determine preconditioner type
	switch (m_project->m_solverParameter.m_preconditioner) {

		// ILU preconditioner
		case NANDRAD::SolverParameter::PRE_ILU : {
			// work with a sparse jacobian
			SOLFRA::JacobianSparseCSR *jacSparse = new SOLFRA::JacobianSparseCSR(n(), nnz(), &m_ia[0], &m_ja[0],
				&m_iaT[0], &m_jaT[0]);

			m_jacobian = jacSparse;

			// ILUT preconditioner
			if (!m_project->m_solverParameter.m_intPara[NANDRAD::SolverParameter::IP_PreILUWidth].name.empty()) {
				// always > 0, already checked
				unsigned int fillIn = m_project->m_solverParameter.m_intPara[NANDRAD::SolverParameter::IP_PreILUWidth].toUInt(true);

				m_preconditioner = new SOLFRA::PrecondILUT(SOLFRA::PrecondInterface::Right, fillIn);
				precondName = IBK::FormatString("ILUT preconditioner (with max. %1 non-zero elements per column of U and L)").arg(fillIn).str();
			}
			else {
				m_preconditioner = new SOLFRA::PrecondILU(SOLFRA::PrecondInterface::Right);
				precondName = "ILU preconditioner";

			}
		} break;

		// no preconditioner
		case NANDRAD::SolverParameter::NUM_PRE : ;
	}

	SOLFRA::LESInterfaceIterative * lesIter = dynamic_cast<SOLFRA::LESInterfaceIterative *>(m_lesSolver);
	IBK_ASSERT(lesIter != nullptr);

	// set iterative LES solver options
	lesIter->m_maxKrylovDim = m_project->m_solverParameter.m_intPara[NANDRAD::SolverParameter::IP_MaxKrylovDim].toUInt();
	lesIter->m_linIterConvCoeff = m_project->m_solverParameter.m_para[NANDRAD::SolverParameter::P_IterativeSolverConvCoeff].value;

	IBK_Message(IBK::FormatString("%1 selected, MaxKrylovDim = %2\n")
		.arg(precondName).arg(lesIter->m_maxKrylovDim),  IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);

	return m_lesSolver;
}


SOLFRA::JacobianInterface *  NandradModel::jacobianInterface() {
	return m_jacobian;
}


SOLFRA::IntegratorInterface * NandradModel::integratorInterface() {
	FUNCID(NandradModel::integratorInterface);

	if (m_integrator != nullptr)
		return m_integrator;

	IBK::IBK_Message("\n", IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
	IBK::IBK_Message("Creating Integrator\n", IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
	IBK_MSG_INDENT;

	if (m_project->m_solverParameter.m_integrator == NANDRAD::SolverParameter::I_ExplicitEuler) {
		IBK::IBK_Message("Using Explict Euler integrator.\n", IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
		SOLFRA::IntegratorExplicitEuler * integrator = new SOLFRA::IntegratorExplicitEuler();
		integrator->m_dt = dt0();
		m_integrator = integrator;
	}
	else if (m_project->m_solverParameter.m_integrator == NANDRAD::SolverParameter::I_ImplicitEuler) {
		IBK::IBK_Message("Using Implicit Euler integrator.\n", IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
		SOLFRA::IntegratorImplicitEuler * integrator = new SOLFRA::IntegratorImplicitEuler();
		// set parameters given by Solverparameter section
		integrator->m_absTol = m_project->m_solverParameter.m_para[NANDRAD::SolverParameter::P_AbsTol].value;
		integrator->m_relTol = m_project->m_solverParameter.m_para[NANDRAD::SolverParameter::P_RelTol].value;
		integrator->m_dtMax = m_project->m_solverParameter.m_para[NANDRAD::SolverParameter::P_MaxTimeStep].value;
		integrator->m_nonLinConvCoeff = m_project->m_solverParameter.m_para[NANDRAD::SolverParameter::P_NonlinSolverConvCoeff].value;
		integrator->m_maximumNonlinearIterations = m_project->m_solverParameter.m_intPara[NANDRAD::SolverParameter::IP_MaxNonlinIter].toUInt(true);

		m_integrator = integrator;
	}
	else if (m_project->m_solverParameter.m_integrator == NANDRAD::SolverParameter::I_CVODE ||
		m_project->m_solverParameter.m_integrator == NANDRAD::SolverParameter::NUM_I)
	{
		IBK::IBK_Message("Using CVODE integrator.\n", IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
		SOLFRA::IntegratorSundialsCVODE * integrator = new SOLFRA::IntegratorSundialsCVODE();
		// set parameters given by Solverparameter section
		integrator->m_absTol = m_project->m_solverParameter.m_para[NANDRAD::SolverParameter::P_AbsTol].value;
		integrator->m_relTol = m_project->m_solverParameter.m_para[NANDRAD::SolverParameter::P_RelTol].value;
		integrator->m_dtMax = m_project->m_solverParameter.m_para[NANDRAD::SolverParameter::P_MaxTimeStep].value;
		integrator->m_dtMin = m_project->m_solverParameter.m_para[NANDRAD::SolverParameter::P_MinTimeStep].value;
		integrator->m_maxOrder = m_project->m_solverParameter.m_intPara[NANDRAD::SolverParameter::IP_MaxOrder].toUInt(true);
		integrator->m_maxSteps = 100000000; // extremely large value
		integrator->m_nonLinConvCoeff = m_project->m_solverParameter.m_para[NANDRAD::SolverParameter::P_NonlinSolverConvCoeff].value;
		integrator->m_maxNonLinIters = m_project->m_solverParameter.m_intPara[NANDRAD::SolverParameter::IP_MaxNonlinIter].toUInt(true);

		m_integrator = integrator;
	}
	return m_integrator;
}


SOLFRA::PrecondInterface * NandradModel::preconditionerInterface() {
	return m_preconditioner;
}


SOLFRA::ModelInterface::CalculationResult NandradModel::calculateErrorWeights(const double *y, double *weights) {
	// tolerances are properties of all error controled integrators
	SOLFRA::IntegratorErrorControlled *integrator =
		dynamic_cast<SOLFRA::IntegratorErrorControlled *>(integratorInterface());
	// protect against invalid integrator
	IBK_ASSERT(integrator != nullptr);
	// start with the classic definition
	const double absTol = integrator->m_absTol;
	const double relTol = integrator->m_relTol;
	// fill error weights with classical definition
	for (unsigned int i = 0; i < m_n; ++i) {
		weights[i] = 1.0/(relTol * std::fabs(y[i]) + absTol);
	}

	// modify weighting factor for all zones
	for (unsigned int i=0; i<m_nZones; ++i) {
		// currently each zone has exactly one state variable
		unsigned int idx = m_zoneVariableOffset[i];
		// weight single zone balance with mean number of wall discrtization elements
		weights[idx] *= m_weightsFactorZones;
	}

	// loop over all network models
	for (unsigned int i=0; i<m_nNetworks; ++i) {
		// start index for network's y vector
		unsigned int idx = m_networkVariableOffset[i];

		// retrieve error weight for all network variables
		std::vector<double> errWeights;
		m_networkStatesModelContainer[i]->calculateErrorWeightFactors(errWeights);

		// enlarge default factors for all network variables
		for (unsigned int j=0; j<m_networkStatesModelContainer[i]->nPrimaryStateResults(); ++j)
			weights[idx + j] *= errWeights[j];
	}

	return SOLFRA::ModelInterface::CalculationSuccess;
}


bool NandradModel::hasErrorWeightsFunction() {
	return true;
}


std::size_t NandradModel::serializationSize() const {
	size_t s = 0;
	// serialize all model states
	// these iclude controller states and stored solution variables
	// of steady-state sub-systems
	for(const AbstractModel *model : m_modelContainer)
		s += model->serializationSize();

	return s;
}


void NandradModel::serialize(void* & dataPtr) const {
	// cache all model states
	for(const AbstractModel *model : m_modelContainer)
		model->serialize(dataPtr);
}


void NandradModel::deserialize(void* & dataPtr) {
	// restore all model states
	for(AbstractModel *model : m_modelContainer)
		model->deserialize(dataPtr);
}


void NandradModel::writeMetrics(double simtime, std::ostream * metricsFile) {
	(void)simtime;
	(void)metricsFile;

#ifdef IBK_STATISTICS
	FUNCID(NandradModel::writeMetrics);
	std::string ustr = IBK::Time::suitableTimeUnit(simtime);
	double tTimeEval = TimerSum(NANDRAD_TIMER_TIMEDEPENDENT);
	IBK::IBK_Message(IBK::FormatString("Nandrad model: Time Function evaluation    = %1 (%2 %%)  %3\n")
		.arg(IBK::Time::format_time_difference(tTimeEval, ustr, true), 13)
		.arg(tTimeEval / simtime * 100, 5, 'f', 2)
		.arg(m_nTimeFunctionEvals, 8),
		IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
	double tStatesEval = TimerSum(NANDRAD_TIMER_STATEDEPENDENT);
	IBK::IBK_Message(IBK::FormatString("Nandrad model: State Function evaluation   = %1 (%2 %%)  %3\n")
		.arg(IBK::Time::format_time_difference(tStatesEval, ustr, true), 13)
		.arg(tStatesEval / simtime * 100, 5, 'f', 2)
		.arg(m_nStateFunctionEvals, 8),
		IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
	double tSetTime = TimerSum(NANDRAD_TIMER_SETTIME);
	IBK::IBK_Message(IBK::FormatString("Nandrad model: SetTime calls               = %1 (%2 %%)  %3\n")
		.arg(IBK::Time::format_time_difference(tSetTime, ustr, true), 13)
		.arg(tSetTime / simtime * 100, 5, 'f', 2)
		.arg(m_nSetTimeCalls, 8),
		IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
	double tSetY = TimerSum(NANDRAD_TIMER_SETY);
	IBK::IBK_Message(IBK::FormatString("Nandrad model: SetY calls                  = %1 (%2 %%)  %3\n")
		.arg(IBK::Time::format_time_difference(tSetY, ustr, true), 13)
		.arg(tSetY / simtime * 100, 5, 'f', 2)
		.arg(m_nSetYCalls, 8),
		IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
	double tUpdateODEModels = TimerSum(NANDRAD_TIMER_UPDATE_ODEMODELS);
	IBK::IBK_Message(IBK::FormatString("Nandrad model: ODE models update           = %1 (%2 %%)  %3\n")
		.arg(IBK::Time::format_time_difference(tUpdateODEModels, ustr, true), 13)
		.arg(tUpdateODEModels / simtime * 100, 5, 'f', 2)
		.arg(m_nODEModelsUpdate, 8),
		IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
	double tModelGroupsUpdate = TimerSum(NANDRAD_TIMER_UPDATE_MODELGROUPS);
	IBK::IBK_Message(IBK::FormatString("Nandrad model: Cyclic model groups update  = %1 (%2 %%)  %3\n")
		.arg(IBK::Time::format_time_difference(tModelGroupsUpdate, ustr, true), 13)
		.arg(tModelGroupsUpdate / simtime * 100, 5, 'f', 2)
		.arg(m_nModelGroupsUpdate, 8),
		IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
	double tModelsUpdate = TimerSum(NANDRAD_TIMER_UPDATE_MODELS);
	IBK::IBK_Message(IBK::FormatString("Nandrad model: Models update               = %1 (%2 %%)  %3\n")
		.arg(IBK::Time::format_time_difference(tModelsUpdate, ustr, true), 13)
		.arg(tModelsUpdate / simtime * 100, 5, 'f', 2)
		.arg(m_nModelsUpdate, 8),
		IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
	double tYdot = TimerSum(NANDRAD_TIMER_YDOT);
	IBK::IBK_Message(IBK::FormatString("Nandrad model: Ydot calls                  = %1 (%2 %%)  %3\n")
		.arg(IBK::Time::format_time_difference(tYdot, ustr, true), 13)
		.arg(tYdot / simtime * 100, 5, 'f', 2)
		.arg(m_nYdotCalls, 8),
		IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
#endif
}



/*** Functions re-implemented from SOLFRA::OutputScheduler. ***/

double NandradModel::nextOutputTime(double t) {

	// loop over all defined output grids and search for next scheduled output
	double tOutNext = std::numeric_limits<double>::max(); // largest possible value

	// get time including start offset, since output intervals are defined in terms of absolute time reference
	double tWithStartOffset = t + m_project->m_simulationParameter.m_interval.m_para[NANDRAD::Interval::P_Start].value;

	for (std::vector<NANDRAD::OutputGrid>::const_iterator it = m_project->m_outputs.m_grids.begin();
		 it != m_project->m_outputs.m_grids.end(); ++it)
	{
		tOutNext = std::min(tOutNext, it->computeNextOutputTime(tWithStartOffset));
	}

	// convert tOutNext back to simulation time by subtracting offset
	return tOutNext - m_project->m_simulationParameter.m_interval.m_para[NANDRAD::Interval::P_Start].value;
}



/*** Public Static Functions ***/

void NandradModel::printVersionStrings() {
	FUNCID(ModelImpl::printVersionStrings);

	// print compiler and version information
	IBK::Version::printCompilerVersion();
	IBK::IBK_Message("\n", IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
	IBK::IBK_Message("NANDRAD version                                  " + std::string(NANDRAD::LONG_VERSION) + "\n", IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
	IBK::IBK_Message("IBK library version                              " + std::string(IBK::VERSION) + "\n", IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
	IBK::IBK_Message("CCM library version                              " + std::string(CCM::LONG_VERSION) + "\n", IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
}


void NandradModel::initSolverParameter(const NANDRAD::ArgsParser & args) {
	FUNCID(NandradModel::initSolverParameter);

	IBK::IBK_Message( IBK::FormatString("Initializing Solver Parameter\n"), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
	IBK_MSG_INDENT;

	NANDRAD::SolverParameter &solverParameter = m_project->m_solverParameter;

	// process override command line flags/options

	// --integrator
	if (args.hasOption(NANDRAD::ArgsParser::OO_INTEGRATOR)) {
		// check if this is a valid/known solver
		std::string solverString = args.option(NANDRAD::ArgsParser::OO_INTEGRATOR);
		if (IBK::toupper_string(solverString) == "CVODE") // CVode
			solverParameter.m_integrator = NANDRAD::SolverParameter::I_CVODE;
		else if (IBK::toupper_string(solverString) == "EXPLICITEULER") // ExplicitEuler
			solverParameter.m_integrator = NANDRAD::SolverParameter::I_ExplicitEuler;
		else if (IBK::toupper_string(solverString) == "IMPLICITEULER") // ImplicitEuler
			solverParameter.m_integrator = NANDRAD::SolverParameter::I_ImplicitEuler;
		else {
			throw IBK::Exception( IBK::FormatString("Unknown/unsupported integrator '%1'.").arg(solverString), FUNC_ID);
		}
	}

	// --les-solver
	if (!args.m_lesSolverName.empty()) {
		// check if this is a valid/known solver
		if (IBK::toupper_string(args.m_lesSolverName) == "DENSE") {
			solverParameter.m_lesSolver = NANDRAD::SolverParameter::LES_Dense;
			if (args.m_lesSolverOption != (unsigned int)-1)
				throw IBK::Exception( IBK::FormatString("Invalid format of --les-solver=DENSE option."), FUNC_ID);
		}
		else if (IBK::toupper_string(args.m_lesSolverName) == "KLU") {
			solverParameter.m_lesSolver = NANDRAD::SolverParameter::LES_KLU;
			if (args.m_lesSolverOption != (unsigned int)-1)
				throw IBK::Exception(IBK::FormatString("Invalid format of --les-solver=KLU option."), FUNC_ID);
		}
		else if (IBK::toupper_string(args.m_lesSolverName) == "GMRES") {
			solverParameter.m_lesSolver = NANDRAD::SolverParameter::LES_GMRES;
			if (args.m_lesSolverOption != (unsigned int)-1)
				// also store Krylov subspace dimension
				solverParameter.m_para[NANDRAD::SolverParameter::IP_MaxKrylovDim].value = args.m_lesSolverOption;
		}
		else if (IBK::toupper_string(args.m_lesSolverName) == "BICGSTAB") {
			solverParameter.m_lesSolver = NANDRAD::SolverParameter::LES_BiCGStab;
			if (args.m_lesSolverOption != (unsigned int)-1)
				solverParameter.m_para[NANDRAD::SolverParameter::IP_MaxKrylovDim].value = args.m_lesSolverOption;
		}
		else {
			throw IBK::Exception( IBK::FormatString("Unknown/unsupported LES-solver '%1'.").arg(args.m_lesSolverName), FUNC_ID);
		}
	}

	// --precond
	if (!args.m_preconditionerName.empty()) {
		// must have an iterative solver for preconditioner use
		if (solverParameter.m_lesSolver != NANDRAD::SolverParameter::LES_GMRES &&
			solverParameter.m_lesSolver != NANDRAD::SolverParameter::LES_BiCGStab)
		{
			throw IBK::Exception( IBK::FormatString("Cannot use --precond option with direct LES-solver."), FUNC_ID);
		}

		// check if this is a valid/known preconditioner
		if (IBK::toupper_string(args.m_preconditionerName) == "ILU") {
			solverParameter.m_preconditioner = NANDRAD::SolverParameter::PRE_ILU;
			if (args.m_preconditionerOption != (unsigned int)-1)
				// also store fill-in
				solverParameter.m_intPara[NANDRAD::SolverParameter::IP_PreILUWidth].set(
					NANDRAD::KeywordList::Keyword("SolverParameter::intPara_t", NANDRAD::SolverParameter::IP_PreILUWidth),
					(int)args.m_preconditionerOption);
		}
		else {
			throw IBK::Exception( IBK::FormatString("Unknown/unsupported preconditioner '%1'.").arg(args.m_preconditionerName), FUNC_ID);
		}
	}


	// *** mandatory arguments (defaults are always specified) ***

	const IBK::Parameter &absTol = solverParameter.m_para[NANDRAD::SolverParameter::P_AbsTol];
	if (absTol.value <= 0.0)
		throw IBK::Exception(IBK::FormatString("Error initializing wall solver: "
			"SolverParameter 'AbsTol' is smaller than/ equal to zero."), FUNC_ID);

	const IBK::Parameter &relTol = solverParameter.m_para[NANDRAD::SolverParameter::P_RelTol];
	if (relTol.value <= 0.0)
		throw IBK::Exception(IBK::FormatString("Error initializing wall solver: "
			"SolverParameter 'RelTol' is smaller than/ equal to zero."), FUNC_ID);

	const IBK::Parameter & minDx = solverParameter.m_para[NANDRAD::SolverParameter::P_DiscMinDx];
	if (minDx.value <= 1e-10)
		throw IBK::Exception("Invalid parameter for DiscMinDx in SolverParameter settings, should be > 1e-10 m.", FUNC_ID);

	const IBK::Parameter & density = solverParameter.m_para[NANDRAD::SolverParameter::P_DiscStretchFactor];
	if (density.value < 1 && density.value != 0.0)
		throw IBK::Exception("Invalid parameter for DiscDetailLevel in SolverParameter settings (grid density must be == 0 or >= 1).", FUNC_ID);

	solverParameter.m_intPara[NANDRAD::SolverParameter::IP_MaxKrylovDim].toUInt(true,
		"Invalid parameter for MaxKrylovDim in SolverParameter settings.");

	const IBK::Parameter &nonlinConvCoeff = solverParameter.m_para[NANDRAD::SolverParameter::P_NonlinSolverConvCoeff];
	// check validity
	if (nonlinConvCoeff.value <= 0.0)
		throw IBK::Exception("Invalid parameter for NonlinSolverConvCoeff in SolverParameter settings.", FUNC_ID);

	const IBK::Parameter &iterativeConvCoeff = solverParameter.m_para[NANDRAD::SolverParameter::P_IterativeSolverConvCoeff];
	// check validity
	if (iterativeConvCoeff.value <= 0.0)
		throw IBK::Exception(IBK::FormatString("Invalid parameter for IterativeSolverConvCoeff in SolverParameter settings."), FUNC_ID);

	// *** optional arguments (without guaranteed default value) ***

	const IBK::Parameter & initialDt = solverParameter.m_para[NANDRAD::SolverParameter::P_InitialTimeStep];
	if (initialDt.value <= 0)
		throw IBK::Exception("Invalid parameter for InitialTimeStep in SolverParameter settings.", FUNC_ID);

	// *** Define standard behavior if definitions are missing ***

	if (m_project->m_solverParameter.m_integrator != NANDRAD::SolverParameter::I_ExplicitEuler) {
		// if no LES solver has been specified, default to GMRES with ILU preconditioner
		if (m_project->m_solverParameter.m_lesSolver == NANDRAD::SolverParameter::NUM_LES) {
			m_project->m_solverParameter.m_lesSolver = NANDRAD::SolverParameter::LES_KLU;
			IBK::IBK_Message("Auto-selecting KLU as linear equation system solver.\n", IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
		}
	}

	// *** Parallel Code Setup ***
#ifdef _OPENMP
#pragma omp parallel
	{
#pragma omp master
		{
			m_numThreads = omp_get_num_threads();
		}
	}
	IBK::IBK_Message(IBK::FormatString("Parallel solver version, running with %1 threads\n").arg(m_numThreads), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);

#else // _OPENMP
	m_numThreads = 1;
#endif // _OPENMP
}


void NandradModel::initSimulationParameter() {
	FUNCID(NandradModel::initSimulationParameter);

	IBK::IBK_Message( IBK::FormatString("Initializing Simulation Parameter\n"), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);

	const NANDRAD::SimulationParameter &simPara = m_project->m_simulationParameter;

	// check validity of the parameter data, since data may be overrided in project file
	simPara.checkParameters();

	// Set simulation interval

	// Simulation time always starts with zero (=solver time).
	// All models dealing with absolute time reference (climate data and schedules) will
	// shift the simulation time by the start time/year offset.
	m_t0  = 0;

	// We always have end time and start time given, as part of the simulation defaults.
	// Project file may have different values, but by reading project file, settings cannot be removed.

	m_tEnd = simPara.m_interval.m_para[NANDRAD::Interval::P_End].value -
		simPara.m_interval.m_para[NANDRAD::Interval::P_Start].value;
}


void NandradModel::initClimateData() {
	FUNCID(NandradModel::initClimateData);

	IBK::IBK_Message(IBK::FormatString("Initializing Climatic Data\n"), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
	IBK_MSG_INDENT;

	try {
		m_loads = new Loads;
		// insert into model container
		m_modelContainer.push_back(m_loads);	// now owns the model and handles memory cleanup
		// insert into time model container
		m_timeModelContainer.push_back(m_loads);

		m_loads->setup(m_project->m_location, m_project->m_simulationParameter, m_project->m_placeholders);
	}
	catch (IBK::Exception & ex) {
		throw IBK::Exception(ex, IBK::FormatString("Error initializing climatic loads model."), FUNC_ID);
	}
}


void NandradModel::initSchedules() {
	FUNCID(NandradModel::initSchedules);
	IBK::IBK_Message(IBK::FormatString("Initializing Schedules\n"), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
	IBK_MSG_INDENT;

	try {
		m_schedules = new Schedules; // owned, memory released in destructor

		// insert into time model container
		m_timeModelContainer.push_back(m_schedules);

		// init schedules
		m_schedules->setup(*m_project);
	}
	catch (IBK::Exception & ex) {
		throw IBK::Exception(ex, IBK::FormatString("Error initializing schedules."), FUNC_ID);
	}
}


void NandradModel::initFMI() {
	FUNCID(NandradModel::initFMI);
	IBK::IBK_Message(IBK::FormatString("Initializing FMI interface\n"), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
	IBK_MSG_INDENT;

	// check validity of the parameter data
	m_project->m_fmiDescription.checkParameters();

	try {
		// create FMIInputOutput model
		m_fmiInputOutput = new FMIInputOutput;
		// init FMI import/export model - this should only be done, if we are actually inside an FMU
		m_fmiInputOutput->setup(*m_project);
		// insert into model container
		m_modelContainer.push_back(m_fmiInputOutput);
		// insert into time model container
		m_timeModelContainer.push_back(m_fmiInputOutput);
	}
	catch (IBK::Exception & ex) {
		throw IBK::Exception(ex, IBK::FormatString("Error initializing FMI interface model."), FUNC_ID);
	}
}


void NandradModel::initZones() {
	FUNCID(NandradModel::initZones);
	IBK::IBK_Message( IBK::FormatString("Initializing Zones\n"), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
	IBK_MSG_INDENT;

	// create model instances for all active zones
	// for each active zone, we need:
	// - a RoomBalanceModel
	// - a RoomThermalLoadsModel (collects heating loads from model instances)

	// created models are added to m_stateModelContainer (which owns them)
	// a reference is also placed in m_roomStateModelContainer, which is used by the

	const NANDRAD::SimulationParameter &simPara = m_project->m_simulationParameter;

	// remember all zones that require a room state model
	std::vector<const NANDRAD::Zone*> activeZones;
	// process all active zones in list of zones
	for (const NANDRAD::Zone & zone : m_project->m_zones) {
		IBK::IBK_Message( IBK::FormatString("Zone [%1] '%2':").arg(zone.m_id).arg(zone.m_displayName), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_INFO);
		try {
			// check for required parameters
			zone.checkParameters();
		}
		catch (IBK::Exception & ex) {
			throw IBK::Exception(ex, IBK::FormatString("Error in zone parametrization for zone #%1 '%2'")
				.arg(zone.m_id).arg(zone.m_displayName), FUNC_ID);
		}

		switch (zone.m_type) {
			case NANDRAD::Zone::ZT_Active : {
				IBK::IBK_Message( IBK::FormatString(" ACTIVE\n").arg(zone.m_id).arg(zone.m_displayName), IBK::MSG_CONTINUED, FUNC_ID, IBK::VL_INFO);

				// create implicit room state and room balance models
				RoomStatesModel * roomStatesModel = new RoomStatesModel(zone.m_id, zone.m_displayName);
				m_modelContainer.push_back(roomStatesModel); // transfer ownership

				// initialize room state model
				try {
					roomStatesModel->setup(zone, simPara);
				}
				catch (IBK::Exception & ex) {
					throw IBK::Exception(ex, IBK::FormatString("Error in setup of model 'RoomStatesModel' for zone #%1 '%2'")
						.arg(zone.m_id).arg(zone.m_displayName), FUNC_ID);
				}
				// also remember this model in the container with room state models m_roomStatesModelContainer
				// because we need to call yInitial() and update(y).
				m_roomStatesModelContainer.push_back(roomStatesModel);


				RoomBalanceModel * roomBalanceModel = new RoomBalanceModel(zone.m_id, zone.m_displayName);
				m_modelContainer.push_back(roomBalanceModel); // transfer ownership

				// initialize room balance model
				try {
					roomBalanceModel->setup(simPara);
				}
				catch (IBK::Exception & ex) {
					throw IBK::Exception(ex, IBK::FormatString("Error in setup of 'RoomBalanceModel' for zone #%1 '%2'")
						.arg(zone.m_id).arg(zone.m_displayName), FUNC_ID);
				}

				// also remember this model in the container with room state models m_roomBalanceModelContainer
				// because we need to call ydot().
				m_roomBalanceModelContainer.push_back(roomBalanceModel);
				// and register model for evaluation
				registerStateDependendModel(roomBalanceModel);


				// create summation models

				// solar radiation model - only needed if we have at least one construction with embedded window object
				// and an interface to us
				bool needSolarRadiationSummationModel = false;
				for (const NANDRAD::ConstructionInstance & ci : m_project->m_constructionInstances) {
					// only constructions connected to us
					if (!ci.connectedTo(zone.m_id)) continue;
					for (const NANDRAD::EmbeddedObject & eo : ci.m_embeddedObjects) {
						// only windows
						if (eo.objectType() != NANDRAD::EmbeddedObject::OT_Window) continue;
						needSolarRadiationSummationModel = true;
						break;
					}
					if (needSolarRadiationSummationModel)
						break;
				}
				if (needSolarRadiationSummationModel) {
					RoomRadiationLoadsModel * sumModel = new RoomRadiationLoadsModel(zone.m_id, zone.m_displayName);
					m_modelContainer.push_back(sumModel); // transfer ownership
					// Note: setup call not needed for a simple summation model

					// and register model for evaluation
					registerStateDependendModel(sumModel);
				}


				// thermal comfort model, does not need setup
				ThermalComfortModel * comfortModel = new ThermalComfortModel(zone.m_id, zone.m_displayName);
				m_modelContainer.push_back(comfortModel); // transfer ownership
				registerStateDependendModel(comfortModel); // and register model for evaluation

				// remember current zone
				activeZones.push_back(&zone);
			} break;


			// initialise a constant zone model
			case NANDRAD::Zone::ZT_Constant :
			case NANDRAD::Zone::ZT_Scheduled: {
				ConstantZoneModel *constantZoneModel = new ConstantZoneModel(zone.m_id, zone.m_displayName);
				m_modelContainer.push_back(constantZoneModel); // transfer ownership
				constantZoneModel->setup(zone);
				// register as state dependend (model may depend on scheduled temperature)
				registerStateDependendModel(constantZoneModel);
			} break;


			case NANDRAD::Zone::ZT_Ground:
			{
			} break;

			case NANDRAD::Zone::NUM_ZT :
				throw IBK::Exception( IBK::FormatString("Undefined or unsupported zone type in zone with id #%1")
													.arg(zone.m_id), FUNC_ID);
		} // switch
	} // for (Zones)

	m_nZones = (unsigned int) m_roomBalanceModelContainer.size();
	IBK::IBK_Message( IBK::FormatString("%1 active zones.\n").arg(m_nZones), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_INFO);
}


void NandradModel::initWallsAndInterfaces() {
	FUNCID(NandradModel::initWallsAndInterfaces);
	IBK::IBK_Message( IBK::FormatString("Initializing Constructions and Interfaces\n"), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
	IBK_MSG_INDENT;

	// *** check databases used by construction instances ***

	for (unsigned int i=0; i<m_project->m_materials.size(); ++i) {
		NANDRAD::Material & mat = m_project->m_materials[i];
		try {
			mat.checkParameters();
		}
		catch (IBK::Exception & ex) {
			throw IBK::Exception(ex, IBK::FormatString("Error initializing material #%1 '%2' (id=%3).")
								 .arg(i).arg(mat.m_displayName).arg(mat.m_id), FUNC_ID);
		}
	}

	// now process all construction types and:
	// - check for valid parameters
	// - check for valid material references (and update these material refs)

	for (unsigned int i=0; i<m_project->m_constructionTypes.size(); ++i) {
		NANDRAD::ConstructionType & ct = m_project->m_constructionTypes[i];
		try {
			ct.checkParameters(m_project->m_materials);
		}
		catch (IBK::Exception & ex) {
			throw IBK::Exception(ex, IBK::FormatString("Error initializing construction type #%1 '%2' (id=%3).")
								 .arg(i).arg(ct.m_displayName).arg(ct.m_id), FUNC_ID);
		}
	}

	for (unsigned int i=0; i<m_project->m_windowGlazingSystems.size(); ++i) {
		NANDRAD::WindowGlazingSystem & gs = m_project->m_windowGlazingSystems[i];
		try {
			gs.checkParameters();
		}
		catch (IBK::Exception & ex) {
			throw IBK::Exception(ex, IBK::FormatString("Error initializing window glazing system #%1 '%2' (id=%3).")
								 .arg(i).arg(gs.m_displayName).arg(gs.m_id), FUNC_ID);
		}
	}


	// check all construction instances for correct parameters and initialize quick-access pointer links
	for (unsigned int i=0; i<m_project->m_constructionInstances.size(); ++i) {
		NANDRAD::ConstructionInstance & ci = m_project->m_constructionInstances[i];
		try {
			ci.checkParameters(*m_project);
			// now the net surface area has been computed, which is needed in the next loop
		}
		catch (IBK::Exception & ex) {
			throw IBK::Exception(ex, IBK::FormatString("Error initializing construction instance #%1 '%2'.")
								 .arg(ci.m_id).arg(ci.m_displayName), FUNC_ID);
		}
	}
	// Prepare data for flux distribution:
	// For area-weighted flux distribution, we need to know the total area of all opaque surfaces connected to a zone.
	// So we create a dictionary for all active zones and add net surface areas for each construction interfacing
	// a zone.

	// key is zone ID, value is sum of surface areas
	std::map<unsigned int, double> zoneSurfaceAreas; // TODO: if we need this in other places as well, make it a member variable

	for (const NANDRAD::ConstructionInstance & ci : m_project->m_constructionInstances) {
		unsigned int zoneId = ci.interfaceAZoneID();
		if (zoneId != 0)
			zoneSurfaceAreas[zoneId] += ci.m_netHeatTransferArea;

		zoneId = ci.interfaceBZoneID();
		if (zoneId != 0)
			zoneSurfaceAreas[zoneId] += ci.m_netHeatTransferArea;
	}

	// process all construction instances and:
	// - check if they are connected to at least one room - otherwise
	// - check that the referenced construction type exists and create quick-access reference to construction instance

	for (unsigned int i=0; i<m_project->m_constructionInstances.size(); ++i) {
		NANDRAD::ConstructionInstance & ci = m_project->m_constructionInstances[i];
		try {

			// we now have all parameters needed to create the ConstructionStatesModel and ConstructionsBalanceModel and
			// associated boundary condition models

			/// \todo filter out constructions that have no impact on calculation results of any zones...
			///       but beware, this is tricky! Maybe it is best to keep the construction but later flag it as "unused"
			///       and simply skip over it during calculation.

			// For now, we simply check if either side has boundary conditions assigned and issue a warning if no boundary
			// conditions are present.
			if (!ci.m_interfaceA.haveBCParameters() && !ci.m_interfaceB.haveBCParameters()) {
				IBK::IBK_Message("Construction instance #%1 '%2' does not have boundary conditions on either side. To "
								 "improve performance, this construction instance should be removed, unless it is "
								 "otherwise needed.", IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
			}

			// *** construction states model ***

			ConstructionStatesModel * statesModel = new ConstructionStatesModel(ci.m_id, ci.m_displayName);
			m_modelContainer.push_back(statesModel); // transfer ownership

			IBK::IBK_Message(IBK::FormatString("Initializating construction model (id=%1)\n").arg(ci.m_id),
							 IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_INFO);
			IBK_MSG_INDENT;
			// does the entire initialization
			statesModel->setup(ci, m_project->m_simulationParameter, m_project->m_solverParameter, *m_loads);

			// remember model in states container, so that we can call update(y)
			m_constructionStatesModelContainer.push_back(statesModel);


			// *** construction balance model ***

			// now also initialize balance model - hereby re-using data from states model
			ConstructionBalanceModel * balanceModel = new ConstructionBalanceModel(ci.m_id, ci.m_displayName);
			m_modelContainer.push_back(balanceModel); // transfer ownership

			// does the entire initialization - balance model reuses data from statesModel
			balanceModel->setup(ci, zoneSurfaceAreas[ci.interfaceAZoneID()], zoneSurfaceAreas[ci.interfaceBZoneID()], statesModel);

			// register model for evaluation
			registerStateDependendModel(balanceModel);

			// remember model in balance container, so that we can call ydot(ydot)
			m_constructionBalanceModelContainer.push_back(balanceModel);


			// *** embedded objects (if any) ***

			for (const NANDRAD::EmbeddedObject & e : ci.m_embeddedObjects) {
				// Note: embedded object parameters have already been checked as part of construction instance's checkParameters()

				// switch type of object
				switch (e.objectType()) {
					case NANDRAD::EmbeddedObject::OT_Window : {
						// create embedded window model object
						WindowModel * windowModel = new WindowModel(e.m_id, e.m_displayName);
						m_modelContainer.push_back(windowModel); // transfer ownership

						// start entire initialization
						windowModel->setup(e.m_window, m_project->m_simulationParameter, ci, *m_loads);

						// register model for evaluation
						registerStateDependendModel(windowModel);
						// also store as time-dependent model (for pre-computed shading)
						if (e.m_window.m_shading.m_modelType == NANDRAD::WindowShading::MT_Precomputed)
							m_timeModelContainer.push_back(windowModel);
					}
					break;
					default: IBK_ASSERT(false);
				}
			}

		}
		catch (IBK::Exception & ex) {
			throw IBK::Exception(ex, IBK::FormatString("Error initializing construction instance #%1 '%2'.")
								 .arg(ci.m_id).arg(ci.m_displayName), FUNC_ID);
		}

	}
	m_nWalls = (unsigned int) m_constructionBalanceModelContainer.size();
	IBK::IBK_Message( IBK::FormatString("%1 construction models.\n").arg(m_nZones), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_INFO);

}


void NandradModel::initModels() {
	FUNCID(NandradModel::initModels);
	IBK::IBK_Message(IBK::FormatString("Initializing Models\n"), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
	IBK_MSG_INDENT;

	// natural ventilation
	if (!m_project->m_models.m_naturalVentilationModels.empty()) {
		IBK::IBK_Message(IBK::FormatString("Initializing natural ventilation models\n"), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
		IBK_MSG_INDENT;

		for (const NANDRAD::NaturalVentilationModel & m : m_project->m_models.m_naturalVentilationModels) {
			NANDRAD_MODEL::NaturalVentilationModel * mod = new NANDRAD_MODEL::NaturalVentilationModel(m.m_id, m.m_displayName);
			m_modelContainer.push_back(mod); // transfer ownership

			try {
				m.checkParameters();
				mod->setup(m, m_project->m_simulationParameter, m_project->m_objectLists, m_project->m_zones);
			}
			catch (IBK::Exception & ex) {
				throw IBK::Exception(ex, IBK::FormatString("Error initializing natural ventilation model (id=%1).").arg(m.m_id), FUNC_ID);
			}

			registerStateDependendModel(mod);
		}
	}

	// internal loads
	if (!m_project->m_models.m_internalLoadsModels.empty()) {
		IBK::IBK_Message(IBK::FormatString("Initializing internal loads models\n"), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
		IBK_MSG_INDENT;

		for (const NANDRAD::InternalLoadsModel & m : m_project->m_models.m_internalLoadsModels) {
			NANDRAD_MODEL::InternalLoadsModel * mod = new NANDRAD_MODEL::InternalLoadsModel(m.m_id, m.m_displayName);
			m_modelContainer.push_back(mod); // transfer ownership

			try {
				m.checkParameters();
				mod->setup(m, m_project->m_objectLists, m_project->m_zones);
			}
			catch (IBK::Exception & ex) {
				throw IBK::Exception(ex, IBK::FormatString("Error initializing internal loads model "
														   "(id=%1).").arg(m.m_id), FUNC_ID);
			}

			registerStateDependendModel(mod);
		}
	}

	// internal moisture loads
	if (!m_project->m_models.m_internalMoistureLoadsModels.empty()) {
		IBK::IBK_Message(IBK::FormatString("Initializing internal moisture loads models\n"), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
		IBK_MSG_INDENT;

		for (const NANDRAD::InternalMoistureLoadsModel & m : m_project->m_models.m_internalMoistureLoadsModels) {
			NANDRAD_MODEL::InternalMoistureLoadsModel * mod = new NANDRAD_MODEL::InternalMoistureLoadsModel(m.m_id, m.m_displayName);
			m_modelContainer.push_back(mod); // transfer ownership

			try {
				m.checkParameters(m_project->m_simulationParameter);
				mod->setup(m, m_project->m_objectLists, m_project->m_zones);
			}
			catch (IBK::Exception & ex) {
				throw IBK::Exception(ex, IBK::FormatString("Error initializing internal loads model "
														   "(id=%1).").arg(m.m_id), FUNC_ID);
			}

			registerStateDependendModel(mod);
		}
	}

	// shading control model
	if (!m_project->m_models.m_shadingControlModels.empty()) {
		IBK::IBK_Message(IBK::FormatString("Initializing shading control models\n"), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
		IBK_MSG_INDENT;

		for (NANDRAD::ShadingControlModel & m: m_project->m_models.m_shadingControlModels) {
			NANDRAD_MODEL::ShadingControlModel * mod = new NANDRAD_MODEL::ShadingControlModel(m.m_id, m.m_displayName);
			m_modelContainer.push_back(mod); // transfer ownership

			try {
				m.checkParameters(m_project->m_location.m_sensors, m_project->m_constructionInstances);
				mod->setup(m, *m_loads);
			}
			catch (IBK::Exception & ex) {
				throw IBK::Exception(ex, IBK::FormatString("Error initializing shading control model "
														   "(id=%1).").arg(m.m_id), FUNC_ID);
			}
			// model contains a controller, add to time-state model objects
			m_timeModelContainer.push_back(mod);
		}
	}

	// thermostats
	if (!m_project->m_models.m_thermostats.empty()) {
		IBK::IBK_Message(IBK::FormatString("Initializing thermostat models\n"), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
		IBK_MSG_INDENT;

		for (NANDRAD::Thermostat & m: m_project->m_models.m_thermostats) {
			NANDRAD_MODEL::ThermostatModel * mod = new NANDRAD_MODEL::ThermostatModel(m.m_id, m.m_displayName);
			m_modelContainer.push_back(mod); // transfer ownership

			try {
				m.checkParameters();
				mod->setup(m, m_project->m_objectLists, m_project->m_zones);
			}
			catch (IBK::Exception & ex) {
				throw IBK::Exception(ex, IBK::FormatString("Error initializing thermostat model "
														   "(id=%1).").arg(m.m_id), FUNC_ID);
			}
			// register model for calculation
			registerStateDependendModel(mod);
			// model contains a controller, add to time-state model objects
			m_timeModelContainer.push_back(mod);
		}
	}

	// ideal heating/cooling
	if (!m_project->m_models.m_idealHeatingCoolingModels.empty()) {
		IBK::IBK_Message(IBK::FormatString("Initializing ideal heating/cooling models\n"), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
		IBK_MSG_INDENT;

		for (NANDRAD::IdealHeatingCoolingModel & m: m_project->m_models.m_idealHeatingCoolingModels) {
			NANDRAD_MODEL::IdealHeatingCoolingModel * mod = new NANDRAD_MODEL::IdealHeatingCoolingModel(m.m_id, m.m_displayName);
			m_modelContainer.push_back(mod); // transfer ownership

			try {
				m.checkParameters();
				mod->setup(m, m_project->m_objectLists, m_project->m_zones);
			}
			catch (IBK::Exception & ex) {
				throw IBK::Exception(ex, IBK::FormatString("Error initializing ideal heating/cooling model (id=%1).").arg(m.m_id), FUNC_ID);
			}
			// register model for calculation
			registerStateDependendModel(mod);
			m_timeModelContainer.push_back(mod); // also a time-state model because of stepCompleted()
		}
	}

	// ideal surface heating
	if (!m_project->m_models.m_idealSurfaceHeatingCoolingModels.empty()) {
		IBK::IBK_Message(IBK::FormatString("Initializing ideal surface heating models\n"), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
		IBK_MSG_INDENT;

		for (NANDRAD::IdealSurfaceHeatingCoolingModel & m: m_project->m_models.m_idealSurfaceHeatingCoolingModels) {
			NANDRAD_MODEL::IdealSurfaceHeatingCoolingModel * mod = new NANDRAD_MODEL::IdealSurfaceHeatingCoolingModel(m.m_id, m.m_displayName);
			m_modelContainer.push_back(mod); // transfer ownership

			try {
				m.checkParameters(m_project->m_zones);
				mod->setup(m, m_project->m_objectLists);
			}
			catch (IBK::Exception & ex) {
				throw IBK::Exception(ex, IBK::FormatString("Error initializing ideal heating/cooling model (id=%1).").arg(m.m_id), FUNC_ID);
			}
			// register model for calculation
			registerStateDependendModel(mod);
		}
	}

	// ideal pipe register models
	if (!m_project->m_models.m_idealPipeRegisterModels.empty()) {
		IBK::IBK_Message(IBK::FormatString("Initializing ideal pipe register models\n"), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
		IBK_MSG_INDENT;

		for (NANDRAD::IdealPipeRegisterModel & m: m_project->m_models.m_idealPipeRegisterModels) {
			NANDRAD_MODEL::IdealPipeRegisterModel * mod = new NANDRAD_MODEL::IdealPipeRegisterModel(m.m_id, m.m_displayName);
			m_modelContainer.push_back(mod); // transfer ownership

			try {
				m.checkParameters(m_project->m_zones);
				mod->setup(m, m_project->m_objectLists);
			}
			catch (IBK::Exception & ex) {
				throw IBK::Exception(ex, IBK::FormatString("Error initializing ideal heating/cooling model (id=%1).").arg(m.m_id), FUNC_ID);
			}
			// register model for calculation
			registerStateDependendModel(mod);
		}
	}

	// summation models
	if (!m_project->m_models.m_heatLoadSummationModels.empty()) {
		IBK::IBK_Message(IBK::FormatString("Initializing heat load summation models\n"), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
		IBK_MSG_INDENT;

		for (NANDRAD::HeatLoadSummationModel & m: m_project->m_models.m_heatLoadSummationModels) {
			NANDRAD_MODEL::HeatLoadSummationModel * mod = new NANDRAD_MODEL::HeatLoadSummationModel(m.m_id, m.m_displayName);
			m_modelContainer.push_back(mod); // transfer ownership

			try {
				m.checkParameters();
				mod->setup(m, m_project->m_objectLists);
			}
			catch (IBK::Exception & ex) {
				throw IBK::Exception(ex, IBK::FormatString("Error initializing heat load summation model (id=%1).").arg(m.m_id), FUNC_ID);
			}
			// register model for calculation
			registerStateDependendModel(mod);
		}
	}

	// network adapter models
	if (!m_project->m_models.m_networkInterfaceAdapterModels.empty()) {
		IBK::IBK_Message(IBK::FormatString("Initializing network interface adapter models models\n"), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
		IBK_MSG_INDENT;

		for (NANDRAD::NetworkInterfaceAdapterModel & m: m_project->m_models.m_networkInterfaceAdapterModels) {
			NANDRAD_MODEL::NetworkInterfaceAdapterModel * mod = new NANDRAD_MODEL::NetworkInterfaceAdapterModel(m.m_id, m.m_displayName);
			m_modelContainer.push_back(mod); // transfer ownership

			try {
				m.checkParameters();
				mod->setup(m);
			}
			catch (IBK::Exception & ex) {
				throw IBK::Exception(ex, IBK::FormatString("Error initializing network interface adapter model (id=%1).").arg(m.m_id), FUNC_ID);
			}
			// register model for calculation
			registerStateDependendModel(mod);
		}
	}
}


void NandradModel::initObjectLists() {
	FUNCID(NandradModel::initObjectLists);
	IBK::IBK_Message(IBK::FormatString("Initializing Object Lists\n"), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
	IBK_MSG_INDENT;

	// All object lists are checked for valid parameters.
	// All ID groups are resolved and populated with lists of available model object IDs.
	// Afterwards, it is possible to just use NANDRAD::ObjectList::m_ids when checking for IDs, however,
	// testing with contains() may be faster (especially, when all IDs is set).

	// NOTE: We must not modify the object list vector here (add/remove entries), because other objects/models
	//		 have already stored persistent pointers to individual object list entries.
	for (unsigned int i=0; i<m_project->m_objectLists.size(); ++i) {
		NANDRAD::ObjectList & objectlist = m_project->m_objectLists[i];

		// retrieve reference type
		const NANDRAD::IDGroup  objectIDs = objectlist.m_filterID;

		// check id intervals not allowing intervals including id 0, and valid range definition
		for (auto interval : objectIDs.m_idIntervals) {
			unsigned int lowerId = interval.first;
			unsigned int upperId = interval.second;
			if (lowerId == 0)
				throw IBK::Exception(IBK::FormatString("Error initializing object list #%1 '%2': id 0 is not allowed in range.")
					.arg(i).arg(objectlist.m_name), FUNC_ID);
			if (upperId < lowerId)
				throw IBK::Exception(IBK::FormatString("Error initializing object list #%1 '%2': incorrect interval definition")
					.arg(i).arg(objectlist.m_name), FUNC_ID);
		}

		// special treatment for network elements
		if(objectlist.m_referenceType == NANDRAD::ModelInputReference::MRT_NETWORKELEMENT)  {
			// resolve IDs by searching through model objects
			std::set<unsigned int> resolvedIds;
			// search model id inside network model container
			for(const NANDRAD::HydraulicNetwork &network : m_project->m_hydraulicNetworks) {
				// find all network elements
				for(const NANDRAD::HydraulicNetworkElement &e : network.m_elements) {
					// element coverd requested id space
					if(objectlist.m_filterID.contains(e.m_id))
						resolvedIds.insert(e.m_id);
				}
			}
			// clear id filter
			objectlist.m_filterID.m_ids.clear();
			objectlist.m_filterID.m_ids.insert(resolvedIds.begin(), resolvedIds.end());
			// continue in loop
			// issue a warning if the object list resolved no valid IDs
			if (objectlist.m_filterID.m_ids.empty()) {
				IBK::IBK_Message(IBK::FormatString("Object list '%1' did not select any valid/existing objects.")
								 .arg(objectlist.m_name), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
			}
			continue;
		}

		// resolve IDs by searching through model objects
		std::set<unsigned int> resolvedIds;
		// insert all models that match current definition
		for (unsigned int i = 0; i < m_modelContainer.size(); ++i) {
			const AbstractModel *model = m_modelContainer[i];
			// skip models with wrong reference type
			if (model->referenceType() != objectlist.m_referenceType)
				continue;
			// fill model ids that are inside the defined id space
			if (objectlist.m_filterID.contains(model->id()) ) {
				// fill the resolved list
				resolvedIds.insert(model->id());
				continue;
			}
			// TODO: special treatment of networks
		}
		// fill the id filter, but only add resolved IDs (clear existing ones first)!
		objectlist.m_filterID.m_ids.clear();
		objectlist.m_filterID.m_ids.insert(resolvedIds.begin(), resolvedIds.end());

		// set 0-id for schedules and location
		if (objectlist.m_referenceType == NANDRAD::ModelInputReference::MRT_LOCATION ||
			objectlist.m_referenceType == NANDRAD::ModelInputReference::MRT_SCHEDULE)
		{
			objectlist.m_filterID.m_ids.insert(0);
		}
		else {
			// issue a warning if the object list resolved no valid IDs
			if (objectlist.m_filterID.m_ids.empty()) {
				IBK::IBK_Message(IBK::FormatString("Object list '%1' did not select any valid/existing objects.")
								 .arg(objectlist.m_name), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
			}
		}
	}

}


void NandradModel::initNetworks() {
	if (!m_project->m_hydraulicNetworks.empty()) {
		FUNCID(NandradModel::initNetworks);
		IBK::IBK_Message(IBK::FormatString("Initializing Networks\n"), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
		IBK_MSG_INDENT;
		// store all used zone ids that are used as node
		std::set<unsigned int> zoneNodeIds;
		// process all networks and create NM::HydraulicNetworkModel instances
		for (NANDRAD::HydraulicNetwork & nw : m_project->m_hydraulicNetworks) {
			IBK::IBK_Message(IBK::FormatString("Initializing network '%1' (#%2)\n").arg(nw.m_displayName).arg(nw.m_id),
							 IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
			IBK_MSG_INDENT;
			// check network parameters
			try {
				nw.checkParameters(*m_project, zoneNodeIds);
			}
			catch (IBK::Exception & ex) {
				throw IBK::Exception(ex, IBK::FormatString("Error initializing hydraulic network with id #%1.")
									.arg(nw.m_id), FUNC_ID);
			}
			// create a network model object
			HydraulicNetworkModel * nwmodel = new HydraulicNetworkModel(nw, m_project->m_models.m_thermostats, nw.m_id, nw.m_displayName,
																		m_project->m_solverParameter.m_para[NANDRAD::SolverParameter::P_HydraulicNetworkAbsTol].value,
																		m_project->m_solverParameter.m_para[NANDRAD::SolverParameter::P_HydraulicNetworkMassFluxScale].value);
			m_modelContainer.push_back(nwmodel); // transfer ownership
			// initialize
			nwmodel->setup();
			// register model for evaluation
			registerStateDependendModel(nwmodel);
			// register model as time dependent (to backup Newton solution as starting value for next call)
			m_timeModelContainer.push_back(nwmodel);
			// add thermal network states model
			if (nw.m_modelType == NANDRAD::HydraulicNetwork::MT_ThermalHydraulicNetwork
				|| nw.m_modelType == NANDRAD::HydraulicNetwork::MT_AirNetwork) {
				ThermalNetworkStatesModel *statesModel = new ThermalNetworkStatesModel(nw.m_id, nw.m_displayName);
				m_modelContainer.push_back(statesModel); // transfer ownership
				// initialize
				statesModel->setup(nw, *nwmodel, m_project->m_simulationParameter);
				// register model as time dependent (spline values update needed for each time step)
				m_timeModelContainer.push_back(statesModel);
				// add to thermal network states container
				m_networkStatesModelContainer.push_back(statesModel);

				// add thermal network balance model
				ThermalNetworkBalanceModel *balanceModel = new ThermalNetworkBalanceModel(nw.m_id, nw.m_displayName);
				m_modelContainer.push_back(balanceModel); // transfer ownership
				// initialize
				balanceModel->setup(statesModel);
				// register model for evaluation
				registerStateDependendModel(balanceModel);
				// add to thermal network balance container
				m_networkBalanceModelContainer.push_back(balanceModel);
			}
		}
	}
	m_nNetworks = (unsigned int) m_networkBalanceModelContainer.size();
}


void NandradModel::initOutputs(bool restart) {
	FUNCID(NandradModel::initOutputs);
	IBK::IBK_Message(IBK::FormatString("Initializing Outputs\n"), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
	IBK_MSG_INDENT;

	/// \todo if output time unit is missing, define one based on simulation duration
	if (m_project->m_outputs.m_timeUnit.base_id() == 0) {
		m_project->m_outputs.m_timeUnit.set("h"); // for now hours
	}

	try {
		m_outputHandler = new OutputHandler; // we own the model, memory is released in destructor
		m_outputHandler->setup(restart, *m_project, m_dirs.m_resultsDir);

		// append the output file objects to the model container, so that variables can be resolved
		m_modelContainer.insert(m_modelContainer.end(),
								m_outputHandler->m_outputFiles.begin(),
								m_outputHandler->m_outputFiles.end()); // transfers ownership

		// NOTE: output file objects are not evaluated as other models in updateStateDependentModels().
		//       Hence, we do not

		for (NANDRAD_MODEL::OutputFile * of : m_outputHandler->m_outputFiles) {
			// Only add those output files that have at least one integral value.
			if (of->haveIntegrals())
				m_timeModelContainer.push_back(of);
		}
	}
	catch (IBK::Exception & ex) {
		throw IBK::Exception(ex, "Error initializing outputs.", FUNC_ID);
	}
}


void NandradModel::initModelDependencies() {
	FUNCID(NandradModel::initModelDependencies);

	// *** complete initialization for all models ***

	IBK::StopWatch timer;

	// *** initializing model results ***

	IBK::IBK_Message(IBK::FormatString("Initializing all model results\n"), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
	std::unique_ptr<IBK::MessageIndentor> indent(new IBK::MessageIndentor);

	// The key is of type QuantityDescription, a simple class that identifies a variable based on reference type and id
	// (both addressing an object) and variable name (identifying the variable of the object).
	// The map can be used to quickly find the object the holds a required result variable.
	// It maps QuantityDescription (i.e. global identification of a result variable) to the object that provides this variable.
	// Note: the object's reference type must not necessarily match the reference type stored in the QuantityDescription.
	std::map<QuantityDescription, AbstractModel*> modelResultReferences;

	// prepare for parallelization - get number of threads and prepare thread-storage vectors
#if defined(_OPENMP)
	// for openMP we need to collect vector within each loop and merge them into the central map together - this avoids synchronization overhead during runtime
	// since each thread can operate in its own memory vector
	std::vector<std::vector<std::pair<QuantityDescription, AbstractModel*> > > modelResultReferencesVec(m_numThreads);
	std::vector<std::string> threadErrors(m_numThreads);
#endif

#if defined(_OPENMP)
#pragma omp parallel for schedule(static,200)
#endif
	for (int i = 0; i < (int)m_modelContainer.size(); ++i) { // omp loop variables must be int's for Visual Studio

		// progress is handled by master thread only
#if defined(_OPENMP)
		if (omp_get_thread_num()==0) {
#else
		{
#endif
			if (timer.intervalCompleted()) // side-effect guarded by _OPENMP ifdef
				IBK::IBK_Message(IBK::FormatString("  Loop 1: %1 %% done\n").arg(i*100.0 / m_modelContainer.size()), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
		} // end master section

		// currentModel is the model that we current look up input references for
		AbstractModel * currentModel = m_modelContainer[(size_t)i];
		try {
			// let models initialize their results, i.e. generate information on computed results
			currentModel->initResults(m_modelContainer);

			// ask model/object instance for published results
			std::vector<QuantityDescription> resDescs;
			currentModel->resultDescriptions(resDescs);
			// now process all published variables
			for (unsigned int j = 0; j < resDescs.size(); ++j) {
				QuantityDescription resRef = resDescs[j];
				// now create our "key" data type for the lookup map

				// If quantity reference does not yet have a referenceTyp assigned,
				// store additional information for object lookup
				// This should be the standard for most model objects and their results.
				if (resRef.m_referenceType == NANDRAD::ModelInputReference::NUM_MRT) {
					resRef.m_id = currentModel->id();
					resRef.m_referenceType = currentModel->referenceType();
				}

#if !defined(_OPENMP)
				IBK_FastMessage(IBK::VL_DETAILED)(IBK::FormatString("%1(id=%2).%3 [%4]\n")
												  .arg(NANDRAD::KeywordList::Keyword("ModelInputReference::referenceType_t",resRef.m_referenceType))
												  .arg(resRef.m_id).arg(resRef.m_name).arg(resRef.m_unit), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_DETAILED);
#endif

#if defined(_OPENMP)
				// store in thread-specific vector
				modelResultReferencesVec[omp_get_thread_num()].push_back(std::make_pair(resRef, currentModel));
#else
				// ensure that this variable is not yet existing in our map, this would be a programming
				// error, since global uniqueness would not be guaranteed
				IBK_ASSERT(modelResultReferences.find(resRef) == modelResultReferences.end());

				// single-core run, store directly in map
				modelResultReferences[resRef] = currentModel;
#endif
			}
		}
		catch (IBK::Exception &ex) {
#if defined(_OPENMP)
			// OpenMP code may not throw exceptions beyond parallel region, hence only store errors in error list for
			// later evaluation
			threadErrors[omp_get_thread_num()] += ex.msgStack() + "\n" + IBK::FormatString("Error initializing results "
																						   "for model %1 with id #%2!\n")
																						   .arg(currentModel->ModelIDName())
																						   .arg(currentModel->id()).str();
#else
			throw IBK::Exception(ex, IBK::FormatString("Error initializing results for model %1 with id #%2!")
				.arg(currentModel->ModelIDName())
				.arg(currentModel->id()),
				FUNC_ID);
#endif // _OPENMP
		}
	} // end - pragma parallel omp for

#if defined(_OPENMP)
	// error checking
	for (int i=0; i<m_numThreads; ++i)
		if (!threadErrors[i].empty()) {
			throw IBK::Exception(threadErrors[i], FUNC_ID);
		}

	// merge thread-specific vectors into global map
	for (unsigned int i=0; i<(unsigned int)m_numThreads; ++i) {
		IBK::IBK_Message(IBK::FormatString("  Loop 1: merging %1 model result references from thread #%2\n")
						 .arg(modelResultReferencesVec[i].size()).arg(i), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
		for (unsigned int j=0; j<modelResultReferencesVec[i].size(); ++j)
			modelResultReferences[ modelResultReferencesVec[i][j].first] = modelResultReferencesVec[i][j].second;
	}
#endif

	// *** check for ambiguities in vector-valued model results

	std::map<std::string, std::set<unsigned int> > vectorValuedModelResults;
	for (auto refDesc : modelResultReferences) {
		// we only check model results (because they can have vector valued results that
		// may have ambiguous ids - since users may define model parameter blocks
		// with overlapping object lists
		if (refDesc.first.m_referenceType != NANDRAD::ModelInputReference::MRT_MODEL)
			continue;
		// only process vector-valued results with indexes of type ModelID
		if (refDesc.first.m_indexKeyType != VectorValuedQuantityIndex::IK_ModelID)
			continue;
		std::set<unsigned int> & idSet = vectorValuedModelResults[refDesc.first.m_name];
		for (unsigned int id : refDesc.first.m_indexKeys) {
			// check for existance of ID, and add if not yet existing
			if (idSet.find(id) != idSet.end())
				throw IBK::Exception(IBK::FormatString("Ambiguous model parametrization block for model with id=%1 resulting "
													   "from two (overlapping) objects lists which reference the same object "
													   "(results for object with ID %2 are generated twice).")
									 .arg(refDesc.first.m_id).arg(id), FUNC_ID);
			idSet.insert(id);
		}
	}


	// *** initializing model input references ***

	delete indent.release();
	IBK::IBK_Message(IBK::FormatString("Initializing all model input references\n"), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
	indent.reset(new IBK::MessageIndentor);

	// This set stores all inputs requested by all model objects - this can be a large list
	std::set<InputReference> globalInputRefList;

#if defined(_OPENMP)
#pragma omp parallel for schedule(static,200)
#endif
	for (int i = 0; i < (int) m_modelContainer.size(); ++i) {
		// progress is handled by master thread only
#if defined(_OPENMP)
		if (omp_get_thread_num()==0) {
#else
		{
#endif
			if (timer.intervalCompleted()) // Note: side-effect guarded by _OPENMP ifdef
				IBK::IBK_Message(IBK::FormatString("  Loop 2: %1 %% done\n").arg(i*100.0 / m_modelContainer.size()), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
		} // end master section

		AbstractModel * currentModel = m_modelContainer[(size_t)i];
		// currentModel is the model that we current look up input references for
		AbstractStateDependency * currentStateDependency = dynamic_cast<AbstractStateDependency *> (m_modelContainer[(size_t)i]);
		// skip all models that are not state-dependent and have no input requirements
		if (currentStateDependency == nullptr)
			continue;

		try {
			// tell model to initialize its inputs
			currentStateDependency->initInputReferences(m_modelContainer);

			// request published input variables from model instance
			std::vector<InputReference> inputRefs;
			currentStateDependency->inputReferences(inputRefs);

#if defined(_OPENMP)
#pragma omp critical
{
#endif
			for (const InputReference & iref : inputRefs) {
				// ignore invalid/unused input references
				if (iref.m_referenceType != NANDRAD::ModelInputReference::NUM_MRT)
					globalInputRefList.insert(iref);
			}
#if defined(_OPENMP)
}
#endif

			std::vector<const double*> resultValueRefs;
			std::vector<QuantityDescription> resultQuantityDescs;

			// process and lookup all of the variables variables
			for (unsigned int j = 0; j < inputRefs.size(); ++j) {

				const InputReference &inputRef = inputRefs[j];
				QuantityDescription quantityDesc; // here we store the quantity information of the variable

				// if inputRef.m_referenceType = NUM_MRT skip this and set nullptr directly
				if (inputRef.m_referenceType == NANDRAD::ModelInputReference::NUM_MRT) {
					resultValueRefs.push_back(nullptr);
					resultQuantityDescs.push_back(quantityDesc);
					continue;
				}

				// now lookup address of requested variable
				const double * srcVarAddress = nullptr;
				AbstractModel * srcObject = nullptr;

				// 1. FMU interface variables

				// First check FMU import model - it may override variables by providing variables for
				// exactly the same InputReference as those of other models. E.g., an FMU import
				// may generate a variable Zone[13].AirTemperature and thus override the air temperature
				// variable generated by the zone model (instance with id=13) itself. All models using this
				// temperature will use the variable from the FMI import model, instead.

				// prevent Option A (direct feedback) and do not allow fmiInputOutput model to get its own results!
				if (m_fmiInputOutput != nullptr && m_fmiInputOutput != currentModel)
					srcVarAddress = m_fmiInputOutput->resolveResultReference(inputRef, quantityDesc);


				std::string lookupErrorMessage;
				// 2. regular lookup (only if not yet found in FMU)
				if (srcVarAddress == nullptr) {
					// compose search key - for vector valued quantities we ignore the index in ValueReference,
					// since we only want to find the object that actually provides the *variable*
					QuantityDescription valueRef;
					valueRef.m_id = inputRef.m_id;
					valueRef.m_referenceType = inputRef.m_referenceType;
					valueRef.m_name = inputRef.m_name.m_name;

					std::map<QuantityDescription, AbstractModel*>::const_iterator it = modelResultReferences.find(valueRef);
					if (it != modelResultReferences.end()) {
						// remember source object's pointer, to create the dependency graph afterwards
						srcObject = it->second;
						quantityDesc = it->first;
						// request the address to the requested variable from the source object
						try {
							srcVarAddress = srcObject->resultValueRef(inputRef);
							if (srcVarAddress == nullptr)
								throw IBK::Exception("Object returns nullptr reference for exported variable.", FUNC_ID);
						} catch (IBK::Exception & ex) {
							// in case of error, simply cache a warning to be used in the error message if this variable
							// is required
							lookupErrorMessage = IBK::FormatString("Error resolving variable reference %1(id=%2).%3. %4")
								.arg(NANDRAD::KeywordList::Keyword("ModelInputReference::referenceType_t",valueRef.m_referenceType))
								.arg(valueRef.m_id).arg(inputRef.m_name.encodedString()).arg(ex.what()).str();
						}
					}
				}

				// 3. schedule lookup

				// Schedules are defined for object lists, which in turn reference objects via reference types and
				// id groups.
				// Schedule object has a function to lookup matching schedules, so simply call upon this function
				// and let the schedule object resolve the schedule model.
				//
				// Example: a schedule may define "HeatingSetPoint" for object list "All zones" and thus it would
				//          resolve an input reference (ZONE, "HeatingSetPoint", id=12), but also resolve a request
				//          based on the object list name "All zones.HeatingSetPoint". Both request would actually
				//          get the same memory storage pointer returned.

				if (srcVarAddress == nullptr) {
					srcVarAddress = m_schedules->resolveResultReference(inputRef, quantityDesc);
				}


				// If we didn't find a pointer to the result variable, yet, raise an exception if the requested variable
				// was a required input
				if (srcVarAddress == nullptr) {
					if (inputRef.m_required) {
						// error: reference was not resolved
						if (lookupErrorMessage.empty())
							throw IBK::Exception(IBK::FormatString("Could not resolve reference to quantity '%1' (reftype '%2', object id #%3) needed by %4 with id #%5!")
								.arg(inputRef.m_name.m_name)
								.arg(NANDRAD::KeywordList::Keyword("ModelInputReference::referenceType_t", inputRef.m_referenceType))
								.arg(inputRef.m_id)
								.arg(NANDRAD::KeywordList::Keyword("ModelInputReference::referenceType_t", m_modelContainer[(size_t)i]->referenceType()))
								.arg(m_modelContainer[(size_t)i]->id()), FUNC_ID);
						else {
							throw IBK::Exception(lookupErrorMessage, FUNC_ID);
						}
					}
					else {
						// if not required, tell the model object that we do not have such an input by giving a nullptr;
						// the model must handle this appropriately
						resultValueRefs.push_back(nullptr);
						resultQuantityDescs.push_back(quantityDesc);
					}
				}
				else {
					// tell model the persistent memory location of the requested input
					resultValueRefs.push_back(srcVarAddress);
					resultQuantityDescs.push_back(quantityDesc);
					// register this model as dependency, but only if providing model was an object in the model graph
					// and the input variable is not a constant
					if (srcObject != nullptr && !quantityDesc.m_constant) {
						// add a graph element
						const ZEPPELIN::DependencyObject *sourceObject = dynamic_cast<ZEPPELIN::DependencyObject*>(srcObject);
						if (sourceObject != nullptr)
							currentStateDependency->dependsOn(*sourceObject);
							// Note: we are calling the const-variant, so that sourceObject won't
							//       be modified and this function remains thread-safe. The parents are set after this
							//       parallel for-loop has completed.
					}
				}
			} // input value refs loop

			// set collected value refs in object
			currentStateDependency->setInputValueRefs(resultQuantityDescs, resultValueRefs);
		}
		catch (IBK::Exception &ex) {
#if defined(_OPENMP)
			// OpenMP code may not throw exceptions beyond parallel region, hence only store errors in error list for
			// later evaluation
			threadErrors[omp_get_thread_num()] += ex.msgStack() + "\n" + IBK::FormatString("Error initializing results "
																						   "for model #%1 with id #%2!\n")
																						   .arg(currentModel->ModelIDName())
																						   .arg(currentModel->id()).str();
#else
			throw IBK::Exception(ex, IBK::FormatString("Error initializing input references for model '%1' with id #%2!")
				.arg(currentModel->ModelIDName())
				.arg(currentModel->id()),
				FUNC_ID);
#endif // _OPENMP
		}
	} // end - pragma parallel omp for

	// TODO : OpenMP
//#if defined(_OPENMP)
//	// error checking
//	for (int i=0; i<m_numThreads; ++i)
//		if (!threadErrors[i].empty()) {
//			throw IBK::Exception(threadErrors[i], FUNC_ID);
//		}

//	for (unsigned int i=0; i<(unsigned int)m_numThreads; ++i) {
//		for (unsigned int j=0; j<modelInputReferencesVec[i].size(); ++j)
//			modelInputReferences[ modelInputReferencesVec[i][j].first] = modelInputReferencesVec[i][j].second;
//	}
//#endif

	// set backward connections for all objects before initializing model graph
	// we will need parents for identifying single sequential connections
	for (unsigned int i = 0; i < m_modelContainer.size(); ++i) {
		// progress is handled by master thread only
		AbstractStateDependency * currentStateDependency = dynamic_cast<AbstractStateDependency*>(m_modelContainer[i]);

		// skip all pure time dependend models
		if (currentStateDependency == nullptr)
			continue;

		currentStateDependency->updateParents();
	}


	// now all model objects have pointers to their input variables stored and can access these variable values


#if 0
	// *** resolve input references for all ODE states and balance models
	for (unsigned int i = 0; i < m_ODEStatesAndBalanceModelContainer.size(); ++i) {
		AbstractODEStatesModel* statesModel = m_ODEStatesAndBalanceModelContainer[i].first;
		AbstractODEBalanceModel* balanceModel = m_ODEStatesAndBalanceModelContainer[i].second;
		// connect states
		const double *y = balanceModel->ODEStatesValueRef();
		IBK_ASSERT(y != NULL);
		// set states reference
		statesModel->setODEStatesInputValueRef(y);
	}
#endif



	// dump input reference list to file
	std::shared_ptr<std::ofstream> inputRefList(
		IBK::create_ofstream(m_dirs.m_varDir / "input_reference_list.txt")
	);

	(*inputRefList) << std::setw(50) << std::left << "Variable name" << '\t'
					<< std::setw(20) << std::left << "Source object id(s)" << '\t'
					<< std::setw(20) << std::left << "Vector indexes/ids" << '\n';

	// we need to generate the following information for users:
	// for scalar variables
	// - full variable name: <reftype>.<varname>
	// - list of objects (IDs) that provide the variable
	//
	// for vector-valued variables
	// - full variable base name: <reftype>.<vector-varname>
	// - object id that provides this variable
	// - possible ids/indexes to request a value

	// We dump this out in a table:
	//   fullvarName     objectIDs     possible IDs/Indexes
	//
	// first, we create a sorted container that collects input references by variable name, then we process all of these
	// variable names, and handle scalar/vector valued quantities separately
	std::map<std::string, std::vector<InputReference> > sortedList;
	for (const InputReference & iref : globalInputRefList) {
		std::string fullVarName = IBK::FormatString("%1.%2")
			.arg(NANDRAD::KeywordList::Keyword("ModelInputReference::referenceType_t", iref.m_referenceType))
			.arg(iref.m_name.m_name).str();
		sortedList[fullVarName].push_back(iref);
	}

	// now process all variable names and collect info for scalar/vector-valued input refs
	for (const auto & varNameData : sortedList) {
		// here we store the object IDs of scalar variables
		std::set<unsigned int> objectIDs;
		// here we store the vector indexes/IDs for vector-valued variables,
		// key = objectId, value = vector-value-IDs/indexes
		std::map<unsigned int, std::vector<unsigned int> > vectorIndexes;
		for (const InputReference & iref : varNameData.second) {
			// if vector-valued, write directly
			if (iref.m_name.m_index != -1) {
				vectorIndexes[iref.m_id].push_back((unsigned int)iref.m_name.m_index);
			}
			else {
				// first only collect object IDs
				objectIDs.insert(iref.m_id);
			}
		}
		// if we have at least one object ID, write line for scalar variables
		if (!objectIDs.empty()) {
			std::string objectIDstr = IBK::join_numbers(objectIDs, ',');
			std::stringstream strm;
			strm << std::setw(50) << std::left << varNameData.first << '\t'
				 << std::setw(20) << std::left << objectIDstr << "\t\n";
			IBK::IBK_Message( IBK::FormatString("%1").arg(strm.str()), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_INFO);
			(*inputRefList) << strm.rdbuf();
		}
		if (!vectorIndexes.empty()) {
			for (const auto & vectorVar : vectorIndexes) {
				std::string vectorIDstr = IBK::join_numbers(vectorVar.second, ',');
				std::stringstream strm;
				strm << std::setw(50) << std::left << varNameData.first << '\t'
					 << std::setw(20) << std::left << vectorVar.first << '\t'
					 << std::setw(20) << std::left << vectorIDstr << '\n';
				IBK::IBK_Message( IBK::FormatString("%1").arg(strm.str()), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_INFO);
				(*inputRefList) << strm.rdbuf();
			}
		}
	}
	inputRefList->flush();
	inputRefList->close();

}


void NandradModel::initModelGraph() {
	FUNCID(NandradModel::initModelGraph);

	// *** create state dependency graph ***

	IBK::IBK_Message(IBK::FormatString("Creating Dependency Graph for %1 State-Dependent Models\n").arg((int)
		m_unorderedStateDependencies.size()), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);

	try {
		std::vector<ZEPPELIN::DependencyObject*> stateDepObjects(m_unorderedStateDependencies.begin(),
			m_unorderedStateDependencies.end());
		try {
			// set objects in graph and compute evaluation order
			m_stateDependencyGraph.setObjects(stateDepObjects, m_stateDependencyGroups);
		}
		catch (std::exception &ex) {
			throw IBK::Exception(ex.what(), FUNC_ID);
		}

		// retrieve parallel groups
		const std::vector<ZEPPELIN::DependencyGraph::ParallelObjects> & orderedStateDependentObjects
			= m_stateDependencyGraph.orderedParallelObjects();

		// loop over all parallel groups in ordered vectors
		for (unsigned int i = 0; i < orderedStateDependentObjects.size(); ++i) {
			const ZEPPELIN::DependencyGraph::ParallelObjects & objects = orderedStateDependentObjects[i];

			// insert a new vector of AbstractTimeStateObjects
			m_orderedStateDependentSubModels.push_back(ParallelStateObjects());

			ParallelStateObjects & objs = m_orderedStateDependentSubModels.back();
			// loop over all parallel groups of one group vector
			for (unsigned int j = 0; j < objects.size(); ++j) {
				// select all objects of one group and sort into subModels-vector
				const ZEPPELIN::DependencyGroup* objGroup = dynamic_cast<const ZEPPELIN::DependencyGroup*>(objects[j]);

				// error
				IBK_ASSERT(objGroup != NULL);

				// construct a state object group
				StateModelGroup *stateModelGroup = new StateModelGroup;
				stateModelGroup->init(*objGroup, m_project->m_solverParameter);
				// add to group container
				m_stateModelGroups.insert(stateModelGroup);
				// register group as state dependend object
				m_stateModelContainer.push_back(stateModelGroup);
				// insert into ordered vector
				objs.push_back(stateModelGroup);
			}
		}
	}
	catch (IBK::Exception &ex) {
		throw IBK::Exception(ex, IBK::FormatString("Error creating ordered state dependency graph!"),
			FUNC_ID);
	}

	// insert head and tail
	// head models are prepended at start of queue
	m_orderedStateDependentSubModels.insert(m_orderedStateDependentSubModels.begin(),
		m_orderedStateDependentSubModelsHead.begin(),
		m_orderedStateDependentSubModelsHead.end());

	// tail models are appended at end of queue
	m_orderedStateDependentSubModels.insert(m_orderedStateDependentSubModels.end(),
		m_orderedStateDependentSubModelsTail.begin(),
		m_orderedStateDependentSubModelsTail.end());
}


void NandradModel::initOutputReferenceList() {
	FUNCID(NandradModel::initOutputReferenceList);

	// we need to generate the following information for users:
	// for scalar variables
	// - full variable name: <reftype>.<varname>
	// - list of objects (IDs) that provide the variable
	//
	// for vector-valued variables
	// - full variable base name: <reftype>.<vector-varname>
	// - object id that provides this variable
	// - possible ids/indexes to request a value

	// We dump this out in a table:
	//   fullvarName     objectIDs     possible IDs/Indexes
	//
	// Also, we create a variable substitution map, that maps the prefix of an output's variable
	// name as it appears in the output file to the dispay name of the object that this variable
	// originates from.


	IBK::IBK_Message( IBK::FormatString("Initializing Output Quantity List\n"), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
	{
		IBK_MSG_INDENT;

		// generate and dump calculation results of all models
		std::map<std::string, std::vector<QuantityDescription> > refDescs;
		for (unsigned int i=0; i<m_modelContainer.size(); ++i) {
			AbstractModel * currentModel = m_modelContainer[i];
			currentModel->variableReferenceSubstitutionMap(m_varSubstitutionMap); // for most model instances, this does nothing

			NANDRAD::ModelInputReference::referenceType_t refType = currentModel->referenceType();
			// skip models that do not generate outputs
			if (refType == NANDRAD::ModelInputReference::NUM_MRT)
				continue;
			std::vector<QuantityDescription> varDescs;
			currentModel->resultDescriptions(varDescs);
			try {
				std::string refTypeName = NANDRAD::KeywordList::Keyword("ModelInputReference::referenceType_t", refType);
				for (unsigned int j=0; j<varDescs.size(); ++j) {
					std::stringstream fullVariableName; // refobject type + variable name
					if (varDescs[j].m_size == 0)
						continue;
					// if different reference type is given, use this reference type instead
					if (varDescs[j].m_referenceType != NANDRAD::ModelInputReference::NUM_MRT)
						fullVariableName << NANDRAD::KeywordList::Keyword("ModelInputReference::referenceType_t", varDescs[j].m_referenceType)
									<< "." << varDescs[j].m_name;
					else {
						// update object id in quantity description
						varDescs[j].m_id = currentModel->id();
						fullVariableName << refTypeName << "." << varDescs[j].m_name;
					}
					refDescs[fullVariableName.str()].push_back( varDescs[j] );

					// for schedules add reduced definition
					if (refType == NANDRAD::ModelInputReference::MRT_SCHEDULE) {
						std::string reducedName = varDescs[j].m_name;
						std::string::size_type pos = reducedName.rfind(':');
						IBK_ASSERT(pos != std::string::npos);
						reducedName = reducedName.substr(pos + 1);
						// create a reduced description
						std::stringstream reducedDescription;
						reducedDescription << refTypeName << "." << reducedName;
						// create a new quantity description
						QuantityDescription reducedDesc = varDescs[j];
						reducedDesc.m_name = reducedName;
						// store
						refDescs[fullVariableName.str()].push_back( reducedDesc );
					}
				}
			}
			catch (IBK::Exception & ex) {
				IBK::IBK_Message(ex.what(), IBK::MSG_ERROR, FUNC_ID);
			}
		}

		// append scheduled quantities
		try  {
			std::vector<QuantityDescription> scheduleDescs;
			m_schedules->resultDescriptions(scheduleDescs);
			// add to container
			for(const QuantityDescription &refDesc : scheduleDescs) {
				std::string refTypeName = NANDRAD::KeywordList::Keyword("ModelInputReference::referenceType_t", refDesc.m_referenceType);
				std::string fullVariableName = refTypeName + "." + refDesc.m_name; // refobject type + variable name
				refDescs[fullVariableName].push_back( refDesc );
			}
		}
		catch (IBK::Exception & ex) {
			IBK::IBK_Message(ex.what(), IBK::MSG_ERROR, FUNC_ID);
		}


		// dump output reference list to file
		std::shared_ptr<std::ofstream> outputList(
			IBK::create_ofstream(m_dirs.m_varDir / "output_reference_list.txt")
		);

		(*outputList)   << std::setw(50) << std::left << "Variable name" << '\t'
						<< std::setw(20) << std::left << "Source object id(s)" << '\t'
						<< std::setw(20) << std::left << "Vector indexes/ids" << '\t'
						<< std::setw(10) << std::left << "Unit" << '\t'
						<< "Description" << '\n';

		for (const auto & varNameData : refDescs) {
			// here we store the object IDs of scalar variables
			std::set<unsigned int> objectIDs;
			// here we store the vector indexes/IDs for vector-valued variables,
			// key = objectId, value = vector-value-IDs/indexes
			std::map<unsigned int, std::vector<unsigned int> > vectorIndexes;
			for (const QuantityDescription & ref : varNameData.second) {
				// if vector-valued, write directly
				if (ref.m_indexKeyType != VectorValuedQuantityIndex::NUM_IndexKeyType) {
					vectorIndexes[ref.m_id] = ref.m_indexKeys;
				}
				else {
					// for scalar variables, only collect object IDs
					objectIDs.insert(ref.m_id);
				}
			}
			// if we have at least one object ID, write line for scalar variables
			if (!objectIDs.empty()) {
				std::string objectIDstr = IBK::join_numbers(objectIDs, ',');
				std::stringstream strm;
				strm << std::setw(50) << std::left << varNameData.first << '\t'
					 << std::setw(20) << std::left << objectIDstr << '\t'
					 << std::setw(20) << std::left << " " << '\t'
					 << std::setw(10) << varNameData.second[0].m_unit << '\t'
					 << varNameData.second[0].m_description
					 << '\n';
				IBK::IBK_Message( IBK::FormatString("%1").arg(strm.str()), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_INFO);
				(*outputList) << strm.rdbuf();
			}
			if (!vectorIndexes.empty()) {
				for (const auto & vectorVar : vectorIndexes) {
					std::string vectorIDstr = IBK::join_numbers(vectorVar.second, ',');
					std::stringstream strm;
					strm << std::setw(50) << std::left << varNameData.first << '\t'
						 << std::setw(20) << std::left << vectorVar.first << '\t'
						 << std::setw(20) << std::left << vectorIDstr << '\t'
						 << std::setw(10) << varNameData.second[0].m_unit << '\t'
						 << varNameData.second[0].m_description
						 << '\n';
					IBK::IBK_Message( IBK::FormatString("%1").arg(strm.str()), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_INFO);
					(*outputList) << strm.rdbuf();
				}
			}
		}
		outputList->flush();
		outputList->close();
	}


	// *** append variable name substitutions for zones, construction instances, ... ***

	for (const NANDRAD::Zone & zone : m_project->m_zones) {
		// skip zones without display name
		if (zone.m_displayName.empty())
			continue;
		std::string zoneObjectRef = IBK::FormatString("Zone(id=%1)").arg(zone.m_id).str();
		m_varSubstitutionMap[zoneObjectRef] = zone.m_displayName;
	}

	for (const NANDRAD::ConstructionInstance &ci: m_project->m_constructionInstances) {
		for (const NANDRAD::EmbeddedObject &eo: ci.m_embeddedObjects) {
			// skip eos without display name
			if (!eo.m_displayName.empty()) {
				std::string eoObjectRef = IBK::FormatString("EmbeddedObject(id=%1)").arg(eo.m_id).str();
				m_varSubstitutionMap[eoObjectRef] = eo.m_displayName;
			}

		}
		// skip cis without display name
		if (!ci.m_displayName.empty()) {
			std::string ciObjectRef = IBK::FormatString("ConstructionInstance(id=%1)").arg(ci.m_id).str();
			m_varSubstitutionMap[ciObjectRef] = ci.m_displayName;
		}
	}


	// *** replace any "[" and "]" in the display names to avoid problems in post proc
	for (std::map<std::string, std::string>::iterator it = m_varSubstitutionMap.begin(); it != m_varSubstitutionMap.end(); ++it ){
		it->second = IBK::replace_string(it->second, "[", "(");
		it->second = IBK::replace_string(it->second, "]", ")");
	}

	IBK::IBK_Message( IBK::FormatString("Writing Variable - Displayname Mapping Table\n"), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
	IBK_MSG_INDENT;
	IBK::Path m_mappingFilePath = (m_dirs.m_varDir / "objectref_substitutions.txt");
	m_mappingFilePath.removeRelativeParts();

	// create the mapping file
	std::unique_ptr<std::ofstream> vapMapStream( IBK::create_ofstream(m_mappingFilePath) );
	// now write a line with variable mappings for each of the variables in question
	for (const auto & m : m_varSubstitutionMap)
		*vapMapStream << m.first << '\t' << m.second << '\n';
}


void NandradModel::initSolverVariables() {
	FUNCID(NandradModel::initSolverVariables);
	IBK::IBK_Message( IBK::FormatString("Initializing Solver Variables\n"), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
	IBK_MSG_INDENT;

	// In this function the number of conserved variables is calculated (summing up states in zones and constructions)
	// and linear memory arrays for y, y0 and ydot are created.

	// Zone state variables are stored first, followed by wall (Finite-Volume) states.
	// The offsets to the start of each memory block are stored in m_zoneVariableOffset and m_wallVariableOffset.

	m_n = 0;

	if (m_nZones == 0)
		throw IBK::Exception("Currently, NANDRAD requires at last one active zone for calculation.", FUNC_ID);

	// *** count number of unknowns in zones and initialize zone offsets ***

	// all zones have the same number of unknowns
	unsigned int numVarsPerZone = m_roomStatesModelContainer.front()->nPrimaryStateResults();
	m_n = m_nZones*numVarsPerZone;

	// populate vector with offsets to zone-balance variables in global vector
	m_zoneVariableOffset.resize(m_nZones,0);
	for (unsigned int i=0; i<m_nZones; ++i)
		m_zoneVariableOffset[i] = i*numVarsPerZone;

	// *** count number of unknowns in walls and initialize wall offsets ***

	// m_n counts the number of unknowns
	m_constructionVariableOffset.resize(m_nWalls);
	for (unsigned int i=0; i<m_nWalls; ++i) {
		// store starting position inside y-vector
		m_constructionVariableOffset[i] = m_n;
		// number of unknowns/state variabes
		unsigned int nUnknowns = m_constructionStatesModelContainer[i]->nPrimaryStateResults();
		m_n += nUnknowns;
	}

	// *** count number of unknowns in thermal networks and initialize wall offsets ***

	// m_n counts the number of unknowns
	m_networkVariableOffset.resize(m_nNetworks);
	for (unsigned int i=0; i<m_nNetworks; ++i) {
		// store starting position inside y-vector
		m_networkVariableOffset[i] = m_n;
		// number of unknowns/state variabes
		unsigned int nUnknowns = m_networkStatesModelContainer[i]->nPrimaryStateResults();
		m_n += nUnknowns;
	}

	// *** set weighting factor ***

	// Problem: there are many more construction elements/states than room states. When computing
	//          WRMS-norms we sum up errors in all variables, and divide by total number (to get the mean square norm).
	//          This, however, causes errors in room zones (i.e. temperature oscillations) to pass by undetected, since
	//          their errors are just swamped by the chear mass of construction states.
	//          Hence, we artifically increase the weight of zone balances by adding a weight factor for each
	//          zone state that compensates this effect.

	/// \todo Enlarge weighting factor, since states of construction elements are in the order of 1e8
	/// (example: 293.15*2000*870 = 510081000) wheres zone states are in the order of 1e5
	/// (example: 293.15*30*1.2*1.006 = 10616.7204)
	if (m_nWalls > 0) {
		// calculate mean number of diecrtization elements for each wall
		//m_weightsFactorZones =(double) (m_n - m_nZones)/ (double) m_nWalls;
		m_weightsFactorZones = (double) (m_n - m_nZones)/ (double) m_nWalls * 100; // lets scale by 100 for now
		// ensure that we are larger then 1
		IBK_ASSERT(m_weightsFactorZones >= 1.0);
	}

	// resize storage vectors
	m_y.resize(m_n);
	m_y0.resize(m_n);
	m_ydot.resize(m_n);

	// *** Retrieve initial conditions ***
	for (unsigned int i=0; i<m_nZones; ++i) {
		m_roomStatesModelContainer[i]->yInitial(&m_y0[ m_zoneVariableOffset[i] ]);
	}

	// energy density of the constructions
	for (unsigned int i=0; i<m_nWalls; ++i) {
		m_constructionStatesModelContainer[i]->yInitial(&m_y0[0] + m_constructionVariableOffset[i]);
	}

	// enthalpies of the thermal networks
	for (unsigned int i=0; i<m_nNetworks; ++i) {
		m_networkStatesModelContainer[i]->yInitial(&m_y0[0] + m_networkVariableOffset[i]);
	}

#if 0
	// explicit models
	for (unsigned int i = 0; i<m_ODEStatesAndBalanceModelContainer.size(); ++i) {
		AbstractODEBalanceModel *balanceModel = m_ODEStatesAndBalanceModelContainer[i].second;
		std::memcpy(&y0tmp[0] + m_ODEVariableOffset[i], balanceModel->y0(),
			balanceModel->n() * sizeof(double));
	}
#endif

	// set time point to -1, which means the first call is the initialization call
	m_t = -1;

	// *** Select serial code for small problem sizes ***

	IBK::IBK_Message( IBK::FormatString("%1 unknowns\n").arg(m_n), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);

	if (m_numThreads > 1 && m_n > 1000) {
		m_useSerialCode = false;
	}
	else {
		if (m_numThreads > 1)
			IBK::IBK_Message(IBK::FormatString("Only %1 unknowns, using serial code in model evaluation!\n").arg(m_n), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
		m_useSerialCode = true;
	}
}


void NandradModel::initSolverMatrix() {
	FUNCID(NandradModel::initSolverMatrix);

	// define container for all value references with source name 'y'
	IBK::IBK_Message("Creating Jacobian matrix pattern\n", IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
	IBK::MessageIndentor indent1; (void)indent1;

	IBK::StopWatch timer;

	IBK::IBK_Message(IBK::FormatString("Composing dependency pattern\n"), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);

	try {

		// global vector of result adresses with pattern index
		std::map<const double*, unsigned int> resultValueRefs;

		// count number of unknowns: we start at position 2 * n
		unsigned int nUnknowns = (unsigned int) m_y.size() + (unsigned int) m_ydot.size();
		// count number of registered y-states
		unsigned int nYStates = 0;
		// count number of registered ydot-states
		unsigned int nYdotStates = 0;

		// the vectorial positions of the variables follow the following order
		// | R0 | R1 | ... | RnZones-1 | C0 | C1 | ... | CnConstr-1 |
		//     dot{R0} | dot{R1} | ... | dot{RnZones-1} | dot{C0} | dot{C1} | ... | dot{CnConstr-1} |
		//     .. other vars ... |

		// where R0 is either either 1 variable for thermal transport alone, or 2 for hygrothermal or ...

		// we list y-vector as results: rows and columns 0 -> nY
		// all rooms
		InputReference inRefY;
		inRefY.m_name.m_name = "y";
		for (unsigned int i = 0; i < m_roomStatesModelContainer.size(); ++i) {

			const RoomStatesModel *roomModel = m_roomStatesModelContainer[i];
			// access internal y-vector
			const double *y = roomModel->resultValueRef(inRefY);

			// Set contains all pointers to results and inputs generated by the model.
			// This is just needed to avoid adding the same value references several times
			// as different variables.
			std::set<const double*> registeredLocalValueRefs;

			// sort values into global data container
			for (unsigned int k = 0; k < roomModel->nPrimaryStateResults(); ++k) {
				resultValueRefs[y + k] = nYStates + k;
				// register in local container: key   = value ref (pointer to memory location)
				//                              value = index in global variable vector
				registeredLocalValueRefs.insert(y + k);
			}
			nYStates += roomModel->nPrimaryStateResults();

			// find all other unknowns
			std::vector< std::pair<const double *, const double *> > dependenciesIJ;
			roomModel->stateDependencies(dependenciesIJ);

			std::vector<std::pair<const double *, const double *> >::const_iterator
				dependencyIJ = dependenciesIJ.begin();
			// loop over all result quantities of the next pattern (elements aij)
			for (; dependencyIJ != dependenciesIJ.end(); ++dependencyIJ)
			{
				// retrieve row and column storage adresses
				const double * valueRef = dependencyIJ->first;

				// value ref already stored
				if (registeredLocalValueRefs.find(valueRef) !=
					registeredLocalValueRefs.end()) {
					continue;
				}

				// insert element into map with index valueRefs.size()
				registeredLocalValueRefs.insert(valueRef);
				// store inside global container
				resultValueRefs[valueRef] = nUnknowns;
				// update counter
				++nUnknowns;
			}
		}

		// ... all constructions (same procedure as above for zones)
		for (unsigned int i = 0; i < m_constructionStatesModelContainer.size(); ++i) {

			const ConstructionStatesModel *constructionModel = m_constructionStatesModelContainer[i];
			// access internal y-vector
			const double *y = constructionModel->resultValueRef(inRefY);

			std::set<const double*> registeredLocalValueRefs;
			for (unsigned int k = 0; k < constructionModel->nPrimaryStateResults(); ++k) {
				resultValueRefs[y + k] = nYStates + k;
				registeredLocalValueRefs.insert(y + k);
			}
			nYStates += constructionModel->nPrimaryStateResults();

			// find all other unknowns
			std::vector< std::pair<const double *, const double *> > dependenciesIJ;
			constructionModel->stateDependencies(dependenciesIJ);

			std::vector<std::pair<const double *, const double *> >::const_iterator
				dependencyIJ = dependenciesIJ.begin();
			// loop over all result quantities of the next pattern (elements aij)
			for (; dependencyIJ != dependenciesIJ.end(); ++dependencyIJ) {
				// retrieve row and column storage adresses
				const double * valueRef = dependencyIJ->first;

				// already stored
				if (registeredLocalValueRefs.find(valueRef) != registeredLocalValueRefs.end())
					continue;

				// insert element into map with index valueRefs.size()
				registeredLocalValueRefs.insert(valueRef);
				// store inside global container
				resultValueRefs[valueRef] = nUnknowns;
				// update counter
				++nUnknowns;
			}
		}

		// ... all networks (same procedure as above for zones)
		for (unsigned int i = 0; i < m_networkStatesModelContainer.size(); ++i) {

			const ThermalNetworkStatesModel *networkModel = m_networkStatesModelContainer[i];
			// access internal y-vector
			const double *y = networkModel->resultValueRef(inRefY);

			std::set<const double*> registeredLocalValueRefs;
			for (unsigned int k = 0; k < networkModel->nPrimaryStateResults(); ++k) {
				resultValueRefs[y + k] = nYStates + k;
				registeredLocalValueRefs.insert(y + k);
			}
			nYStates += networkModel->nPrimaryStateResults();

			// TODO Anne : register other variables (MeanTemperature etc.) into resultValueRefs
		}

		// counted y-components must equal global y-vector size
		IBK_ASSERT(nYStates == (unsigned int) m_y.size());

		// next we list ydot-vector as results:: row and column nY -> nY + nYdot - 1
		// for all rooms
		InputReference inRefYDot;
		inRefYDot.m_name.m_name = "ydot";
		for (unsigned int i = 0; i < m_roomBalanceModelContainer.size(); ++i) {

			const RoomBalanceModel *roomModel = m_roomBalanceModelContainer[i];
			// access internal y-vector
			const double *ydot = roomModel->resultValueRef(inRefYDot);

			// map storing local valueRefs according to their value pointers
			std::set<const double*> registeredLocalValueRefs;
			// sort values into glibal data container
			for (unsigned int k = 0; k < m_roomStatesModelContainer[i]->nPrimaryStateResults(); ++k) {
				resultValueRefs[ydot + k] = nYStates + nYdotStates + k;
				// register in local container
				registeredLocalValueRefs.insert(ydot + k);
			}
			nYdotStates += m_roomStatesModelContainer[i]->nPrimaryStateResults();

			// find all other unknowns
			std::vector< std::pair<const double *, const double *> > dependenciesIJ;
			// temporarilly store all elements that can be removed and that one that can be inserted
			roomModel->stateDependencies(dependenciesIJ);

			std::vector<std::pair<const double *, const double *> >::const_iterator
				dependencyIJ = dependenciesIJ.begin();
			// loop over all result quantities of the next pattern (elements aij)
			for (; dependencyIJ != dependenciesIJ.end(); ++dependencyIJ) {
				// retrieve row and column storage adresses
				const double * valueRef = dependencyIJ->first;

				// already stored
				if (registeredLocalValueRefs.find(valueRef) != registeredLocalValueRefs.end())
					continue;

				// insert element into map with index valueRefs.size()
				registeredLocalValueRefs.insert(valueRef);
				// store inside global container
				resultValueRefs[valueRef] = nUnknowns;
				// update counter
				++nUnknowns;
			}
		}

		// ... for all constructions
		for (unsigned int i = 0; i < m_constructionBalanceModelContainer.size(); ++i) {

			const ConstructionBalanceModel *constructionModel = m_constructionBalanceModelContainer[i];
			// access internal y-vector
			const double *ydot = constructionModel->resultValueRef(inRefYDot);

			// map storing local valueRefs according to their value pointers
			std::set<const double*> registeredLocalValueRefs;
			// sort values into glibal data container
			for (unsigned int k = 0; k < m_constructionStatesModelContainer[i]->nPrimaryStateResults(); ++k) {
				resultValueRefs[ydot + k] = nYStates + nYdotStates + k;
				// register in local container
				registeredLocalValueRefs.insert(ydot + k);
			}
			nYdotStates += m_constructionStatesModelContainer[i]->nPrimaryStateResults();

			// find all other unknowns
			std::vector< std::pair<const double *, const double *> > dependenciesIJ;
			// temporarilly store all elements that can be removed and that one that can be inserted
			constructionModel->stateDependencies(dependenciesIJ);

			std::vector<std::pair<const double *, const double *> >::const_iterator
				dependencyIJ = dependenciesIJ.begin();
			// loop over all result quantities of the next pattern (elements aij)
			for (; dependencyIJ != dependenciesIJ.end(); ++dependencyIJ) {
				// retrieve row and column storage adresses
				const double * valueRef = dependencyIJ->first;

				// already stored
				if (registeredLocalValueRefs.find(valueRef) != registeredLocalValueRefs.end())
					continue;

				// insert element into map with index valueRefs.size()
				registeredLocalValueRefs.insert(valueRef);
				// store inside global container
				resultValueRefs[valueRef] = nUnknowns;
				// update counter
				++nUnknowns;
			}
		}

		// ... and for all networks
		for (unsigned int i = 0; i < m_networkBalanceModelContainer.size(); ++i) {

			const ThermalNetworkBalanceModel *networkModel = m_networkBalanceModelContainer[i];
			// access internal y-vector
			const double *ydot = networkModel->resultValueRef(inRefYDot);

			// map storing local valueRefs according to their value pointers
			std::set<const double*> registeredLocalValueRefs;
			// sort values into glibal data container
			for (unsigned int k = 0; k < m_networkStatesModelContainer[i]->nPrimaryStateResults(); ++k) {
				resultValueRefs[ydot + k] = nYStates + nYdotStates + k;
				// register in local container
				registeredLocalValueRefs.insert(ydot + k);
			}
			nYdotStates += m_networkStatesModelContainer[i]->nPrimaryStateResults();

			// find all other unknowns
			std::vector< std::pair<const double *, const double *> > dependenciesIJ;
			// temporarilly store all elements that can be removed and that one that can be inserted
			networkModel->stateDependencies(dependenciesIJ);

			std::vector<std::pair<const double *, const double *> >::const_iterator
				dependencyIJ = dependenciesIJ.begin();
			// loop over all result quantities of the next pattern (elements aij)
			for (; dependencyIJ != dependenciesIJ.end(); ++dependencyIJ) {
				// retrieve row and column storage adresses
				const double * valueRef = dependencyIJ->first;

				// already stored
				if (registeredLocalValueRefs.find(valueRef) != registeredLocalValueRefs.end())
					continue;

				// insert element into map with index valueRefs.size()
				registeredLocalValueRefs.insert(valueRef);
				// store inside global container
				resultValueRefs[valueRef] = nUnknowns;
				// update counter
				++nUnknowns;
			}
		}

		// counted ydot-components must equal global ydot-vector size
		IBK_ASSERT(nYdotStates == (unsigned int)m_ydot.size());

		// loop through all models and select algebraic result quantities: row/column nY + nYdot -> nUnknowns
		for (unsigned int i = 0; i < m_stateModelContainer.size(); ++i) {

			AbstractStateDependency *stateDep = m_stateModelContainer[i];
			// skip room balance models and construction balance models
			if (dynamic_cast<RoomBalanceModel*> (stateDep) != nullptr)
				continue;
			if (dynamic_cast<ConstructionBalanceModel*> (stateDep) != nullptr)
				continue;
			if (dynamic_cast<ThermalNetworkBalanceModel*> (stateDep) != nullptr)
				continue;

			// local pattern of results and input references
			std::vector< std::pair<const double *, const double *> > dependenciesIJ;
			// temporarilly store all elements that can be removed and that one that can be inserted
			stateDep->stateDependencies(dependenciesIJ);

			// map storing local valueRefs according to their value pointers
			std::set<const double*> registeredLocalValueRefs;

			std::vector<std::pair<const double *, const double *> >::const_iterator
				dependencyIJ = dependenciesIJ.begin();
			// loop over all result quantities of the next pattern (elements aij)
			for (; dependencyIJ != dependenciesIJ.end(); ++dependencyIJ) {
				// retrieve row and column storage adresses
				const double * valueRef = dependencyIJ->first;

				// already stored
				if (registeredLocalValueRefs.find(valueRef) != registeredLocalValueRefs.end())
					continue;

				// insert element into map with index valueRefs.size()
				registeredLocalValueRefs.insert(valueRef);
				// store inside global container
				resultValueRefs[valueRef] = nUnknowns;
				// update counter
				++nUnknowns;
			}
		}

		/// create a sparse matrix pattern and a transpose pattern
		IBKMK::SparseMatrixPattern pattern(nUnknowns);
		IBKMK::SparseMatrixPattern transposePattern(nUnknowns);


		// we first add direct connections between ydots and their respective y
		// even if these should be evaluated to be 0 during Jacobi matrix calculation
		// we need those positions in the matrix pattern
		for (unsigned int i=0; i<nYStates; ++i) {
			pattern.set(i+nYStates, i);
			transposePattern.set(i, i+nYStates);
		}


		// add all dependencies

		// all room state models
		for (unsigned int i = 0; i < m_roomStatesModelContainer.size(); ++i) {

			const RoomStatesModel *roomModel = m_roomStatesModelContainer[i];
			// find all dependencies
			std::vector< std::pair<const double *, const double *> > dependenciesIJ;
			// temporarilly store all elements that can be removed and that one that can be inserted
			roomModel->stateDependencies(dependenciesIJ);

			std::vector<std::pair<const double *, const double *> >::const_iterator
				dependencyIJ = dependenciesIJ.begin();
			// loop over all result quantities of the next pattern (elements aij)
			for (; dependencyIJ != dependenciesIJ.end(); ++dependencyIJ) {
				// retrieve row and column storage adresses
				const double * resultRef = dependencyIJ->first;
				const double * inputRef = dependencyIJ->second;

				// do not allow nullptr
				IBK_ASSERT(resultRef != nullptr);
				IBK_ASSERT(inputRef != nullptr);

				// search for input value reference inside result vector
				std::map<const double*, unsigned int>::const_iterator inputRefIt =
					resultValueRefs.find(inputRef);

				// skip all references to quantiteis that are no results
				if (inputRefIt == resultValueRefs.end())
					continue;

				// search result reference
				std::map<const double*, unsigned int>::const_iterator resultRefIt =
					resultValueRefs.find(resultRef);
				// result adress must! be given
				IBK_ASSERT(resultRefIt != resultValueRefs.end());

				// get local index
				unsigned int i = resultRefIt->second;
				unsigned int j = inputRefIt->second;
				// register pattern entry
				if (!pattern.test(i, j))
					pattern.set(i, j);
				// register transpose pattern entry
				if (!transposePattern.test(j, i))
					transposePattern.set(j, i);
			}
		}

		// ... all construction states models
		for (unsigned int i = 0; i < m_constructionStatesModelContainer.size(); ++i) {

			const ConstructionStatesModel *constructionModel = m_constructionStatesModelContainer[i];
			// find all dependencies
			std::vector< std::pair<const double *, const double *> > dependenciesIJ;
			// temporarilly store all elements that can be removed and that one that can be inserted
			constructionModel->stateDependencies(dependenciesIJ);

			std::vector<std::pair<const double *, const double *> >::const_iterator
				dependencyIJ = dependenciesIJ.begin();
			// loop over all result quantities of the next pattern (elements aij)
			for (; dependencyIJ != dependenciesIJ.end(); ++dependencyIJ) {
				// retrieve row and column storage adresses
				const double * resultRef = dependencyIJ->first;
				const double * inputRef = dependencyIJ->second;

				// do not allow nullptr
				IBK_ASSERT(resultRef != nullptr);
				IBK_ASSERT(inputRef != nullptr);

				// search for input value reference inside result vector
				std::map<const double*, unsigned int>::const_iterator inputRefIt =
					resultValueRefs.find(inputRef);

				// skip all references to quantiteis that are no results
				if (inputRefIt == resultValueRefs.end())
					continue;

				// search result reference
				std::map<const double*, unsigned int>::const_iterator resultRefIt = resultValueRefs.find(resultRef);
				// result adress must! be given
				IBK_ASSERT(resultRefIt != resultValueRefs.end());

				// get local index
				unsigned int i = resultRefIt->second;
				unsigned int j = inputRefIt->second;
				// register pattern entry
				if (!pattern.test(i, j))
					pattern.set(i, j);
				// register transpose pattern entry
				if (!transposePattern.test(j, i))
					transposePattern.set(j, i);
			}
		}

		// NOTE: no need to get dependencies from ThermalNetworkStatesModel here, because all these dependencies
		//       are included in ThermalNetworkBalanceModel which is in the m_stateModelContainer.

		// ... and all remaining states model
		for (unsigned int i = 0; i < m_stateModelContainer.size(); ++i) {

			AbstractStateDependency *stateDep = m_stateModelContainer[i];

			// local pattern of results and input references
			std::vector< std::pair<const double *, const double *> > dependenciesIJ;
			// temporarilly store all elements that can be removed and that one that can be inserted
			stateDep->stateDependencies(dependenciesIJ);

			std::vector<std::pair<const double *, const double *> >::const_iterator
				dependencyIJ = dependenciesIJ.begin();
			// loop over all result quantities of the next pattern (elements aij)
			for (; dependencyIJ != dependenciesIJ.end(); ++dependencyIJ) {
				// retrieve row and column storage adresses
				const double * resultRef = dependencyIJ->first;
				const double * inputRef = dependencyIJ->second;
				// do not allow nullptr
				IBK_ASSERT(resultRef != nullptr);
				IBK_ASSERT(inputRef != nullptr);

				// search for input value reference inside result vector
				std::map<const double*, unsigned int>::const_iterator inputRefIt =
					resultValueRefs.find(inputRef);

				// skip all references to quantiteis that are no results
				if (inputRefIt == resultValueRefs.end())
					continue;

				// search result reference
				std::map<const double*, unsigned int>::const_iterator resultRefIt =
					resultValueRefs.find(resultRef);
				// result adress must! be given
				IBK_ASSERT(resultRefIt != resultValueRefs.end());

				// get local index
				unsigned int i = resultRefIt->second;
				unsigned int j = inputRefIt->second;
				// register pattern entry
				if (!pattern.test(i, j))
					pattern.set(i, j);
				// register transpose pattern entry
				if (!transposePattern.test(j, i))
					transposePattern.set(j, i);
			}
		}

		// calculate transitive closure over all algebraic dependencies (block nY + nYdot -> nUnknowns)
		// this will add entries for ydot-y dependencies
		IBKMK::SparseMatrixPattern::calculateTransitiveClosure(pattern, transposePattern,
			nUnknowns, nYStates + nYdotStates, nUnknowns);

		// clear ia and ja
		if (!m_ia.empty())
			m_ia.clear();
		if (!m_ja.empty())
			m_ja.clear();

		// calculate CSR pattern: we only consider ydot-> y block (row nY -> nY + nYdot - 1, column 0 -> nY)
		for (unsigned int i = nYStates; i < nYStates + nYdotStates; ++i) {
			// filter all value references refering to ydot
			m_ia.push_back((unsigned int)m_ja.size());
			// retreive all indices
			std::vector<unsigned int> columns;
			pattern.indexesPerRow(i, columns);
			// fill all column entries into inderx vectoe
			for (unsigned int jIdx = 0; jIdx < columns.size(); ++jIdx) {
				// ignore columns that do not assign a y-component (columnns are sorted)
				unsigned int j = columns[jIdx];
				if (j >= nYStates)
					break;
				// find index inside result index vector
				m_ja.push_back(j);
			}
		}
		// set last element
		m_ia.push_back((unsigned int)m_ja.size());

		// clear iaT and jaT
		if (!m_iaT.empty())
			m_iaT.clear();
		if (!m_jaT.empty())
			m_jaT.clear();
		// generate transpose index
		IBKMK::SparseMatrixCSR::generateTransposedIndex(m_ia, m_ja, m_iaT, m_jaT);
	}
	catch (IBK::Exception &ex) {
		throw IBK::Exception(ex, "Error initializing global solver matrix!", FUNC_ID);
	}
}


void NandradModel::initStatistics(SOLFRA::ModelInterface * modelInterface, bool restart) {
	if (restart) {
		// m_secondsInLastRun is set in setRestart()
		// m_simTimeAtStart is set in setRestart()

		// re-open progressLog file for writing
		m_progressLog = IBK::create_ofstream(m_dirs.m_logDir / "progress.tsv", std::ios_base::app);
	}
	else {
		m_elapsedSecondsAtStart = 0;
		m_elapsedSimTimeAtStart = t0();
		// open progressLog file for writing and write header
		m_progressLog = IBK::create_ofstream(m_dirs.m_logDir / "progress.tsv");
	}

	// setup feedback object, this also starts the stopwatch
	m_feedback.setup(m_progressLog, t0(), tEnd(), m_projectFilePath.str(), m_elapsedSecondsAtStart, m_elapsedSimTimeAtStart, modelInterface);
}


void NandradModel::registerStateDependendModel(AbstractStateDependency *stateModel) {

	unsigned int priority = (unsigned int)stateModel->priorityOfModelEvaluation();
	if (priority == (unsigned int)-1) {
		// set into state dependent stack (in order to enforace a correct update)
		m_unorderedStateDependencies.push_back(stateModel);
	}
	else if (priority < AbstractStateDependency::priorityOffsetTail) {
		// enlarge vector if necessary
		if (m_orderedStateDependentSubModelsHead.size() < priority + 1)
			m_orderedStateDependentSubModelsHead.resize(priority + 1);
		// sort model into vector
		m_orderedStateDependentSubModelsHead[priority].push_back(stateModel);
		// register model as state dependend
		m_stateModelContainer.push_back(stateModel);
	}
	else /*(priority >= AbstractStateDependency::priorityOffsetTail)*/ {
		// enlarge vector if necessary
		unsigned int pos = priority - AbstractStateDependency::priorityOffsetTail;
		if (m_orderedStateDependentSubModelsTail.size() < pos + 1)
			m_orderedStateDependentSubModelsTail.resize(pos + 1);
		// sort model into vector
		m_orderedStateDependentSubModelsTail[pos].push_back(stateModel);
		// register model as state dependend
		m_stateModelContainer.push_back(stateModel);
	}
}


int NandradModel::updateTimeDependentModels() {
	// *** update time in all directly time dependend models ***

	int calculationResultFlag = 0;
	for (std::vector<AbstractTimeDependency*>::iterator it1 = m_timeModelContainer.begin();
		it1 != m_timeModelContainer.end(); ++it1)
	{
#ifdef IBK_STATISTICS
		// set time for all objects independently
		SUNDIALS_TIMED_FUNCTION(NANDRAD_TIMER_SETTIME,
			calculationResultFlag |= (*it1)->setTime(m_t)
		);
		++m_nSetTimeCalls;
#else
		calculationResultFlag |= (*it1)->setTime(m_t);
#endif
	}
	if (calculationResultFlag != 0) {
		if (calculationResultFlag & 2)
			return 2;
		else
			return 1;
	}

	// mark solution as updated
	m_tChanged = false;
	// signal success
	return 0;
}


int NandradModel::updateStateDependentModels() {

	// *** update global states (head models) ***

	// Only transfers room energy - cheap functions, does not need to be parallelized
	for (unsigned int i = 0; i < m_roomStatesModelContainer.size(); ++i) {
		m_roomStatesModelContainer[i]->update(&m_y[0] + m_zoneVariableOffset[i]);
	}

	// update states in all construction solver models
	// Note: since setY() is already a very very fast function, this parallel loop is not
	//       making much difference in the overall simulation performance of larger models.
#ifdef _OPENMP
	if (!m_useSerialCode) {
#pragma omp parallel for
		for (int i = 0; i < (int)m_constructionStatesModelContainer.size(); ++i) {
			m_constructionStatesModelContainer[i]->update(&m_y[0] + m_constructionVariableOffset[i]);
		}
	}
#endif // _OPENMP
	if (m_useSerialCode) {
		for (unsigned int i = 0; i < m_constructionStatesModelContainer.size(); ++i) {
			m_constructionStatesModelContainer[i]->update(&m_y[0] + m_constructionVariableOffset[i]);
		}
	}

	// update network model container
	for (unsigned int i = 0; i < m_networkStatesModelContainer.size(); ++i) {
		m_networkStatesModelContainer[i]->update(&m_y[0] + m_networkVariableOffset[i]);
	}


	// evaluate ordered graph
	int calculationResultFlag = 0;


	// to keep original serial code performance, distinguish between serial and OpenMP code
#ifdef _OPENMP
	if (!m_useSerialCode) {

		// create storage vector for thread-specific return codes
		std::vector<int> calculationResultFlags(m_numThreads, 0);

		// process all parallel object groups
		for (unsigned int k = 0; k < m_orderedStateDependentSubModels.size(); ++k) {
			ParallelStateObjects &parallelObjects = m_orderedStateDependentSubModels[k];

	#pragma omp parallel
			{
				// store target location for thread-specific error codes
				int & calculationResultFlag = calculationResultFlags[omp_get_thread_num()];
	#pragma omp for
				for (int i = 0; i < (int)parallelObjects.size(); ++i) {
					calculationResultFlag |= parallelObjects[i]->update();
				} // end for

			} // end parallel region

		} // end m_orderedStateDependentSubModels loop

		// in multi-threaded code, check all return codes
		for (int i=0; i<m_numThreads; ++i) {
			if (calculationResultFlags[i] != 0) {
	//			IBK::IBK_Message(IBK::FormatString("Error code %1 in thread #%2").arg(calculationResultFlags[i]).arg(i), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
				if (calculationResultFlags[i] & 2)
					return 2;
				else
					return 1;
			}
		}
	} // use serial code

#endif // _OPENMP

	if (m_useSerialCode) {
		for (unsigned int k = 0; k < m_orderedStateDependentSubModels.size(); ++k) {
			ParallelStateObjects &parallelObjects = m_orderedStateDependentSubModels[k];

			// now begin parallel section
			for (unsigned int i = 0; i < parallelObjects.size(); ++i) {

#ifdef IBK_STATISTICS
				if (parallelObjects[i]->m_modelTypeId & DefaultStateDependency::ODE) {
					SUNDIALS_TIMED_FUNCTION(NANDRAD_TIMER_UPDATE_ODEMODELS,
						calculationResultFlag |= parallelObjects[i]->update()
					);
					++m_nODEModelsUpdate;
				}
				else if (parallelObjects[i]->m_modelTypeId & DefaultStateDependency::CyclicGroup) {
					SUNDIALS_TIMED_FUNCTION(NANDRAD_TIMER_UPDATE_MODELGROUPS,
						calculationResultFlag |= parallelObjects[i]->update()
					);
					++m_nModelGroupsUpdate;
				}
				else {
					SUNDIALS_TIMED_FUNCTION(NANDRAD_TIMER_UPDATE_MODELS,
						calculationResultFlag |= parallelObjects[i]->update()
					);
					++m_nModelsUpdate;
				}
#else
				calculationResultFlag |= parallelObjects[i]->update();
#endif
				if (calculationResultFlag != 0)
					return calculationResultFlag;
			}
		}
	} // useSerialCode




	// *** update ydot-values and store in (locally numbered) m_ydot vector***

	// update states in all room state models
	for (unsigned int i=0; i<m_roomBalanceModelContainer.size(); ++i) {
#ifdef IBK_STATISTICS
		SUNDIALS_TIMED_FUNCTION(NANDRAD_TIMER_YDOT,
			calculationResultFlag |= m_roomBalanceModelContainer[i]->update();
			calculationResultFlag |= m_roomBalanceModelContainer[i]->ydot(&m_ydot[0] + m_zoneVariableOffset[i]);
		);
#else
		calculationResultFlag |= m_roomBalanceModelContainer[i]->ydot(&m_ydot[0] + m_zoneVariableOffset[i]);
#endif
	}
	if (calculationResultFlag != 0) {
		if (calculationResultFlag & 2)
			return 2;
		else
			return 1;
	}

	// update states in all construction solver models
	for (unsigned int i=0; i<m_constructionBalanceModelContainer.size(); ++i) {
#ifdef IBK_STATISTICS
		SUNDIALS_TIMED_FUNCTION(NANDRAD_TIMER_YDOT,
			calculationResultFlag |= m_constructionBalanceModelContainer[i]->ydot(&m_ydot[0] + m_constructionVariableOffset[i]);
		);
		++m_nYdotCalls;
#else
		calculationResultFlag |= m_constructionBalanceModelContainer[i]->ydot(&m_ydot[0] + m_constructionVariableOffset[i]);
#endif
	}
	if (calculationResultFlag != 0) {
		if (calculationResultFlag & 2)
			return 2;
		else
			return 1;
	}

	// update states in all thermal network models
	for (unsigned int i=0; i<m_networkBalanceModelContainer.size(); ++i) {
#ifdef IBK_STATISTICS
		SUNDIALS_TIMED_FUNCTION(NANDRAD_TIMER_YDOT,
			calculationResultFlag |= m_networkBalanceModelContainer[i]->ydot(&m_ydot[0] + m_networkVariableOffset[i]);
		);
		++m_nYdotCalls;
#else
		calculationResultFlag |= m_networkBalanceModelContainer[i]->ydot(&m_ydot[0] + m_networkVariableOffset[i]);
#endif
	}
	if (calculationResultFlag != 0) {
		if (calculationResultFlag & 2)
			return 2;
		else
			return 1;
	}

	// mark solution as updated
	m_yChanged = false;

	// signal success
	return 0;
}


} // namespace NANDRAD_MODEL


