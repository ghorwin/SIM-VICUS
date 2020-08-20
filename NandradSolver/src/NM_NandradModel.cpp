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

#include "NM_NandradModel.h"

#if defined(_OPENMP)
#include <omp.h>
#endif // _OPENMP

#include <memory>

#include <IBK_Version.h>
#include <IBK_messages.h>
#include <IBK_FormatString.h>

#include <CCM_Constants.h>

#include <NANDRAD_ArgsParser.h>
#include <NANDRAD_Constants.h>
#include <NANDRAD_Project.h>
#include <NANDRAD_KeywordList.h>

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
#include "NM_OutputFile.h"
#include "NM_StateModelGroup.h"
#include "NM_ValueReference.h"
#include "NM_FMIInputOutput.h"
#include "NM_OutputHandler.h"
#include "NM_ConstructionStatesModel.h"
#include "NM_ConstructionBalanceModel.h"

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
	delete m_fmiInputOutput;
	delete m_outputHandler;
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
	// *** Initialize Global Parameters ***
	initGlobals();
	// *** Initialize RoomBalanceModels and ConstantZoneModels ***
	initZones();
	// *** Initialize EnergyPerformanceIndicatorModels ***
//	initZoneLists();
	// *** Initialize Wall and Construction BC Modules ***
	initWallsAndInterfaces();
	// *** Initialize Window Models ***
//	initEmbeddedObjects();
	// *** Initialize ModelGroups ***
//	initModelGroups();
	// *** Initialize all internal fmus ***
//	initFMUComponents();
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
	return m_project->m_solverParameter.m_para[NANDRAD::SolverParameter::SP_INITIAL_DT].value;
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
	// update state of model to output time and variables
	// finally write the output
	setTime(t_out);
	setY(y_out);
	ydot(nullptr);

	// move (relative) simulation time to absolute time (offset to midnight, January 1st of the start year)
	double t_secondsOfYear = t_out + m_project->m_simulationParameter.m_interval.m_para[NANDRAD::Interval::IP_START].value;

	m_outputHandler->writeOutputs(t_out, t_secondsOfYear);

	// write feedback to user
	IBK_ASSERT(m_t == t_out);
	m_feedback.writeFeedback(t_out, false);
}


void NandradModel::writeFinalOutputs() {
	SUNDIALS_TIMED_FUNCTION( SUNDIALS_TIMER_WRITE_OUTPUTS,
		m_outputHandler->flushCache();
	);
}


std::string NandradModel::simTime2DateTimeString(double t) const {
	// add start time offset to t and then call parent function
	int startYear = m_project->m_simulationParameter.m_intpara[NANDRAD::SimulationParameter::SIP_YEAR].value;
	t += m_project->m_simulationParameter.m_interval.m_para[NANDRAD::Interval::IP_START].value;
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

#if 0
	// update states in all exoplicit ODE models
	for (unsigned int i = 0; i<m_ODEStatesAndBalanceModelContainer.size(); ++i) {
		AbstractODEBalanceModel *balanceModel = m_ODEStatesAndBalanceModelContainer[i].second;
		balanceModel->stepCompleted(t, y + m_ODEVariableOffset[i]);
	}
	// update states in all staedy state solver models
	for (unsigned int i = 0; i< m_steadyStateModelContainer.size(); ++i) {
		SteadyStateSolver *steadyStateModel = m_steadyStateModelContainer[i];
		steadyStateModel->stepCompleted(t, nullptr);
	}
#endif
}



SOLFRA::LESInterface * NandradModel::lesInterface() {
	const char * const FUNC_ID = "[NandradModel::lesInterface]";

	if (m_lesSolver != nullptr)
		return m_lesSolver;
	IBK_ASSERT(m_preconditioner == nullptr);
	IBK_ASSERT(m_jacobian == nullptr);

	if (m_project->m_solverParameter.m_integrator == NANDRAD::SolverParameter::I_EXPLICIT_EULER) {
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
		case NANDRAD::SolverParameter::LES_DENSE : {
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
		case NANDRAD::SolverParameter::LES_BICGSTAB : {
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
			if (!m_project->m_solverParameter.m_para[NANDRAD::SolverParameter::SP_PRE_ILUWIDTH].name.empty()/*
				&& m_project->m_solverParameter.m_para[NANDRAD::SolverParameter::SP_PRE_ILUWIDTH].value > 0*/) {
				unsigned int fillIn = (unsigned int)m_project->m_solverParameter.m_para[NANDRAD::SolverParameter::SP_PRE_ILUWIDTH].value;

				m_preconditioner = new SOLFRA::PrecondILUT(SOLFRA::PrecondInterface::Right, fillIn);
				precondName = IBK::FormatString("ILUT preconditioner").str();
			}
			else {
				m_preconditioner = new SOLFRA::PrecondILU(SOLFRA::PrecondInterface::Right);
				precondName = IBK::FormatString("ILU preconditioner").str();

			}
		} break;

		// no preconditioner
		case NANDRAD::SolverParameter::NUM_PRE :
		default : ;
	}

	SOLFRA::LESInterfaceIterative * lesIter = dynamic_cast<SOLFRA::LESInterfaceIterative *>(m_lesSolver);
	IBK_ASSERT(lesIter != nullptr);

	// set iterative LES solver options
	lesIter->m_maxKrylovDim = (unsigned int)m_project->m_solverParameter.m_para[NANDRAD::SolverParameter::SP_MAX_KRYLOV_DIM].value;
	lesIter->m_linIterConvCoeff = m_project->m_solverParameter.m_para[NANDRAD::SolverParameter::SP_ITERATIVESOLVERCONVCOEFF].value;

	IBK_Message(IBK::FormatString("%1 selected, MaxKrylovDim = %2\n")
		.arg(precondName).arg(lesIter->m_maxKrylovDim),  IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);

	return m_lesSolver;
}


SOLFRA::JacobianInterface *  NandradModel::jacobianInterface() {
	return m_jacobian;
}


SOLFRA::IntegratorInterface * NandradModel::integratorInterface() {

	const char * const FUNC_ID = "[NandradModel::integratorInterface]";
	if (m_integrator != nullptr)
		return m_integrator;

	IBK::IBK_Message("\n", IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
	IBK::IBK_Message("Creating Integrator\n", IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
	IBK_MSG_INDENT;

	if (m_project->m_solverParameter.m_integrator == NANDRAD::SolverParameter::I_EXPLICIT_EULER) {
		IBK::IBK_Message("Using Explict Euler integrator.\n", IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
		SOLFRA::IntegratorExplicitEuler * integrator = new SOLFRA::IntegratorExplicitEuler();
		integrator->m_dt = dt0();
		m_integrator = integrator;
	}
	else if (m_project->m_solverParameter.m_integrator == NANDRAD::SolverParameter::I_IMPLICIT_EULER) {
		IBK::IBK_Message("Using Implicit Euler integrator.\n", IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
		SOLFRA::IntegratorImplicitEuler * integrator = new SOLFRA::IntegratorImplicitEuler();
		// set parameters given by Solverparameter section
		integrator->m_absTol = m_project->m_solverParameter.m_para[NANDRAD::SolverParameter::SP_ABSTOL].value;
		integrator->m_relTol = m_project->m_solverParameter.m_para[NANDRAD::SolverParameter::SP_RELTOL].value;
		integrator->m_nonLinConvCoeff = m_project->m_solverParameter.m_para[NANDRAD::SolverParameter::SP_NONLINSOLVERCONVCOEFF].value;
		integrator->m_maximumNonlinearIterations = (unsigned int) m_project->m_solverParameter.m_para[NANDRAD::SolverParameter::SP_MAX_NONLIN_ITER].value;
		/// \todo Specify ImplicitEuler parameters

		m_integrator = integrator;
	}
	else if (m_project->m_solverParameter.m_integrator == NANDRAD::SolverParameter::I_CVODE ||
		m_project->m_solverParameter.m_integrator == NANDRAD::SolverParameter::NUM_I)
	{
		IBK::IBK_Message("Using CVODE integrator.\n", IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
		SOLFRA::IntegratorSundialsCVODE * integrator = new SOLFRA::IntegratorSundialsCVODE();
		// set parameters given by Solverparameter section
		integrator->m_absTol = m_project->m_solverParameter.m_para[NANDRAD::SolverParameter::SP_ABSTOL].value;
		integrator->m_relTol = m_project->m_solverParameter.m_para[NANDRAD::SolverParameter::SP_RELTOL].value;
		integrator->m_dtMax = m_project->m_solverParameter.m_para[NANDRAD::SolverParameter::SP_MAX_DT].value;
		integrator->m_dtMin = m_project->m_solverParameter.m_para[NANDRAD::SolverParameter::SP_MIN_DT].value;
		integrator->m_maxOrder = (unsigned int)m_project->m_solverParameter.m_para[NANDRAD::SolverParameter::SP_MAX_ORDER].value;
		integrator->m_maxSteps = 100000000; // extremely large value
		integrator->m_nonLinConvCoeff = m_project->m_solverParameter.m_para[NANDRAD::SolverParameter::SP_NONLINSOLVERCONVCOEFF].value;
		integrator->m_maxNonLinIters = (unsigned int) m_project->m_solverParameter.m_para[NANDRAD::SolverParameter::SP_MAX_NONLIN_ITER].value;

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
	// wrong definition
	IBK_ASSERT(integrator != nullptr);
	// start with the classic definition
	const double absTol = integrator->m_absTol;
	const double relTol = integrator->m_relTol;
	// fill error weights with classical definition
	for(unsigned int i = 0; i < m_n; ++i) {
		weights[i] = 1.0/(relTol * std::fabs(y[i]) + absTol);
	}

#if 0
	// modify weighting factor for all zones
	for (unsigned int i=0; i<m_nZones; ++i) {
		// currently each zone has exactly one state variable
		unsigned int idx = m_zoneVariableOffset[i];
		// weight single zone balance with mean number of wall discrtization elements
		weights[idx] *= m_weightsFactorZones;
	}

	unsigned int k = 0;
	// set integral value references fo all outputs
	for (unsigned int i = 0; i<m_outputFiles.size(); ++i) {

		if (m_outputFiles[i]->timeType() == NANDRAD::OutputDefinition::OTT_NONE)
			continue;
		// modify weighting factor for all outputs
		unsigned int idx = m_outputVariableOffset[k];

		for (unsigned int n = 0; n < m_outputFiles[i]->m_nOutputValuesForOneTimeStep; ++n) {
			// weight single zone balance with mean number of wall discrtization elements
			weights[idx + n] *= m_weightsFactorOutputs;
		}
		++k;
	}
#endif
	return SOLFRA::ModelInterface::CalculationSuccess;
}


bool NandradModel::hasErrorWeightsFunction() {
	return true;
}


std::size_t NandradModel::serializationSize() const {
	// nothing to serialize
	size_t s = 0;
	return s;
}


void NandradModel::serialize(void* & /*dataPtr*/) const {
	// nothing to serialize
}


void NandradModel::deserialize(void* & /*dataPtr*/) {
	// nothing to serialize
}


void NandradModel::writeMetrics(double simtime, std::ostream * metricsFile) {
	(void)simtime;
	(void)metricsFile;

#ifdef IBK_STATISTICS
	const char * const FUNC_ID = "[NandradModel::writeMetrics]";
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
	double tWithStartOffset = t + m_project->m_simulationParameter.m_interval.m_para[NANDRAD::Interval::IP_START].value;

	for (std::vector<NANDRAD::OutputGrid>::const_iterator it = m_project->m_outputs.m_grids.begin();
		 it != m_project->m_outputs.m_grids.end(); ++it)
	{
		tOutNext = std::min(tOutNext, it->computeNextOutputTime(tWithStartOffset));
	}

	// convert tOutNext back to simulation time by subtracting offset
	return tOutNext - m_project->m_simulationParameter.m_interval.m_para[NANDRAD::Interval::IP_START].value;
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
			solverParameter.m_integrator = NANDRAD::SolverParameter::I_EXPLICIT_EULER;
		else if (IBK::toupper_string(solverString) == "IMPLICITEULER") // ImplicitEuler
			solverParameter.m_integrator = NANDRAD::SolverParameter::I_IMPLICIT_EULER;
		else {
			throw IBK::Exception( IBK::FormatString("Unknown/unsupported integrator '%1'.").arg(solverString), FUNC_ID);
		}
	}

	// --les-solver
	if (!args.m_lesSolverName.empty()) {
		// check if this is a valid/known solver
		if (IBK::toupper_string(args.m_lesSolverName) == "DENSE") {
			solverParameter.m_lesSolver = NANDRAD::SolverParameter::LES_DENSE;
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
				solverParameter.m_para[NANDRAD::SolverParameter::SP_MAX_KRYLOV_DIM].value = args.m_lesSolverOption;
		}
		else if (IBK::toupper_string(args.m_lesSolverName) == "BICGSTAB") {
			solverParameter.m_lesSolver = NANDRAD::SolverParameter::LES_BICGSTAB;
			if (args.m_lesSolverOption != (unsigned int)-1)
				solverParameter.m_para[NANDRAD::SolverParameter::SP_MAX_KRYLOV_DIM].value = args.m_lesSolverOption;
		}
		else {
			throw IBK::Exception( IBK::FormatString("Unknown/unsupported LES-solver '%1'.").arg(args.m_lesSolverName), FUNC_ID);
		}
	}

	// --precond
	if (!args.m_preconditionerName.empty()) {
		// must have an iterative solver for preconditioner use
		if (solverParameter.m_lesSolver != NANDRAD::SolverParameter::LES_GMRES &&
			solverParameter.m_lesSolver != NANDRAD::SolverParameter::LES_BICGSTAB)
		{
			throw IBK::Exception( IBK::FormatString("Cannot use --precond option with direct LES-solver."), FUNC_ID);
		}

		// check if this is a valid/known preconditioner
		if (IBK::toupper_string(args.m_preconditionerName) == "BAND") {
			solverParameter.m_preconditioner = NANDRAD::SolverParameter::PRE_BAND;
			if (args.m_preconditionerOption != (unsigned int)-1)
				// also store bandwidth
				solverParameter.m_para[NANDRAD::SolverParameter::SP_PRE_BANDWIDTH].set(
					NANDRAD::KeywordList::Keyword("SolverParameter::para_t", NANDRAD::SolverParameter::SP_PRE_BANDWIDTH),
					args.m_preconditionerOption, IBK::Unit("---"));
		}
		else if (IBK::toupper_string(args.m_preconditionerName) == "ILU") {
			solverParameter.m_preconditioner = NANDRAD::SolverParameter::PRE_ILU;
			if (args.m_preconditionerOption != (unsigned int)-1)
				// also store bandwidth
				solverParameter.m_para[NANDRAD::SolverParameter::SP_PRE_ILUWIDTH].set(
					NANDRAD::KeywordList::Keyword("SolverParameter::para_t", NANDRAD::SolverParameter::SP_PRE_ILUWIDTH),
					args.m_preconditionerOption, IBK::Unit("---"));
		}
		else {
			throw IBK::Exception( IBK::FormatString("Unknown/unsupported preconditioner '%1'.").arg(args.m_preconditionerName), FUNC_ID);
		}
	}


	// *** mandatory arguments (defaults are always specified) ***

	const IBK::Parameter &absTol = solverParameter.m_para[NANDRAD::SolverParameter::SP_ABSTOL];
	if (absTol.value <= 0.0)
		throw IBK::Exception(IBK::FormatString("Error initializing wall solver: "
			"SolverParameter 'AbsTol' is smaller than/ equal to zero."), FUNC_ID);

	const IBK::Parameter &relTol = solverParameter.m_para[NANDRAD::SolverParameter::SP_RELTOL];
	if (relTol.value <= 0.0)
		throw IBK::Exception(IBK::FormatString("Error initializing wall solver: "
			"SolverParameter 'RelTol' is smaller than/ equal to zero."), FUNC_ID);

	const IBK::Parameter & minDx = solverParameter.m_para[NANDRAD::SolverParameter::SP_DISCRETIZATION_MIN_DX];
	if (minDx.value <= 1e-10)
		throw IBK::Exception("Invalid parameter for DiscMinDx in SolverParameter settings, should be > 1e-10 m.", FUNC_ID);

	const IBK::Parameter & density = solverParameter.m_para[NANDRAD::SolverParameter::SP_DISCRETIZATION_STRECH_FACTOR];
	if (density.value < 1 && density.value != 0.0)
		throw IBK::Exception("Invalid parameter for DiscDetailLevel in SolverParameter settings (grid density must be == 0 or >= 1).", FUNC_ID);

	const IBK::Parameter & krylovDim = solverParameter.m_para[NANDRAD::SolverParameter::SP_MAX_KRYLOV_DIM];
	if ((int)krylovDim.value <= 0)
		throw IBK::Exception("Invalid parameter for MaxKrylovDim in SolverParameter settings.", FUNC_ID);

	const IBK::Parameter &nonlinConvCoeff = solverParameter.m_para[NANDRAD::SolverParameter::SP_NONLINSOLVERCONVCOEFF];
	// check validity
	if (nonlinConvCoeff.value <= 0.0)
		throw IBK::Exception("Invalid parameter for NonlinSolverConvCoeff in SolverParameter settings.", FUNC_ID);

	const IBK::Parameter &iterativeConvCoeff = solverParameter.m_para[NANDRAD::SolverParameter::SP_ITERATIVESOLVERCONVCOEFF];
	// check validity
	if (iterativeConvCoeff.value <= 0.0)
		throw IBK::Exception(IBK::FormatString("Invalid parameter for IterativeSolverConvCoeff in SolverParameter settings."), FUNC_ID);

	// *** optional arguments (without guaranteed default value) ***

	const IBK::Parameter & initialDt = solverParameter.m_para[NANDRAD::SolverParameter::SP_INITIAL_DT];
	if (initialDt.value <= 0)
		throw IBK::Exception("Invalid parameter for InitialTimeStep in SolverParameter settings.", FUNC_ID);

	// *** Define standard behavior if definitions are missing ***

	if (m_project->m_solverParameter.m_integrator != NANDRAD::SolverParameter::I_EXPLICIT_EULER) {
		// if no LES solver has been specified, default to GMRES with ILU preconditioner
		if (m_project->m_solverParameter.m_lesSolver == NANDRAD::SolverParameter::NUM_LES) {
			m_project->m_solverParameter.m_lesSolver = NANDRAD::SolverParameter::LES_KLU;
//			m_project->m_solverParameter.m_preconditioner = NANDRAD::SolverParameter::PRE_ILU;
			IBK::IBK_Message("Auto-selecting GMRES with ILU preconditioner.\n", IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
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

	IBK::IBK_Message( IBK::FormatString("Initializing Simulation Parameter\n"), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_INFO);

	const NANDRAD::SimulationParameter &simPara = m_project->m_simulationParameter;

	/// \todo check validity of the parameter data, since data may be overrided in project file

	// Set simulation interval

	// Simulation time always starts with zero (=solver time).
	// All models dealing with absolute time reference (climate data and schedules) will
	// shift the simulation time by the start time/year offset.
	m_t0  = 0;

	// We always have end time and start time given, as part of the simulation defaults.
	// Project file may have different values, but by reading project file, settings cannot be removed.

	m_tEnd = simPara.m_interval.m_para[NANDRAD::Interval::IP_END].value -
		simPara.m_interval.m_para[NANDRAD::Interval::IP_START].value;

	int startYear = 2001;
	if (!simPara.m_intpara[NANDRAD::SimulationParameter::SIP_YEAR].name.empty())
		startYear = simPara.m_intpara[NANDRAD::SimulationParameter::SIP_YEAR].value;

	if (m_tEnd <= 0)
		throw IBK::Exception(IBK::FormatString("End time point %1 preceedes start time point %2 (must be later than start time!)")
							 .arg(IBK::Time(startYear, simPara.m_interval.m_para[NANDRAD::Interval::IP_END].value).toDateTimeFormat())
							 .arg(IBK::Time(startYear, simPara.m_interval.m_para[NANDRAD::Interval::IP_START].value).toDateTimeFormat()), FUNC_ID);

	// check simulation parameters for meaningful values, since they are user-defined and can be "unsuitable"

	if (simPara.m_para[NANDRAD::SimulationParameter::SP_INITIAL_TEMPERATURE].value < 123.15)
		throw IBK::Exception(IBK::FormatString("Invalid initial temperature %1 in SimulationParameters.")
							 .arg(simPara.m_para[NANDRAD::SimulationParameter::SP_INITIAL_TEMPERATURE].get_value("C")), FUNC_ID);

	/// \todo other checks
	///
	/// \todo check relative humidity when moisture balance is enabled
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
	/// \todo check here, if we are inside an FMU and only initialize FMI interface model when needed

	FUNCID(NandradModel::initFMI);
	IBK::IBK_Message(IBK::FormatString("Initializing FMI interface\n"), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
	IBK_MSG_INDENT;


	try {
		m_fmiInputOutput = new FMIInputOutput;
		// insert into model container
		m_modelContainer.push_back(m_fmiInputOutput);	// now owns the model and handles memory cleanup
		// insert into time model container
		m_timeModelContainer.push_back(m_fmiInputOutput);

		// init FMI import/export model - this should only be done, if we are actually inside an FMU
		m_fmiInputOutput->setup(*m_project);
	}
	catch (IBK::Exception & ex) {
		throw IBK::Exception(ex, IBK::FormatString("Error initializing FMI interface model."), FUNC_ID);
	}
}


void NandradModel::initGlobals() {

}


void NandradModel::initZones() {
	FUNCID(NandradModel::initZones);
	IBK::IBK_Message( IBK::FormatString("Initializing Zones\n"), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_INFO);
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
		switch (zone.m_type) {
			case NANDRAD::Zone::ZT_ACTIVE : {
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

				// remember current zone
				activeZones.push_back(&zone);
			} break;


			// initialise a constant zone model
			case NANDRAD::Zone::ZT_CONSTANT :
			{
#if 0
				// create implicit room state model
				ConstantZoneModel * constantZoneModel = NULL;

				// coupled heat anmd moisture balance
				if (m_project->m_simulationParameter.m_flags[NANDRAD::SimulationParameter::SF_ENABLE_MOISTURE_BALANCE].isEnabled()) {
					constantZoneModel = new ConstantZoneMoistureModel(zone.m_id, zone.m_displayName);
				}
				// isolated energy balance
				else{
					constantZoneModel  = new ConstantZoneModel(zone.m_id, zone.m_displayName);
				}

				// initialize room state model
				try {
					constantZoneModel->setup(zone);
				}
				catch (IBK::Exception & ex) {
					throw IBK::Exception(ex, IBK::FormatString("Error in setup for #%1 for zone with id #%2")
													.arg(constantZoneModel->ModelIDName())
													.arg(zone.m_id), FUNC_ID);
				}

				// always put the model first into our central model storage
				m_modelContainer.push_back(constantZoneModel); // this container now owns the model

				// sort into the state model container
				registerStateDependendModel(constantZoneModel);
#endif
			} break;


			case NANDRAD::Zone::ZT_GROUND:
			{
#if 0
				// create implicit room state model
				GroundZoneModel * groundZoneModel = NULL;

				// coupled heat anmd moisture balance
				if (m_project->m_simulationParameter.m_flags[NANDRAD::SimulationParameter::SF_ENABLE_MOISTURE_BALANCE].isEnabled()) {
					groundZoneModel = new GroundZoneMoistureModel(zone.m_id, zone.m_displayName);
				}
				// isolated energy balance
				else {
					groundZoneModel = new GroundZoneModel(zone.m_id, zone.m_displayName);
				}

				// initialize room state model
				try {
					groundZoneModel->setup(zone, m_project->m_simulationParameter, m_project->m_placeholders);
				}
				catch (IBK::Exception & ex) {
					throw IBK::Exception(ex, IBK::FormatString("Error in setup for GroundZoneModel for zone with id #%1")
						.arg(zone.m_id), FUNC_ID);
				}

				// always put the model first into our central model storage
				m_modelContainer.push_back(groundZoneModel); // this container now owns the model
				// register model as time dependend
				m_timeModelContainer.push_back(groundZoneModel);
#endif
			} break;

			case NANDRAD::Zone::NUM_ZT :
				throw IBK::Exception( IBK::FormatString("Undefined or unsupported zone type in zone with id #%1")
													.arg(zone.m_id), FUNC_ID);
		} // switch
	} // for (Zones)

#if 0
	// create model instances for all loads of the given _active_ zones
	for (unsigned int i=0; i<activeZones.size(); ++i) {

		const NANDRAD::Zone  *zone = activeZones[i];

		// *** WallsThermalLoadModel ***

		// for each room balance model create a heat conduction loads model
		// this collects all convective heat transfer fluxes from all walls
		WallsThermalLoadModel * wallsThermalLoadModel = new WallsThermalLoadModel(zone->m_id, zone->m_displayName);
		// initialize heat conduction load model from given construction instance vector
		try {
			wallsThermalLoadModel->setup();
		}
		catch (IBK::Exception & ex) {
			throw IBK::Exception(ex, IBK::FormatString("Error in setup for WallsThermalLoadModel for zone with id #%1")
											.arg(zone->m_id), FUNC_ID);
		}
		// always put the model first into our central model storage
		m_modelContainer.push_back(wallsThermalLoadModel); // this container now owns the model
		// sort into the state model container
		registerStateDependendModel(wallsThermalLoadModel);

		// *** WindowsLoadModel ***

		// for each room balance model create a windows loads model
		// this collects all solar radiation fluxes towards the room
		WindowsLoadModel * windowsLoadModel = new WindowsLoadModel(zone->m_id,
																	zone->m_displayName);
		// initialize heat conduction load model from given construction instance vector
		try {
			windowsLoadModel->setup();
		}
		catch (IBK::Exception & ex) {
			throw IBK::Exception(ex, IBK::FormatString("Error in setup for WindowsLoadModel for zone with id #%1")
											.arg(zone->m_id), FUNC_ID);
		}

		// always put the model first into our central model storage
		m_modelContainer.push_back(windowsLoadModel); // this container now owns the model

		// sort into the state model container
		registerStateDependendModel(windowsLoadModel);

		// *** Long wave radiation balance load model ***

		// for each room balance model create a long wave radiation balance load
		// this collects all long wave radiation fluxes at the windows inside surface and
		// directs it towardsthe room
		LWRadBalanceLoadModel * lWRadBalanceLoadModel = new LWRadBalanceLoadModel(zone->m_id,
																	zone->m_displayName);
		// initialize heat conduction load model from given construction instance vector
		try {
			lWRadBalanceLoadModel->setup(*zone);
		}
		catch (IBK::Exception & ex) {
			throw IBK::Exception(ex, IBK::FormatString("Error in setup for LWRadBalanceLoadModel for zone with id #%1")
											.arg(zone->m_id), FUNC_ID);
		}

		// always put the model first into our central model storage
		m_modelContainer.push_back(lWRadBalanceLoadModel); // this container now owns the model

		// sort into the state model container
		registerStateDependendModel(lWRadBalanceLoadModel);


		// *** Short wave radiation balance load model ***

		// for each room balance model create a short wave radiation balance load
		// this collects all short wave radiation fluxes at the windows inside surface and
		// directs it towards the room
		SWRadBalanceLoadModel * sWRadBalanceLoadModel = new SWRadBalanceLoadModel(zone->m_id,
			zone->m_displayName);
		// initialize heat conduction load model from given construction instance vector
		try {
			sWRadBalanceLoadModel->setup(*zone);
		}
		catch (IBK::Exception & ex) {
			throw IBK::Exception(ex, IBK::FormatString("Error in setup for LWRadBalanceLoadModel for zone with id #%1")
				.arg(zone->m_id), FUNC_ID);
		}

		// always put the model first into our central model storage
		m_modelContainer.push_back(sWRadBalanceLoadModel); // this container now owns the model

														   // sort into the state model container ...
		registerStateDependendModel(sWRadBalanceLoadModel);


		// *** CoolingsLoadModel ***

		// for each room balance model create a windows loads model
		CoolingsLoadModel * coolingsLoadModel = new CoolingsLoadModel(zone->m_id,
																		zone->m_displayName);
		// initialize heat conduction load model from object lists vector
		try {
			coolingsLoadModel->setup(*zone);
		}
		catch (IBK::Exception & ex) {
			throw IBK::Exception(ex, IBK::FormatString("Error in setup for CoolingsLoadModel for zone with id #%1")
											.arg(zone->m_id), FUNC_ID);
		}

		// always put the model first into our central model storage
		m_modelContainer.push_back(coolingsLoadModel); // this container now owns the model

		// sort into the state model container
		registerStateDependendModel(coolingsLoadModel);

		// *** HeatingsLoadModel ***

		// for each room balance model create a windows loads model
		HeatingsLoadModel * heatingsLoadModel = new HeatingsLoadModel(zone->m_id,
																		zone->m_displayName);
		// initialize heat conduction load model from object lists vector
		try {
			heatingsLoadModel->setup(*zone);
		}
		catch (IBK::Exception & ex) {
			throw IBK::Exception(ex, IBK::FormatString("Error in setup for HeatingsLoadModel for zone with id #%1")
											.arg(zone->m_id), FUNC_ID);
		}

		// always put the model first into our central model storage
		m_modelContainer.push_back(heatingsLoadModel); // this container now owns the model

		// sort into the state model container
		registerStateDependendModel(heatingsLoadModel);

		// *** User loads model ***

		UsersThermalLoadModel * userLoadsModel =
			new UsersThermalLoadModel(zone->m_id, zone->m_displayName);
		// initialize heat conduction load model from object lists vector
		try {
			userLoadsModel->setup();
		}
		catch (IBK::Exception & ex) {
			throw IBK::Exception(ex, IBK::FormatString("Error in setup for UsersThermalLoadModel for zone with id #%1")
											.arg(zone->m_id), FUNC_ID);
		}
		// add model to model storage
		m_modelContainer.push_back(userLoadsModel);

		// sort into the state model container structure
		registerStateDependendModel(userLoadsModel);

		// *** Equipment loads model ***

		EquipmentLoadModel * equipmentLoadsModel =
			new EquipmentLoadModel(zone->m_id, zone->m_displayName);
		// initialize heat conduction load model from object lists vector
		try {
			equipmentLoadsModel->setup();
		}
		catch (IBK::Exception & ex) {
			throw IBK::Exception(ex, IBK::FormatString("Error in setup for EquipmentLoadModel for zone with id %1")
											.arg(zone->m_id), FUNC_ID);
		}
		// add model to model storage
		m_modelContainer.push_back(equipmentLoadsModel);

		// sort into the state model container structure
		registerStateDependendModel(equipmentLoadsModel);


		// *** Lighting loads model ***

		LightingLoadModel * lightingLoadModel =
			new LightingLoadModel(zone->m_id, zone->m_displayName);
		// initialize heat conduction load model from object lists vector
		try {
			lightingLoadModel->setup();
		}
		catch (IBK::Exception & ex) {
			throw IBK::Exception(ex, IBK::FormatString("Error in setup for LightingLoadModel for zone with id %1")
											.arg(zone->m_id), FUNC_ID);
		}
		// add model to model storage
		m_modelContainer.push_back(lightingLoadModel);

		// sort into the state model container structure
		registerStateDependendModel(lightingLoadModel);


		// *** Natural ventilation loads model ***

		NaturalVentilationLoadModel * naturalVentilationLoadModel =
			new NaturalVentilationLoadModel(zone->m_id, zone->m_displayName);
		// initialize heat conduction load model from object lists vector
		try {
			naturalVentilationLoadModel->setup(*zone, m_project);
		}
		catch (IBK::Exception & ex) {
			throw IBK::Exception(ex, IBK::FormatString("Error in setup for NaturalVentilationLoadModel for zone with id %1")
											.arg(zone->m_id), FUNC_ID);
		}
		// add model to model storage
		m_modelContainer.push_back(naturalVentilationLoadModel);

		// sort into the state model container
		registerStateDependendModel(naturalVentilationLoadModel);


		// *** Air condition load model ***

		AirConditionLoadModel * airConditionLoadModel =
			new AirConditionLoadModel(zone->m_id, zone->m_displayName);
		// initialize heat conduction load model from object lists vector
		try {
			airConditionLoadModel->setup(*zone, m_project);
		}
		catch (IBK::Exception & ex) {
			throw IBK::Exception(ex, IBK::FormatString("Error in setup for AirConditionLoadModel for zone with id %1")
				.arg(zone->m_id), FUNC_ID);
		}
		// add model to model storage
		m_modelContainer.push_back(airConditionLoadModel);

		// sort into the state model container
		registerStateDependendModel(airConditionLoadModel);


		// *** Domestic water consumption model ***

		DomesticWaterConsumptionModel * domesticWaterConsumptionModel =
			new DomesticWaterConsumptionModel(zone->m_id, zone->m_displayName);
		// initialize heat conduction load model from object lists vector
		try {
			domesticWaterConsumptionModel->setup(*zone, m_project->m_simulationParameter);
		}
		catch (IBK::Exception & ex) {
			throw IBK::Exception(ex, IBK::FormatString("Error in setup for NaturalVentilationThermalLoadModel for zone with id %1")
				.arg(zone->m_id), FUNC_ID);
		}
		// add model to model storage
		m_modelContainer.push_back(domesticWaterConsumptionModel);

		// sort into the state model container
		registerStateDependendModel(domesticWaterConsumptionModel);


		// *** InsideBCWindowsLoadModel ***

		// create a splitting model for radiation loads at all internal surfaces
		InsideBCWindowsLoadModel * qWindowsLoad = new InsideBCWindowsLoadModel(zone->m_id,
																			zone->m_displayName);
		// initialize model
		qWindowsLoad->setup();

		// add model to model storage
		m_modelContainer.push_back(qWindowsLoad);

		// register model as state dependend
		registerStateDependendModel(qWindowsLoad);

		// *** InsideBCLWRadExchangeModel ***

		// create a splitting model for radiation loads at all internal surfaces
		InsideBCLWRadExchangeModel * qLWRadExchange = new InsideBCLWRadExchangeModel(zone->m_id,
																zone->m_displayName);
		// initialize model
		qLWRadExchange->setup(*zone, m_project->m_parametrizationDefaults);

		// add model to model storage
		m_modelContainer.push_back(qLWRadExchange);

		// register model as state dependend
		registerStateDependendModel(qLWRadExchange);


		// *** InsideBCSWRadExchangeModel ***

		// create a splitting model for (diffuse) short wave radiation exchange of all internal surfaces
		InsideBCSWRadExchangeModel * qSWRadExchange = new InsideBCSWRadExchangeModel(zone->m_id,
			zone->m_displayName);
		// initialize model
		qSWRadExchange->setup(*zone, m_project->m_parametrizationDefaults);

		// add model to model storage
		m_modelContainer.push_back(qSWRadExchange);

		// register model as state dependend
		registerStateDependendModel(qSWRadExchange);


		// *** InsideBCHeatingsLoadModel ***

		// create a splitting model for radiation loads at all internal surfaces
		InsideBCHeatingsLoadModel * qHeatingsLoad = new InsideBCHeatingsLoadModel(zone->m_id,
																				zone->m_displayName);
		// initialize model
		qHeatingsLoad->setup();

		// add model to model storage
		m_modelContainer.push_back(qHeatingsLoad);

		// register model as state dependend
		registerStateDependendModel(qHeatingsLoad);


		// *** InsideBCCoolingsLoadModel ***

		// create a splitting model for radiation loads at all internal surfaces
		InsideBCCoolingsLoadModel * qCoolingsLoad = new InsideBCCoolingsLoadModel(zone->m_id,
																				zone->m_displayName);
		// initialize model
		qCoolingsLoad->setup();

		// add model to model storage
		m_modelContainer.push_back(qCoolingsLoad);

		// register model as state dependend
		registerStateDependendModel(qCoolingsLoad);


		// *** InsideBCUsersThermalLoadModel ***

		// create a splitting model for radiation loads at all internal surfaces
		InsideBCUsersThermalLoadModel * qUsersLoad = new InsideBCUsersThermalLoadModel(zone->m_id,
																				zone->m_displayName);
		// initialize model
		qUsersLoad->setup();

		// add model to model storage
		m_modelContainer.push_back(qUsersLoad);

		// register model as state dependend
		registerStateDependendModel(qUsersLoad);

		// *** InsideBCEquipmentLoadModel ***

		// create a splitting model for radiation loads at all internal surfaces
		InsideBCEquipmentLoadModel * qEquipmentLoad = new InsideBCEquipmentLoadModel(zone->m_id,
																				zone->m_displayName);
		// initialize model
		qEquipmentLoad->setup();

		// add model to model storage
		m_modelContainer.push_back(qEquipmentLoad);

		// register model as state dependend
		registerStateDependendModel(qEquipmentLoad);


		// *** InsideBCLightingsLoadModel ***

		// create a splitting model for radiation loads at all internal surfaces
		InsideBCLightingsLoadModel * qLightingLoad = new InsideBCLightingsLoadModel(zone->m_id,
																				zone->m_displayName);
		// initialize model
		qLightingLoad->setup();

		// add model to model storage
		m_modelContainer.push_back(qLightingLoad);

		// register model as state dependend
		registerStateDependendModel(qLightingLoad);


		// *** Moisture sources ***

		if (m_project->m_simulationParameter.m_flags[NANDRAD::SimulationParameter::SF_ENABLE_MOISTURE_BALANCE].isEnabled()) {

			// *** Walls moisture and enthalpy release model ***

			// check for wall moisture calculation
			ConstructionSolverModel::WallMoistureBalanceCalculationMode wallCalcMode
				= ConstructionSolverModel::CM_None;

			if (m_project->m_simulationParameter.m_flags[NANDRAD::SimulationParameter::SF_ENABLE_MOISTURE_BALANCE].isEnabled())
			{
				const std::string wallModeStr = m_project->m_simulationParameter.m_stringPara[NANDRAD::SimulationParameter::
					SSP_WALLMOISTUREBALANCECALCULATIONMODE];

				try {
					wallCalcMode = (ConstructionSolverModel::WallMoistureBalanceCalculationMode)
						KeywordList::Enumeration("ConstructionSolverModel::WallMoistureBalanceCalculationMode",
							wallModeStr);
				}
				catch (IBK::Exception) {
					throw IBK::Exception(IBK::FormatString("Unknown value '%1' for string parameter "
						"'%2' in SimulationParameter tag!")
						.arg(wallModeStr)
						.arg(NANDRAD::KeywordList::Keyword("SimulationParameter::stringPara_t",
							NANDRAD::SimulationParameter::SSP_WALLMOISTUREBALANCECALCULATIONMODE)),
						FUNC_ID);
				}
			}

			WallsMoistureLoadModel * wallsMoistLoadModel =
				new WallsMoistureLoadModel(zone->m_id, zone->m_displayName);
			// initialize heat conduction load model from object lists vector
			try {
				wallsMoistLoadModel->setup(wallCalcMode);
			}
			catch (IBK::Exception & ex) {
				throw IBK::Exception(ex, IBK::FormatString("Error in setup for NaturalVentilationMoistureLoadModel "
					"for zone with id %1")
					.arg(zone->m_id), FUNC_ID);
			}
			// add model to model storage
			m_modelContainer.push_back(wallsMoistLoadModel);

			// sort into the state model container
			registerStateDependendModel(wallsMoistLoadModel);

			// *** User mositure sources model ***


			MoistureLoadModel * moistLoadModel =
				new MoistureLoadModel(zone->m_id, zone->m_displayName);
			// initialize heat conduction load model from object lists vector
			try {
				moistLoadModel->setup();
			}
			catch (IBK::Exception & ex) {
				throw IBK::Exception(ex, IBK::FormatString("Error in setup for UsersMoistureLoadModel for zone with id %1")
					.arg(zone->m_id), FUNC_ID);
			}
			// add model to model storage
			m_modelContainer.push_back(moistLoadModel);

			// sort into the state model container
			registerStateDependendModel(moistLoadModel);

			UsersMoistureLoadModel * userMoistLoadModel =
				new UsersMoistureLoadModel(zone->m_id, zone->m_displayName);
			// initialize heat conduction load model from object lists vector
			try {
				userMoistLoadModel->setup();
			}
			catch (IBK::Exception & ex) {
				throw IBK::Exception(ex, IBK::FormatString("Error in setup for UsersMoistureLoadModel for zone with id %1")
					.arg(zone->m_id), FUNC_ID);
			}
			// add model to model storage
			m_modelContainer.push_back(userMoistLoadModel);

			// sort into the state model container
			registerStateDependendModel(userMoistLoadModel);
		}

		// *** CO2 sources ***

		if (m_project->m_simulationParameter.m_flags[NANDRAD::SimulationParameter::SF_ENABLE_CO2_BALANCE].isEnabled()) {

			// *** User CO2 emission model ***

			UsersCO2LoadModel * userCO2LoadModel =
				new UsersCO2LoadModel(zone->m_id, zone->m_displayName);
			// initialize heat conduction load model from object lists vector
			try {
				userCO2LoadModel->setup();
			}
			catch (IBK::Exception & ex) {
				throw IBK::Exception(ex, IBK::FormatString("Error in setup for UsersCO2LoadModel for zone with id %1")
					.arg(zone->m_id), FUNC_ID);
			}
			// add model to model storage
			m_modelContainer.push_back(userCO2LoadModel);

			// sort into the state model container
			registerStateDependendModel(userCO2LoadModel);
		}

		// *** ScheduledZoneParameterModel ***

		ScheduledZoneParameterModel * scheduledZoneParameterModel =
			new ScheduledZoneParameterModel;
		// initialize model
		scheduledZoneParameterModel->setup(*zone, *m_schedules);
		// add model to model storage
		m_modelContainer.push_back(scheduledZoneParameterModel);

		// register model as state dependend
		registerStateDependendModel(scheduledZoneParameterModel);

		// validity check of parameter names is done inside "initInputReferences" after all models are setup

		// *** ThermalComfortModel ***
		ThermalComfortModel * thermalComfortModel =
			new ThermalComfortModel(zone->m_id, zone->m_displayName);
		// initialize model
		thermalComfortModel->setup();
		// add model to model storage
		m_modelContainer.push_back(thermalComfortModel);
		// classify model as time dependent

		// register model as state dependend
		registerStateDependendModel(thermalComfortModel);


		// *** EnergyIndicatorModel ***
		EnergyPerformanceModel * energyPerformanceModel =
			new EnergyPerformanceModel(zone->m_id, zone->m_displayName);
		// initialize model
		energyPerformanceModel->setup(*zone);
		// add model to model storage
		m_modelContainer.push_back(energyPerformanceModel);
		// register model as state dependend
		registerStateDependendModel(energyPerformanceModel);
	}

#endif

	m_nZones = (unsigned int) m_roomBalanceModelContainer.size();
	IBK::IBK_Message( IBK::FormatString("%1 active zones.\n").arg(m_nZones), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_INFO);
}


void NandradModel::initWallsAndInterfaces() {
	FUNCID(NandradModel::initWallsAndInterfaces);

	// now process all construction types and:
	// - check for valid parameters
	// - check for valid material references (and update these material refs)

	for (unsigned int i=0; i<m_project->m_constructionTypes.size(); ++i) {
		NANDRAD::ConstructionType & ct = m_project->m_constructionTypes[i];
		try {
			ct.checkParameters(m_project->m_materials);
		}
		catch (IBK::Exception & ex) {
			throw IBK::Exception(ex, IBK::FormatString("Error initializing construction type #%1 '%2'.").arg(i).arg(ct.m_displayName), FUNC_ID);
		}
	}

	// process all construction instances and:
	// - check if they are connected to at least one room - otherwise
	// - check that the referenced construction type exists and create quick-access reference to construction instance

	for (unsigned int i=0; i<m_project->m_constructionInstances.size(); ++i) {
		NANDRAD::ConstructionInstance & ci = m_project->m_constructionInstances[i];
		try {
			ci.checkParameters(m_project->m_constructionTypes);

			// we now have all parameters needed to create the ConstructionStatesModel and ConstructionsBalanceModel and
			// associated boundary condition models

			/// \todo filter out constructions that have no impact on calculation results of any zones...
			///       but beware, this is tricky! Maybe it is best to keep the construction but later flag it as "unused"
			///       and simply skip over it during calculation.

			// *** construction states model ***

			ConstructionStatesModel * statesModel = new ConstructionStatesModel(ci.m_id, ci.m_displayName);
			m_modelContainer.push_back(statesModel); // transfer ownership

			IBK::IBK_Message(IBK::FormatString("Initializating construction model (id=%1)\n").arg(ci.m_id),
							 IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_INFO);
			IBK_MSG_INDENT;
			// does the entire initialization
			statesModel->setup(ci, m_project->m_simulationParameter, m_project->m_solverParameter);

			// remember model in states container, so that we can call update(y)
			m_constructionStatesModelContainer.push_back(statesModel);


			// *** construction balance model ***

			// now also initialize balance model - hereby re-using data from states model
			ConstructionBalanceModel * balanceModel = new ConstructionBalanceModel(ci.m_id, ci.m_displayName);
			m_modelContainer.push_back(balanceModel); // transfer ownership

			// does the entire initialization - balance model reuses data from statesModel
			balanceModel->setup(ci, statesModel);

			// register model for evaluation
			registerStateDependendModel(balanceModel);

			// remember model in balance container, so that we can call ydot(ydot)
			m_constructionBalanceModelContainer.push_back(balanceModel);

		}
		catch (IBK::Exception & ex) {
			throw IBK::Exception(ex, IBK::FormatString("Error initializing construction instance #%1 '%2'.").arg(i).arg(ci.m_displayName), FUNC_ID);
		}

	}
	m_nWalls = (unsigned int) m_constructionBalanceModelContainer.size();
	IBK::IBK_Message( IBK::FormatString("%1 construction models.\n").arg(m_nZones), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_INFO);

}


void NandradModel::initEmbeddedObjects() {

}


void NandradModel::initObjectLists() {
	FUNCID(NandradModel::initObjectLists);
	IBK::IBK_Message(IBK::FormatString("Initializing Object Lists\n"), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
	IBK_MSG_INDENT;

	// All object lists are checked for valid parameters.
	// All ID groups are resolved and populated with lists of available model object IDs.
	// Afterwards, it is possible to just use NANDRAD::ObjectList::m_ids when checking for IDs, however,
	// testing with contains() may be faster (especially, when all IDs is set).

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

		// resolve IDs by searching through model objects
		std::set<unsigned int> resolvedIds;
		// insert all models that match current definition
		for (unsigned int i = 0; i < m_modelContainer.size(); ++i) {
			const AbstractModel *model = m_modelContainer[i];
			//skip models with wrong reference type
			if (model->referenceType() != objectlist.m_referenceType)
				continue;
			// fill model ids that are inside the defined id space
			if (objectlist.m_filterID.contains(model->id()) ) {
				// fill the resolved list
				resolvedIds.insert(model->id());
				continue;
			}
		}
		// fill the id filter
		objectlist.m_filterID.m_ids.insert(resolvedIds.begin(), resolvedIds.end());

		// set 0-id for schedules and location
		if (objectlist.m_referenceType == NANDRAD::ModelInputReference::MRT_LOCATION ||
			objectlist.m_referenceType == NANDRAD::ModelInputReference::MRT_SCHEDULE)
		{
			objectlist.m_filterID.m_ids.insert(0);
		}
	}

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

	// The key is of type ValueReference, a simply class that identifies a variable based on reference type and id
	// (both addressing an object) and variable name (identifying the variable of the object).
	// The map can be used to quickly find the object the holds a required result variable.
	// It maps ValueReference (i.e. global identification of a result variable) to the object that provides this variable.
	std::map<ValueReference, AbstractModel*> modelResultReferences;

	// prepare for parallelization - get number of threads and prepare thread-storage vectors
#if defined(_OPENMP)
	// for openMP we need to collect vector within each loop and merge them into the central map together - this avoids synchronization overhead during runtime
	// since each thread can operate in its own memory vector
	std::vector<std::vector<std::pair<ValueReference, AbstractModel*> > > modelResultReferencesVec(m_numThreads);
	std::vector<std::string> threadErrors(m_numThreads);
#endif

#pragma omp parallel for schedule(static,200)
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
				const QuantityDescription &resDesc = resDescs[j];
				// now create our "key" data type for the lookup map
				ValueReference resRef;
				// In case you don't know the syntax below: we copy the base class attributes to
				// the derived class by casting the derived class to the base class and thus using
				// base class to base class assignment operator.
				static_cast<QuantityDescription&>(resRef) = resDesc;
				// store additional information for object lookup (not included in QuantityDescription)
				resRef.m_id = currentModel->id();
				resRef.m_referenceType = currentModel->referenceType();

#if !defined(_OPENMP)
				IBK_FastMessage(IBK::VL_DETAILED)(IBK::FormatString("%1(id=%2).%3 [%4]\n")
												  .arg(NANDRAD::KeywordList::Keyword("ModelInputReference::referenceType_t",resRef.m_referenceType))
												  .arg(resRef.m_id).arg(resRef.m_name).arg(resDesc.m_unit), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_DETAILED);
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


	// *** initializing model input references ***

	delete indent.release();
	IBK::IBK_Message(IBK::FormatString("Initializing all model input references\n"), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
	indent.reset(new IBK::MessageIndentor);

#pragma omp parallel for schedule(static,200)
	for (int i = 0; i < (int) m_modelContainer.size(); ++i) {
		// progress is handled by master thread only
#if defined(_OPENMP)
		if (omp_get_thread_num()==0) {
#else
		{
#endif
			if (timer.intervalCompleted()) // side-effect guarded by _OPENMP ifdef
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
				srcVarAddress = m_fmiInputOutput->resolveResultReference(inputRef, quantityDesc);


				std::string lookupErrorMessage;
				// 2. regular lookup (only if not yet found in FMU)
				if (srcVarAddress == nullptr) {
					// compose search key - for vector valued quantities we ignore the index in ValueReference,
					// since we only want to find the object that actually provides the *variable*
					ValueReference valueRef;
					valueRef.m_id = inputRef.m_id;
					valueRef.m_referenceType = inputRef.m_referenceType;
					valueRef.m_name = inputRef.m_name.m_name;

					std::map<ValueReference, AbstractModel*>::const_iterator it = modelResultReferences.find(valueRef);
					if (it != modelResultReferences.end()) {
						// remember source object's pointer, to create the dependency graph afterwards
						srcObject = it->second;
						quantityDesc = it->first; // Note: slicing is ok here, i.e. we go from ValueReference to QuantityDescription and loose id and reference type on the way, but that's ok.
						// request the address to the requested variable from the source object
						try {
							srcVarAddress = srcObject->resultValueRef(inputRef.m_name);
							if (srcVarAddress == nullptr)
								throw IBK::Exception("Object return nullptr reference for exported variable.", FUNC_ID);
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
							throw IBK::Exception(IBK::FormatString("Could not resolve reference to quantity %1 of %2 with id #%3!")
								.arg(inputRef.m_name.m_name)
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

#if defined(_OPENMP)
	// error checking
	for (int i=0; i<m_numThreads; ++i)
		if (!threadErrors[i].empty()) {
			throw IBK::Exception(threadErrors[i], FUNC_ID);
		}

	for (unsigned int i=0; i<(unsigned int)m_numThreads; ++i) {
		for (unsigned int j=0; j<modelInputReferencesVec[i].size(); ++j)
			modelInputReferences[ modelInputReferencesVec[i][j].first] = modelInputReferencesVec[i][j].second;
	}
#endif

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
	IBK::IBK_Message( IBK::FormatString("Initializing Output Quantity List\n"), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_INFO);
	IBK_MSG_INDENT;
	// generate and dump calculation results of all models
	std::map<std::string, QuantityDescription> refDescs;
	for (unsigned int i=0; i<m_modelContainer.size(); ++i) {
		AbstractModel * currentModel = m_modelContainer[i];
		NANDRAD::ModelInputReference::referenceType_t refType = currentModel->referenceType();
		// skip models that do not generate outputs
		if (refType == NANDRAD::ModelInputReference::NUM_MRT)
			continue;
		std::vector<QuantityDescription> varDescs;
		currentModel->resultDescriptions(varDescs);
		try {
			std::string refTypeName = NANDRAD::KeywordList::Keyword("ModelInputReference::referenceType_t", refType);
			for (unsigned int j=0; j<varDescs.size(); ++j) {
				std::stringstream description;
				if (varDescs[j].m_size == 0)
					continue;
				description << refTypeName << "." << varDescs[j].m_name;
				if (!varDescs[j].m_indexKeys.empty())
					description << "(" << varDescs[j].m_size << ")";
				refDescs[description.str()] = varDescs[j];

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
					refDescs[reducedDescription.str()] = reducedDesc;
				}
			}
		}
		catch (IBK::Exception & ex) {
			IBK::IBK_Message(ex.what(), IBK::MSG_ERROR, FUNC_ID);
		}
	}

	/// \todo append scheduled quantities

	// dump output reference list to file
#ifdef _WIN32
	#if defined(_MSC_VER)
		std::ofstream outputList( (m_dirs.m_varDir / "output_reference_list.txt").wstr().c_str() );
	#else
		std::string outputStrAnsi = IBK::WstringToANSI((m_dirs.m_varDir / "output_reference_list.txt").wstr(), false);
		std::ofstream outputList(outputStrAnsi.c_str());
	#endif
#else
	std::ofstream outputList( (m_dirs.m_varDir / "output_reference_list.txt").str().c_str() );
#endif

	for (std::map<std::string, QuantityDescription>::const_iterator it = refDescs.begin();
		it != refDescs.end(); ++it)
	{
		std::stringstream strm;
		strm << std::setw(30) << std::left << it->first << '\t'
			 << std::setw(10) << std::left << ("[" + it->second.m_unit + "]") << '\t'
			 << it->second.m_description << std::endl;
		IBK::IBK_Message( IBK::FormatString("%1").arg(strm.str()), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_INFO);

		outputList << strm.rdbuf();
	}
	outputList.flush();
	outputList.close();
}


void NandradModel::initSolverVariables() {
	FUNCID(NandradModel::initSolverVariables);

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
		m_weightsFactorZones = (double) (m_n - m_nZones)/ (double) m_nWalls;
		// ensure that we are larger then 1
		IBK_ASSERT(m_weightsFactorZones >= 1.0);
	}

#if 0
	// *** count number of unknowns in explicit models and initialize corresponding offsets ***

	if (!m_ODEStatesAndBalanceModelContainer.empty()) {
		m_ODEVariableOffset.resize(m_ODEStatesAndBalanceModelContainer.size());
		for (unsigned int i = 0; i < m_ODEStatesAndBalanceModelContainer.size(); ++i) {
			AbstractODEBalanceModel *balanceModel = m_ODEStatesAndBalanceModelContainer[i].second;
			// calculate position inside y-vector
			m_ODEVariableOffset[i] = m_n;
			// update solver quantity size
			m_n += balanceModel->n();
		}
	}
#endif

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

		// we list y-vector as results: rows and columns 0 -> nY
		// all rooms
		for (unsigned int i = 0; i < m_roomStatesModelContainer.size(); ++i) {

			const RoomStatesModel *roomModel = m_roomStatesModelContainer[i];
			// access internal y-vector
			const double *y = roomModel->resultValueRef(QuantityName("y"));

			// map storing local valueRefs according to their value pointers
			std::set<const double*> registeredLocalValueRefs;
			// sort values into glibal data container
			for (unsigned int k = 0; k < roomModel->nPrimaryStateResults(); ++k) {
				resultValueRefs[y + k] = nYStates + k;
				// register in local container
				registeredLocalValueRefs.insert(y + k);
			}
			nYStates += roomModel->nPrimaryStateResults();

			// find all other unknowns
			std::vector< std::pair<const double *, const double *> > dependenciesIJ;
			// temporarilly store all elements that can be removed and that one that can be inserted
			roomModel->stateDependencies(dependenciesIJ);

			std::vector<std::pair<const double *, const double *> >::const_iterator
				dependencyIJ = dependenciesIJ.begin();
			// loop over all result quantities of the next pattern (elements aij)
			for (; dependencyIJ != dependenciesIJ.end(); ++dependencyIJ)
			{
				// retrieve row and column storage adresses
				const double * valueRef = dependencyIJ->first;

				// already stored
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

		// ... all constructions
		for (unsigned int i = 0; i < m_constructionStatesModelContainer.size(); ++i) {

			const ConstructionStatesModel *constructionModel = m_constructionStatesModelContainer[i];
			// access internal y-vector
			const double *y = constructionModel->resultValueRef(QuantityName("y"));

			// map storing local valueRefs according to their value pointers
			std::set<const double*> registeredLocalValueRefs;
			// sort values into glibal data container
			for (unsigned int k = 0; k < constructionModel->nPrimaryStateResults(); ++k) {
				resultValueRefs[y + k] = nYStates + k;
				// register in local container
				registeredLocalValueRefs.insert(y + k);
			}
			nYStates += constructionModel->nPrimaryStateResults();

			// find all other unknowns
			std::vector< std::pair<const double *, const double *> > dependenciesIJ;
			// temporarilly store all elements that can be removed and that one that can be inserted
			constructionModel->stateDependencies(dependenciesIJ);

			std::vector<std::pair<const double *, const double *> >::const_iterator
				dependencyIJ = dependenciesIJ.begin();
			// loop over all result quantities of the next pattern (elements aij)
			for (; dependencyIJ != dependenciesIJ.end(); ++dependencyIJ)
			{
				// retrieve row and column storage adresses
				const double * valueRef = dependencyIJ->first;

				// already stored
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

		// counted y-components must equal global y-vector size
		IBK_ASSERT(nYStates == (unsigned int) m_y.size());

		// next we list ydot-vector as results:: row and column nY -> nY + nYdot - 1
		// for all rooms
		for (unsigned int i = 0; i < m_roomBalanceModelContainer.size(); ++i) {

			const RoomBalanceModel *roomModel = m_roomBalanceModelContainer[i];
			// access internal y-vector
			const double *ydot = roomModel->resultValueRef(QuantityName("ydot"));

			// map storing local valueRefs according to their value pointers
			std::set<const double*> registeredLocalValueRefs;
			// sort values into glibal data container
			for (unsigned int k = 0; k < m_roomStatesModelContainer[i]->nPrimaryStateResults();
				++k) {
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
			for (; dependencyIJ != dependenciesIJ.end(); ++dependencyIJ)
			{
				// retrieve row and column storage adresses
				const double * valueRef = dependencyIJ->first;

				// already stored
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

		// ... and for all constructions
		for (unsigned int i = 0; i < m_constructionBalanceModelContainer.size(); ++i) {

			const ConstructionBalanceModel *constructionModel =
				m_constructionBalanceModelContainer[i];
			// access internal y-vector
			const double *ydot = constructionModel->resultValueRef(QuantityName("ydot"));

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
			for (; dependencyIJ != dependenciesIJ.end(); ++dependencyIJ)
			{
				// retrieve row and column storage adresses
				const double * valueRef = dependencyIJ->first;

				// already stored
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

		// counted ydot-components must equal global ydot-vector size
		IBK_ASSERT(nYdotStates == (unsigned int)m_ydot.size());


		// loop through all models and select algebraic result quantities: row/column nY + nYdot -> nUnknowns
		for (unsigned int i = 0; i < m_stateModelContainer.size(); ++i) {


			AbstractStateDependency *stateDep = m_stateModelContainer[i];
			// skip room balance models and construction balance models
			if (dynamic_cast<RoomBalanceModel*> (stateDep) != NULL)
				continue;
			if (dynamic_cast<ConstructionBalanceModel*> (stateDep) != NULL)
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
			for (; dependencyIJ != dependenciesIJ.end(); ++dependencyIJ)
			{
				// retrieve row and column storage adresses
				const double * valueRef = dependencyIJ->first;

				// already stored
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

		/// create a sparse matrix pattern and a transpose pattern
		IBKMK::SparseMatrixPattern pattern(nUnknowns);
		IBKMK::SparseMatrixPattern transposePattern(nUnknowns);

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
			for (; dependencyIJ != dependenciesIJ.end(); ++dependencyIJ)
			{
				// retrieve row and column storage adresses
				const double * resultRef = dependencyIJ->first;
				const double * inputRef = dependencyIJ->second;

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
			for (; dependencyIJ != dependenciesIJ.end(); ++dependencyIJ)
			{
				// retrieve row and column storage adresses
				const double * resultRef = dependencyIJ->first;
				const double * inputRef = dependencyIJ->second;

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
			for (; dependencyIJ != dependenciesIJ.end(); ++dependencyIJ)
			{
				// retrieve row and column storage adresses
				const double * resultRef = dependencyIJ->first;
				const double * inputRef = dependencyIJ->second;

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
#ifdef _WIN32
	#if defined(_MSC_VER)
		m_progressLog = new std::ofstream( (m_dirs.m_logDir / "progress.tsv").wstr().c_str(), std::ios_base::app);
	#else
		std::string dirStrAnsi = IBK::WstringToANSI((m_dirs.m_logDir / "progress.tsv").wstr(), false);
		m_progressLog = new std::ofstream(dirStrAnsi.c_str(), std::ios_base::app);
	#endif
#else
		m_progressLog = new std::ofstream( (m_dirs.m_logDir / "progress.tsv").str().c_str(), std::ios_base::app);
#endif
	}
	else {
		m_elapsedSecondsAtStart = 0;
		m_elapsedSimTimeAtStart = t0();
		// open progressLog file for writing and write header
#ifdef _WIN32
	#if defined(_MSC_VER)
		m_progressLog = new std::ofstream( (m_dirs.m_logDir / "progress.tsv").wstr().c_str());
	#else
		std::string dirStrAnsi = IBK::WstringToANSI((m_dirs.m_logDir / "progress.tsv").wstr(), false);
		m_progressLog = new std::ofstream(dirStrAnsi.c_str());
	#endif
#else
		m_progressLog = new std::ofstream( (m_dirs.m_logDir / "progress.tsv").str().c_str());
#endif
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
			m_constructionStatesModelContainer[i]->setY(&m_y[0] + m_constructionVariableOffset[i]);
		}
}
#endif // _OPENMP
	if (m_useSerialCode) {
		for (unsigned int i = 0; i < m_constructionStatesModelContainer.size(); ++i) {
			m_constructionStatesModelContainer[i]->update(&m_y[0] + m_constructionVariableOffset[i]);
		}
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


	// update output file objects (they never publish results and are always evaluated last)

	/// \todo create container for output files and loop over container


	// mark solution as updated
	m_yChanged = false;

	// signal success
	return 0;
}


} // namespace NANDRAD_MODEL


