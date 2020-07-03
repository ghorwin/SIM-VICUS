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

#include <DATAIO_Constants.h>
#include <CCM_Constants.h>

#include <NANDRAD_ArgsParser.h>
#include <NANDRAD_Constants.h>
#include <NANDRAD_Project.h>

#include <SOLFRA_IntegratorSundialsCVODE.h>
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

namespace NANDRAD_MODEL {

NandradModel::NandradModel() :
	m_project(new NANDRAD::Project),
	m_lesSolver(nullptr),
	m_jacobian(nullptr),
	m_preconditioner(nullptr),
	m_integrator(nullptr)
{
}


NandradModel::~NandradModel() {
	// free memory of owned instances
	delete m_project;
	delete m_lesSolver;
	delete m_jacobian;
	delete m_preconditioner;
	delete m_integrator;
}


void NandradModel::init(const NANDRAD::ArgsParser & args) {
	FUNCID(NandradModel::init);
#if 0
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
	m_project.initDefaults(nullptr);

	// read input data from file
	IBK::IBK_Message( IBK::FormatString("Reading project file\n"), IBK::MSG_PROGRESS, FUNC_ID, 1);
	m_project.readXML(args.m_projectFile);

	// *** Print Out Placeholders ***
	IBK::IBK_Message( IBK::FormatString("Path Placeholders\n"), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_INFO);
	{
		IBK::MessageIndentor indent; (void)indent;
		for (std::map<std::string, IBK::Path>::const_iterator it = m_project.m_placeholders.begin();
			it != m_project.m_placeholders.end(); ++it)
		{
			std::stringstream strm;
			strm << std::setw(25) << std::left << it->first;
			IBK::IBK_Message( IBK::FormatString("%1 -> %2\n").arg(strm.str()).arg(it->second), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_INFO);
		}
	}

	IBK::IBK_Message( IBK::FormatString("Initializing model\n"), IBK::MSG_PROGRESS, FUNC_ID, 1);
	IBK::MessageIndentor indent; (void)indent;

	// *** Initialize simulation parameters ***
	initSolverParameter(args);
	// *** Initialize project parameters ***
	initProjectParameter(args);
	// *** Select all explicit object list definitions and fill m_objectLists vector to its full size***
	initDefinitions();
	// *** Connects all active zones to their space type object***
	initConnections();
	// *** Initialize construction type and material database ***
	initDatabase();
	// *** Initialize simulation parameters ***
	initParametrizationDefaults();
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
	initZoneLists();
	// *** Remove duplicate construction IDs ***
	m_project.mergeSameConstructions();
	// *** Initialize Wall and Construction BC Modules ***
	initWallsAndInterfaces();
	// *** Initialize Window Models ***
	initEmbeddedObjects();
	// *** Initialize ModelGroups ***
	initModelGroups();
	// *** Initialize SubModels ***
	initExplicitModels();
	// *** Initialize all internal fmus ***
	initFMUComponents();
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
	prepareInialModelCalculation();
	// *** Initialize Global Solver ***
	initSolverVariables();
	// *** Initialize sparse solver matrix ***
	initSolverMatrix();
	// *** Init statistics/feedback output ***
	initStatistics(modelInterface, args.m_restart);
#endif
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
	return 0.1; // m_project.m_solverParameter.m_para[NANDRAD::SolverParameter::SP_INITIAL_DT].value;
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
	// all other models depend on the room state, so we update this model first
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
	double t_secondsOfYear = t_out + m_project.m_simulationParameter.m_interval.m_para[NANDRAD::Interval::IP_START].value;

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
	return std::string();
	// add start time offset to t and then call parent function
//	int startYear = m_project.m_simulationParameter.m_intpara[NANDRAD::SimulationParameter::SIP_YEAR].value;
//	t += m_project.m_simulationParameter.m_interval.m_para[NANDRAD::Interval::IP_START].value;
//	return IBK::Time(startYear, t).toShortDateFormat();
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

#if 0
	if (m_project.m_solverParameter.m_integrator == NANDRAD::SolverParameter::I_EXPLICIT_EULER) {
		IBK::IBK_Message("Linear Equation Solver Modules not needed for Explicit Euler.\n", IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
		return nullptr;
	}

	IBK::IBK_Message("\n", IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
	IBK::IBK_Message("Creating Linear Equation Solver Modules\n", IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
	IBK::MessageIndentor indent; (void)indent;
	IBK::IBK_Message( IBK::FormatString("Number of unknowns: %1\n").arg(m_n), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
	IBK::IBK_Message( IBK::FormatString("Bandwidth: %1\n").arg(bandwidth()), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);

	// compute half band width of system
	/// \todo check this!
	unsigned int lesHbw = std::min((bandwidth() - 1)/2, (m_n - 1)/2 ); // half-band width for 1 equation
	unsigned int preHbw = lesHbw;
	// if given, use bandwidth from project structure
	if (!m_project.m_solverParameter.m_para[NANDRAD::SolverParameter::SP_LES_BANDWIDTH].name.empty())
		lesHbw = (unsigned int)m_project.m_solverParameter.m_para[NANDRAD::SolverParameter::SP_LES_BANDWIDTH].value;
	if (!m_project.m_solverParameter.m_para[NANDRAD::SolverParameter::SP_PRE_BANDWIDTH].name.empty())
		preHbw = (unsigned int)m_project.m_solverParameter.m_para[NANDRAD::SolverParameter::SP_PRE_BANDWIDTH].value;

	// create LES solver based on selected setting
	switch (m_project.m_solverParameter.m_lesSolver) {

		// Block tridiag
		case NANDRAD::SolverParameter::LES_BTRIDIAG: {
			m_lesSolver = new SOLFRA::LESBand(1, 1);
			IBK_Message(IBK::FormatString("Using generic Block tridiag solver!\n"),
				IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
			return m_lesSolver;
		}

		// Band
		case NANDRAD::SolverParameter::LES_BAND : {
			m_lesSolver = new SOLFRA::LESBand(lesHbw, lesHbw);
			IBK_Message( IBK::FormatString("Using generic Band solver with bandwidth %1!\n").arg(lesHbw + lesHbw + 1),
				IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
			return m_lesSolver;
		}

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

		// TFQMR
		case NANDRAD::SolverParameter::LES_TFQMR : {
			m_lesSolver = new SOLFRA::LESTFQMR;
			IBK_Message(IBK::FormatString("Using TFQMR solver\n"),  IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
		} break;

		default:
			throw IBK::Exception("Unknown or undefined LES solver.", FUNC_ID);
	}

	// must be an iterative solver, so configure preconditioners and jacobian matrix generators
	std::string precondName = "no preconditioner";

	// determine preconditioner type
	switch (m_project.m_solverParameter.m_preconditioner) {
		// Band preconditioner
		case NANDRAD::SolverParameter::PRE_BAND : {
			// work with a sparse jacobian
			// m_jacobian = new SOLFRA::JacobianDense();
#ifdef ELLPACK_ITPACK
			SOLFRA::JacobianSparseEID *jacSparse = new SOLFRA::JacobianSparseEID(n(), sparseMatrixElementsPerRow(), sparseMatrixIndices(),true);
			jacSparse->setColoringType(SOLFRA::JacobianSparseEID::Automatic);
#else
			SOLFRA::JacobianSparseCSR *jacSparse = new SOLFRA::JacobianSparseCSR(n(), nnz(), &m_ia[0], &m_ja[0],
				&m_iaT[0], &m_jaT[0]);
#endif
			m_jacobian = jacSparse;
			// create band preconditioner
			SOLFRA::PrecondBand *preconditioner = new SOLFRA::PrecondBand(preHbw, preHbw, SOLFRA::PrecondInterface::Right);
			m_preconditioner = preconditioner;

			precondName = IBK::FormatString("band preconditioner with bandwidth (%1)").arg(preHbw + preHbw + 1).str();
		} break;

		// ILU preconditioner
		case NANDRAD::SolverParameter::PRE_ILU : {
			// work with a sparse jacobian
#ifdef ELLPACK_ITPACK
			SOLFRA::JacobianSparseEID *jacSparse = new SOLFRA::JacobianSparseEID(n(), sparseMatrixElementsPerRow(), sparseMatrixIndices(),true);
			jacSparse->setColoringType(SOLFRA::JacobianSparseEID::Automatic);
#else
			SOLFRA::JacobianSparseCSR *jacSparse = new SOLFRA::JacobianSparseCSR(n(), nnz(), &m_ia[0], &m_ja[0],
				&m_iaT[0], &m_jaT[0]);
#endif
			m_jacobian = jacSparse;

			// ILUT preconditioner
			if (!m_project.m_solverParameter.m_para[NANDRAD::SolverParameter::SP_PRE_ILUWIDTH].name.empty()/*
				&& m_project.m_solverParameter.m_para[NANDRAD::SolverParameter::SP_PRE_ILUWIDTH].value > 0*/) {
				unsigned int fillIn = (unsigned int)m_project.m_solverParameter.m_para[NANDRAD::SolverParameter::SP_PRE_ILUWIDTH].value;

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
	};

	SOLFRA::LESInterfaceIterative * lesIter = dynamic_cast<SOLFRA::LESInterfaceIterative *>(m_lesSolver);
	IBK_ASSERT(lesIter != nullptr);

	// set iterative LES solver options
	lesIter->m_maxKrylovDim = (unsigned int)m_project.m_solverParameter.m_para[NANDRAD::SolverParameter::SP_MAX_KRYLOV_DIM].value;
	lesIter->m_linIterConvCoeff = m_project.m_solverParameter.m_para[NANDRAD::SolverParameter::SP_ITERATIVESOLVERCONVCOEFF].value;

	IBK_Message(IBK::FormatString("%1 selected, MaxKrylovDim = %2\n")
		.arg(precondName).arg(lesIter->m_maxKrylovDim),  IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
#endif
	return m_lesSolver;
}


SOLFRA::JacobianInterface *  NandradModel::jacobianInterface() {
	return m_jacobian;
}


SOLFRA::IntegratorInterface * NandradModel::integratorInterface() {
#if 0
	const char * const FUNC_ID = "[NandradModel::integratorInterface]";
	if (m_integrator != nullptr)
		return m_integrator;

	IBK::IBK_Message("\n", IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
	IBK::IBK_Message("Creating Integrator\n", IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
	IBK::MessageIndentor indent; (void)indent;

	if (m_project.m_solverParameter.m_integrator == NANDRAD::SolverParameter::I_EXPLICIT_EULER) {
		IBK::IBK_Message("Using Explict Euler integrator.\n", IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
		SOLFRA::IntegratorExplicitEuler * integrator = new SOLFRA::IntegratorExplicitEuler();
		integrator->m_dt = dt0();
		m_integrator = integrator;
	}
	else if (m_project.m_solverParameter.m_integrator == NANDRAD::SolverParameter::I_IMPLICIT_EULER) {
		IBK::IBK_Message("Using Implicit Euler integrator.\n", IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
		SOLFRA::IntegratorImplicitEuler * integrator = new SOLFRA::IntegratorImplicitEuler();
		// set parameters given by Solverparameter section
		integrator->m_absTol = m_project.m_solverParameter.m_para[NANDRAD::SolverParameter::SP_ABSTOL].value;
		integrator->m_relTol = m_project.m_solverParameter.m_para[NANDRAD::SolverParameter::SP_RELTOL].value;
		integrator->m_nonLinConvCoeff = m_project.m_solverParameter.m_para[NANDRAD::SolverParameter::SP_NONLINSOLVERCONVCOEFF].value;
		integrator->m_maximumNonlinearIterations = (unsigned int) m_project.m_solverParameter.m_para[NANDRAD::SolverParameter::SP_MAX_NONLIN_ITER].value;
		/// \todo Specify ImplicitEuler parameters

		m_integrator = integrator;
	}
	else if (m_project.m_solverParameter.m_integrator == NANDRAD::SolverParameter::I_CVODE ||
		m_project.m_solverParameter.m_integrator == NANDRAD::SolverParameter::NUM_I)
	{
		IBK::IBK_Message("Using CVODE integrator.\n", IBK::MSG_PROGRESS, FUNC_ID, 1);
		SOLFRA::IntegratorSundialsCVODE * integrator = new SOLFRA::IntegratorSundialsCVODE();
		// set parameters given by Solverparameter section
		integrator->m_absTol = m_project.m_solverParameter.m_para[NANDRAD::SolverParameter::SP_ABSTOL].value;
		integrator->m_relTol = m_project.m_solverParameter.m_para[NANDRAD::SolverParameter::SP_RELTOL].value;
		integrator->m_dtMax = m_project.m_solverParameter.m_para[NANDRAD::SolverParameter::SP_MAX_DT].value;
		integrator->m_dtMin = m_project.m_solverParameter.m_para[NANDRAD::SolverParameter::SP_MIN_DT].value;
		integrator->m_maxOrder = (unsigned int)m_project.m_solverParameter.m_para[NANDRAD::SolverParameter::SP_MAX_ORDER].value;
		integrator->m_maxSteps = 100000000; // extremely large value
		integrator->m_nonLinConvCoeff = m_project.m_solverParameter.m_para[NANDRAD::SolverParameter::SP_NONLINSOLVERCONVCOEFF].value;
		integrator->m_maxNonLinIters = (unsigned int) m_project.m_solverParameter.m_para[NANDRAD::SolverParameter::SP_MAX_NONLIN_ITER].value;

		m_integrator = integrator;
	}
#endif
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
	return 3600;
#if 0
	// loop over all defined output grids and search for next scheduled output
	double tOutNext = std::numeric_limits<double>::max(); // largest possible value

	// get time including start offset
	double tWithStartOffset = t + m_project.m_simulationParameter.m_interval.m_para[NANDRAD::Interval::IP_START].value;

	for (std::vector<NANDRAD::OutputGrid>::const_iterator it = m_project.m_outputs.m_grids.begin();
		 it != m_project.m_outputs.m_grids.end(); ++it)
	{
		tOutNext = std::min(tOutNext, it->computeNextOutputTime(tWithStartOffset));
	}

	// convert tOutNext back to simulation time
	return tOutNext - m_project.m_simulationParameter.m_interval.m_para[NANDRAD::Interval::IP_START].value;
#endif
}



/*** Public Static Functions ***/

void NandradModel::printVersionStrings() {
	FUNCID(ModelImpl::printVersionStrings);

	// print compiler and version information
	IBK::Version::printCompilerVersion();
	IBK::IBK_Message("\n", IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
	IBK::IBK_Message("NANDRAD version                                  " + std::string(NANDRAD::LONG_VERSION) + "\n", IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
	IBK::IBK_Message("IBK library version                              " + std::string(IBK::VERSION) + "\n", IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
	IBK::IBK_Message("DATAIO library version                           " + std::string(DATAIO::LONG_VERSION) + "\n", IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
	IBK::IBK_Message("CCM library version                              " + std::string(CCM::LONG_VERSION) + "\n", IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
}


} // namespace NANDRAD_MODEL


