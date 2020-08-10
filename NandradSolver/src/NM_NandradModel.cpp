/*	The Nandrad model library.

Copyright (c) 2012, Institut fuer Bauklimatik, TU Dresden, Germany

Written by
A. Paepcke		<anne.paepcke -[at]- tu-dresden.de>
A. Nicolai		<andreas.nicolai -[at]- tu-dresden.de>
All rights reserved.

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

namespace NANDRAD_MODEL {

NandradModel::NandradModel() :
	m_project(new NANDRAD::Project)
{
}


NandradModel::~NandradModel() {
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
}


void NandradModel::init(const NANDRAD::ArgsParser & args) {
	FUNCID(NandradModel::init);

	// *** Write Information about project file and relevant directories ***

	IBK::IBK_Message( IBK::FormatString("Executable path:    '%1'\n").arg(args.m_executablePath), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_INFO);
	IBK::IBK_Message( IBK::FormatString("Project file:       '%1'\n").arg(args.m_projectFile), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
	IBK::IBK_Message( IBK::FormatString("Project directory:  '%1'\n").arg(m_projectFilePath), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_INFO);

	IBK::IBK_Message( IBK::FormatString("Output root dir:    '%1'\n").arg(m_dirs.m_rootDir), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
	{
		IBK::MessageIndentor indent; (void)indent;
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
		IBK::MessageIndentor indent; (void)indent;
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
	IBK::MessageIndentor indent; (void)indent;

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
	// *** Initialize SubModels ***
//	initExplicitModels();
	// *** Initialize all internal fmus ***
//	initFMUComponents();
	// *** Initialize Object Lists ***
	initObjectLists();
	// *** Setup model dependencies ***
	initModelDependencies();
	// *** Setup states model graph and generate model groups ***
	initModelGraph();
	// *** Initialize list with output references ***
	initOutputReferenceList();
	// *** Initialize outputs ***
	initOutputs(args.m_restart || args.m_restartFrom);
	// *** Check validity for all model initial values ***
//	prepareInitialModelCalculation();
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
	return 0.1; // m_project->m_solverParameter.m_para[NANDRAD::SolverParameter::SP_INITIAL_DT].value;
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
	// \todo memcpy y -> m_y
#if 0
	// map input y to model y
	bool different = false;
	for (unsigned int i=0; i<m_n; ++i) {
		if (m_y[/* m_yIndexMap[*/i/*]*/ ] != y[i]) {
			different = true;
			m_y[ /*m_yIndexMap[*/i/*] */] = y[i];
		}
	}

	// mark model as outdated
	if (different)
		m_yChanged = true;
#endif
	// fluxes and divergences are computed in ydot()
	return SOLFRA::ModelInterface::CalculationSuccess;
}


SOLFRA::ModelInterface::CalculationResult NandradModel::ydot(double * ydot) {
	const char * const FUNC_ID = "[NandradModel::ydot]";
#if 0
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

		// *** map model ydot to output ydot ***
		for (unsigned int i=0; i<m_n; ++i)
			ydot[i] = m_ydot[ /*m_yIndexMap[*/i/*]*/ ];

		// *** feedback to user ***
		IBK_FastMessage(IBK::VL_DETAILED)(IBK::FormatString("    ydot: t=%1 [%2]\n")
							.arg(m_t).arg(IBK::Time(2000,m_t).toTOYFormat()),
							IBK::MSG_PROGRESS, FUNC_ID, 3);
		m_feedback.writeFeedbackFromF(m_t);
	}
	catch (IBK::Exception & ex) {
		throw IBK::Exception(ex, "Error retrieving divergences!", FUNC_ID);
	}
#endif
	return SOLFRA::ModelInterface::CalculationSuccess;
}


void NandradModel::writeOutputs(double t_out, const double * y_out) {
	// update state of model to output time and variables
	// finally write the output
	setTime(t_out);
	setY(y_out);
	ydot(nullptr);

#if 0
	// call step completed for several mdoels
	for (unsigned int i = 0; i < m_stepCompletedForOutputWriting.size(); ++i) {
		m_stepCompletedForOutputWriting[i]->stepCompleted(t_out);
	}
	// call step completed for several mdoels
	for (unsigned int i = 0; i < m_steadyStateModelContainer.size(); ++i) {
		m_steadyStateModelContainer[i]->stepCompleted(t_out, y_out);
	}

	// call ydot() to trigger calculation?

	// move (relative) simulation time to absolute time (offset to midnight, January 1st of the start year)
	double t_secondsOfYear = t_out + m_project->m_simulationParameter.m_interval.m_para[NANDRAD::Interval::IP_START].value;

	for(unsigned int i = 0; i < m_outputFiles.size(); ++i) {
		m_outputFiles[i]->writeOutputs(t_secondsOfYear, y_out, nullptr);
	}
	// write wall solver outputs
	for(unsigned int i = 0; i< m_wallSolverModelContainer.size(); ++i) {
		IBK_ASSERT(m_wallSolverModelContainer[i] != nullptr);
		m_wallSolverModelContainer[i]->writeOutputs(t_secondsOfYear);
	}

#endif

	// write feedback to user
	IBK_ASSERT(m_t == t_out);
	m_feedback.writeFeedback(t_out, false);
}


std::string NandradModel::simTime2DateTimeString(double t) const {
	// add start time offset to t and then call parent function
	int startYear = m_project->m_simulationParameter.m_intpara[NANDRAD::SimulationParameter::SIP_YEAR].value;
	t += m_project->m_simulationParameter.m_interval.m_para[NANDRAD::Interval::IP_START].value;
	return IBK::Time(startYear, t).toShortDateFormat();
}


void NandradModel::stepCompleted(double t, const double *y ) {
#if 0

	// tell all modules that need to remember the last integration step, that they
	// can cache a new state
	for (std::vector<AbstractTimeDependency*>::iterator it = m_timeModelContainer.begin();
		it != m_timeModelContainer.end(); ++it)
	{
		(*it)->stepCompleted(t);
	}

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
	IBK::MessageIndentor indent; (void)indent;
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
	IBK::MessageIndentor indent; (void)indent;

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
	IBK::MessageIndentor indent; (void)indent;

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
	if (minDx.value <= 0)
		throw IBK::Exception("Invalid parameter for DiscMinDx in SolverParameter settings.", FUNC_ID);

	const IBK::Parameter & density = solverParameter.m_para[NANDRAD::SolverParameter::SP_DISCRETIZATION_DETAIL];
	if (density.value < 1 && density.value != 0)
		throw IBK::Exception("Invalid parameter for DiscDetailLevel in SolverParameter settings.", FUNC_ID);

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
			m_project->m_solverParameter.m_lesSolver = NANDRAD::SolverParameter::LES_GMRES;
			m_project->m_solverParameter.m_preconditioner = NANDRAD::SolverParameter::PRE_ILU;
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

	// check validity of the parameter data
	const IBK::Parameter &gammaRad = simPara.m_para[NANDRAD::SimulationParameter::SP_RADIATION_LOAD_FRACTION];
	if (gammaRad.name.empty()) {
		throw IBK::Exception(IBK::FormatString("Error initializing wall solver: "
			"SimulationParameter 'GammaRad' is not defined."), FUNC_ID);
	}

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
}


void NandradModel::initClimateData() {
	FUNCID(NandradModel::initClimateData);

	IBK::IBK_Message(IBK::FormatString("Initializing Climatic Data\n"), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
	IBK::MessageIndentor indent; (void)indent;

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
	IBK::MessageIndentor indent; (void)indent;

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
	IBK::IBK_Message(IBK::FormatString("Initializing FMI import/export\n"), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
	IBK::MessageIndentor indent; (void)indent;


	try {
		m_fmiInputOutput = new FMIInputOutput;
		// insert into model container
		m_modelContainer.push_back(m_loads);	// now owns the model and handles memory cleanup
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
	const char * const FUNC_ID = "[NandradModelImpl::initActiveZones]";
	IBK::IBK_Message( IBK::FormatString("Initializing Zones\n"), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_INFO);
	IBK::MessageIndentor indent; (void)indent;

	// create model instances for all active zones
	// for each active zone, we need:
	// - a RoomBalanceModel
	// - a RoomThermalLoadsModel (collects heating loads from model instances)

	// created models are added to m_stateModelContainer (which owns them)
	// a reference is also placed in m_roomStateModelContainer, which is used by the
#if 0

	// remember all zones that require a room state model
	std::vector<const NANDRAD::Zone*> activeZones;
	// process all active zones in list of zones
	for (const NANDRAD::Zone & zone : m_project->m_zones) {

		const NANDRAD::SimulationParameter &simPara = m_project->m_simulationParameter;

		IBK::IBK_Message( IBK::FormatString("Zone [%1] '%2' -> %3\n").arg(zone.m_id).arg(zone.m_displayName)
			.arg( zone.m_type == NANDRAD::Zone::ZT_CONSTANT ? "CONSTANT" : "ACTIVE"), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_INFO);
		switch (zone.m_type) {
			case NANDRAD::Zone::ZT_ACTIVE : {
				// create implicit room state model
				RoomBalanceModel * roomBalanceModel = new RoomBalanceModel(zone.m_id, zone.m_displayName);
				RoomStatesModel * roomStatesModel = new RoomStatesModel(zone.m_id, zone.m_displayName);

				// initialize room state model
				try {
					roomBalanceModel->setup(simPara);
				}
				catch (IBK::Exception & ex) {
					throw IBK::Exception(ex, IBK::FormatString("Error in setup of model '%1' for zone #%2 '%3'")
						.arg(roomBalanceModel->ModelIDName())
						.arg(zone.m_id).arg(zone.m_displayName), FUNC_ID);
				}

				// always put the model first into our central model storage
				m_modelContainer.push_back(roomBalanceModel); // this container now owns the model
				// also remember this model in the container with room state models m_roomBalanceModelContainer
				// because we need to call ydot().
				m_roomBalanceModelContainer.push_back(roomBalanceModel);

				// sort into the state model container ...
//				registerStateDependendModel(roomBalanceModel);
//				m_stateModelContainer.push_back(roomBalanceModel);

				// initialize room state model
				try {
					roomStatesModel->setup(zone);
				}
				catch (IBK::Exception & ex) {
					throw IBK::Exception(ex, IBK::FormatString("Error in setup of model '%1' for zone #%2 '%3'")
						.arg(roomBalanceModel->ModelIDName())
						.arg(zone.m_id).arg(zone.m_displayName), FUNC_ID);
				}


				// always put the model first into our central model storage
				m_modelContainer.push_back(roomStatesModel); // this container now owns the model

																// sort into the state model container - this model only
																// provides references to y and constant project parameters
//				registerStateDependendModel(roomStatesModel);

				// also remember this model in the container with room state models m_roomBalanceModelContainer
				// because we need to call yInitial().
				m_roomStatesModelContainer.push_back(roomStatesModel);
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
			default :
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
#endif

	m_nZones = (unsigned int) m_roomBalanceModelContainer.size();
	IBK::IBK_Message( IBK::FormatString("%1 active zones.\n").arg(m_nZones), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_INFO);
}


void NandradModel::initWallsAndInterfaces()
{

}


void NandradModel::initEmbeddedObjects() {

}


void NandradModel::initObjectLists() {
	// todo

}

void NandradModel::initModelDependencies() {
	FUNCID(NandradModel::initModelDependencies);

	// *** complete initialization for all models ***

	IBK::StopWatch timer;

	// *** initializing model results ***

	IBK::IBK_Message(IBK::FormatString("Initializing all model results\n"), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);


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
		AbstractModel * currentModel = m_modelContainer[i];
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
				resRef.m_id = currentModel->id();
				resRef.m_referenceType = currentModel->referenceType();
				resRef.m_name = resDesc.m_name;

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

	IBK::IBK_Message(IBK::FormatString("Initializing all model input references\n"), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
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

		AbstractModel * currentModel = m_modelContainer[i];
		// currentModel is the model that we current look up input references for
		AbstractStateDependency * currentStateDependency = dynamic_cast<AbstractStateDependency *> (m_modelContainer[i]);
		// skip all models that are not state-dependent and have no input requirements
		if (currentStateDependency == nullptr)
			continue;

		try {
			// tell model to initialize its inputs
			currentStateDependency->initInputReferences(m_modelContainer);

			// request published input variables from model instance
			std::vector<InputReference> inputRefs;
			currentStateDependency->inputReferences(inputRefs);
			// process and lookup all of the variables variables
			for (unsigned int j = 0; j < inputRefs.size(); ++j) {
				const InputReference &inputRef = inputRefs[j];

				// now lookup address of requested variable
				const double * srcVarAddress = nullptr;
				AbstractModel * srcObject = nullptr;

				// 1. FMU interface variables

				// First check FMU import model - it may override variables by providing variables for
				// exactly the same InputReference as those of other models. E.g., an FMU import
				// may generate a variable Zone[13].AirTemperature and thus override the air temperature
				// variable generated by the zone model (instance with id=13) itself. All models using this
				// temperature will use the variable from the FMI import model, instead.
				srcVarAddress = m_fmiInputOutput->resultValueRef(inputRef);


				// 2. regular lookup (only if not yet found in FMU)
				if (srcVarAddress == nullptr) {
					// compose search key - for vector valued quantities we ignore the index in ValueReference,
					// since we only want to find the object that actually provides the *variable*
					ValueReference valueRef;
					valueRef.m_id = m_modelContainer[i]->id();
					valueRef.m_referenceType = m_modelContainer[i]->referenceType();
					valueRef.m_name = inputRef.m_sourceName.m_name;

					std::map<ValueReference, AbstractModel*>::const_iterator it = modelResultReferences.find(valueRef);
					if (it != modelResultReferences.end()) {
						// remember source object's pointer, to create the dependency graph afterwards
						srcObject = it->second;
						// request the address to the requested variable from the source object
						srcVarAddress = srcObject->resultValueRef(inputRef.m_sourceName);
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
					srcVarAddress = m_fmiInputOutput->resultValueRef(inputRef);
				}


				// If we didn't find a pointer to the result variable, yet, raise an exception if the requested variable
				// was a required input
				if (srcVarAddress == nullptr) {
					if (inputRef.m_required) {
						// error: reference was not resolved
						throw IBK::Exception(IBK::FormatString("Could not resolve reference to quantity %1 of %2 with id #%3!")
							.arg(inputRef.m_sourceName.m_name)
							.arg(NANDRAD::KeywordList::Keyword("ModelInputReference::referenceType_t", m_modelContainer[i]->referenceType()))
							.arg(m_modelContainer[i]->id()), FUNC_ID);
					}
					else {
						// if not required, tell the model object that we do not have such an input by giving a nullptr;
						// the model must handle this appropriately
						currentStateDependency->setInputValueRef(inputRef, nullptr);
					}
				}
				else {
					// tell model the persistent memory location of the requested input
					currentStateDependency->setInputValueRef(inputRef, srcVarAddress);
					// register this model as dependency, but only if providing model was an object in the model graph
					// and the input variable is not a constant
					if (srcObject != nullptr && !inputRef.m_constant) {
						// add a graph element
						const ZEPPELIN::DependencyObject *sourceObject = dynamic_cast<ZEPPELIN::DependencyObject*>(srcObject);
						if (sourceObject != nullptr)
							currentStateDependency->dependsOn(*sourceObject);
							// Note: we are calling the const-variant, so that sourceObject won't
							//       be modified and this function remains thread-safe. The parents are set after this
							//       parallel for-loop has completed.
					}
				}
			}
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
			throw IBK::Exception(ex, IBK::FormatString("Error initializing input references for model %1 with id #%2!")
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

void NandradModel::initModelGraph()
{

}

void NandradModel::initOutputReferenceList() {
	const char * const FUNC_ID = "[NandradModelImpl::initOutputReferenceList]";
	IBK::IBK_Message( IBK::FormatString("Initializing Output Quantity List\n"), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_INFO);
	IBK::MessageIndentor indent; (void)indent;
	// generate and dump calculation results of all models
	std::map<std::string, QuantityDescription> refDescs;
	for (unsigned int i=0; i<m_modelContainer.size(); ++i) {
		AbstractModel * currentModel = m_modelContainer[i];
		std::vector<QuantityDescription> varDescs;
		currentModel->resultDescriptions(varDescs);
		NANDRAD::ModelInputReference::referenceType_t refType = currentModel->referenceType();
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
		outputList << std::setw(30) << std::left << it->first << " \t[" << it->second.m_unit << "] \t" << it->second.m_description << std::endl;
		std::stringstream strm;
		strm << std::setw(30) << std::left << it->first << " \t[" << it->second.m_unit << "] \t" << it->second.m_description;
		IBK::IBK_Message( IBK::FormatString("%1\n").arg(strm.str()), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_INFO);
	}
	outputList.flush();
	outputList.close();
}


void NandradModel::initOutputs(bool restart) {
	FUNCID(NandradModel::initOutputs);
	// we create groups of output definitions, first based on target file name

	// in the initial loop, we also create and store pointer references to output grids and object lists referenced
	// from output definitions - in case of invalid definitions, we bail out with an error message
	std::map<std::string, NANDRAD::OutputDefinition> targetFileMap;
	for (unsigned int i=0; i<m_project->m_outputs.m_outputDefinitions.size(); ++i) {

		NANDRAD::OutputDefinition & od = m_project->m_outputs.m_outputDefinitions[i];

		// sanity checks are done on the fly
		if (IBK::trim_copy(od.m_gridName).empty())
			throw IBK::Exception(IBK::FormatString("Output definition #%1 has empty/invalid output grid name.").arg(i), FUNC_ID);

		std::vector<NANDRAD::OutputGrid>::const_iterator grid_it = std::find(m_project->m_outputs.m_grids.begin(),
																			 m_project->m_outputs.m_grids.end(),
																			 od.m_gridName);
		if (grid_it == m_project->m_outputs.m_grids.end())
			throw IBK::Exception(IBK::FormatString("Output definition #%1 references unknown/undefined output grid '%2'").arg(i).arg(od.m_gridName), FUNC_ID);

		// store pointer link to output grid
	}

}

void NandradModel::initSolverVariables()
{

}

void NandradModel::initSolverMatrix()
{

}

void NandradModel::initStatistics(SOLFRA::ModelInterface * modelInterface, bool restart)
{

}

} // namespace NANDRAD_MODEL


