/*	Solver Control Framework
	Copyright (C) 2010  Andreas Nicolai <andreas.nicolai -[at]- tu-dresden.de>

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "SOLFRA_SolverControlFramework.h"

#include <fstream>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <ctime>
#include <vector>
#include <string>
#include <stdexcept>
#include <cstring>
#include <memory>

#include <IBK_messages.h>
#include <IBK_FormatString.h>
#include <IBK_Exception.h>
#include <IBK_Time.h>
#include <IBK_FileUtils.h>

#ifdef _WIN32
#include <Windows.h>
#endif // _WIN32

#include "SOLFRA_ModelInterface.h"
#include "SOLFRA_IntegratorADI.h"
#include "SOLFRA_IntegratorExplicitEuler.h"
#include "SOLFRA_IntegratorInterface.h"
#include "SOLFRA_OutputScheduler.h"
#include "SOLFRA_IntegratorSundialsCVODE.h"
#include "SOLFRA_IntegratorRungeKutta45.h"
#include "SOLFRA_LESDense.h"
#include "SOLFRA_PrecondInterface.h"
#include "SOLFRA_JacobianInterface.h"
#include "SOLFRA_Constants.h"

#include <sundials/sundials_config.h>
#include <sundials/sundials_timer.h>

namespace SOLFRA {


std::string format_time_difference(double delta_t) {

	std::stringstream strm;
	strm << std::fixed << std::setprecision(3) << std::setw(6) << std::right;
	if (delta_t<60)
		strm << delta_t << " s";
	else if (delta_t<3600)
		strm << delta_t/60.0 << " min";
	else if (delta_t<86400)
		strm << delta_t/3600.0 << " h";
	else if (delta_t<86400*365.0)
		strm << delta_t/86400.0 << " d";
	else
		strm << delta_t/(86400.0*365) << " a";
	return strm.str();
}



SolverControlFramework::SolverControlFramework(ModelInterface * model) :
	m_restartMode(RestartFromLast),
	m_stopAfterSolverInit(false),
	m_useStepStatistics(false),
	m_model(model),
	m_integrator(nullptr),
	m_outputScheduler(nullptr),
	m_defaultIntegrator(nullptr),
	m_defaultLES(nullptr),
	m_defaultOutputScheduler(nullptr)
{
	if (model != nullptr)
		setModel(model); // this will extract integrator, preconditioner, output scheduler from model
}


SolverControlFramework::~SolverControlFramework() {
	delete m_defaultIntegrator;
	delete m_defaultLES;
	delete m_defaultOutputScheduler;
}


void SolverControlFramework::setModel(ModelInterface * model) {
	FUNCID(SolverControlFramework::setModel);

	// clean up previously created
	delete m_defaultIntegrator;
	m_defaultIntegrator = nullptr;
	delete m_defaultLES;
	m_defaultLES = nullptr;
	delete m_defaultOutputScheduler;
	m_defaultOutputScheduler = nullptr;

	// store pointer to model
	m_model = model;

	// store pointer to integrator
	m_integrator = model->integratorInterface();
	if (m_integrator == nullptr) {
		// construct integrator when model doesn't specify one
		m_defaultIntegrator = new IntegratorSundialsCVODE;
		m_integrator = m_defaultIntegrator;
	}

	// store pointer to linear equation system solver
	m_lesSolver = model->lesInterface();

	// ADI integrator requires ADI LES interface
	IntegratorADI *integratorADI = dynamic_cast<IntegratorADI*>(m_integrator);
	if (integratorADI != nullptr) {
		m_lesSolver = integratorADI->lesInterface();
	}


	// the explicit integrators do not need an LES solver
	IntegratorRungeKutta45 * integratorRK = dynamic_cast<IntegratorRungeKutta45*>(m_integrator);
	IntegratorExplicitEuler * integratorExpEuler = dynamic_cast<IntegratorExplicitEuler*>(m_integrator);

	if (integratorRK != nullptr || integratorExpEuler != nullptr) {
		if (m_lesSolver != nullptr)
			throw IBK::Exception("Providing LES solvers for explicit integrators is not needed and hence treated as an error.", FUNC_ID);
		IBK_Message( IBK::FormatString("Explicit intergrators to not require LES solvers.\n"), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_INFO);
	}
	else if (m_lesSolver == nullptr) {
		// if none is given, construct a dense direct matrix solver
		IBK_Message( IBK::FormatString("Creating generic Dense LES solver.\n"), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_INFO);
		m_defaultLES = new LESDense;
		m_lesSolver = m_defaultLES;
	}

	// store pointer to preconditioner
	m_precondInterface = model->preconditionerInterface();

	// store pointer to jacobian
	m_jacobianInterface = model->jacobianInterface();

	// store pointer to output scheduler
	m_outputScheduler = model->outputScheduler();
	if (m_outputScheduler == nullptr) {
		// if none is given, construct default scheduler
		m_defaultOutputScheduler = new OutputScheduler;
		m_outputScheduler = m_defaultOutputScheduler;
	}

}


void SolverControlFramework::setIntegrator(IntegratorInterface * integrator) {
	m_integrator = integrator;
}


void SolverControlFramework::setLESSolver(LESInterface	* lesSolver) {
	m_lesSolver = lesSolver;
}


void SolverControlFramework::setPreconditioner(PrecondInterface	* precondInterface) {
	m_precondInterface = precondInterface;
}


void SolverControlFramework::setJacobian(JacobianInterface	* jacobianInterface) {
	m_jacobianInterface = jacobianInterface;
}


void SolverControlFramework::setOutputScheduler(OutputScheduler	* outputScheduler) {
	m_outputScheduler = outputScheduler;
}


void SolverControlFramework::setRestartFile(const std::string & fname, RestartFileMode restartMode) {
	m_restartFilename = fname;
	m_restartMode = restartMode;
}


void SolverControlFramework::restartFrom(double t_restart) {
	FUNCID(SolverControlFramework::restartFrom=);
	if (m_model == nullptr || m_integrator == nullptr || m_outputScheduler == nullptr)
		throw IBK::Exception("Invalid pointers to model, integrator or outputScheduler.", FUNC_ID);
	std::vector<double> tmp(m_model->n() + m_model->serializationSize()/sizeof(double));
	double t;
	bool success = readRestartFile(-2, t_restart, t, &tmp[0]);
	if (!success)
		throw IBK::Exception("Reading of restart file failed.", FUNC_ID);
	// initialize integrator with read solution
	try {
		m_integrator->init(m_model, t, &tmp[0], m_lesSolver, m_precondInterface, m_jacobianInterface);
	}
	catch (IBK::Exception & ex) {
		throw IBK::Exception(ex, "Initialization of Integrator failed.", FUNC_ID);
	}
	if (m_stopAfterSolverInit) {
		IBK::IBK_Message("Stopping after successful initialization of integrator.\n", IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
		return;
	}

	// now pass serialization data to model as well
	double * modelDataPtr = tmp.data() + m_model->n();
	void * modPtr = (void*)modelDataPtr;
	// check if enough data is available - i.e. source data memory is large enough for target memory in model
	unsigned int dataSize = m_model->serializationSize()/sizeof(double); // number of doubles
	if (tmp.size() - m_model->n() != dataSize)
		throw IBK::Exception("Invalid data size in restart file. You MUST NOT add/remove "
							 "outputs with time integration/averaging when restarting!", FUNC_ID);
	m_model->deserialize(modPtr);


	run(t);
}


void SolverControlFramework::restart(int step) {
	FUNCID(SolverControlFramework::restart);
	if (m_model == nullptr || m_integrator == nullptr || m_outputScheduler == nullptr)
		throw IBK::Exception("Missing model, integrator or outputScheduler.", FUNC_ID);

	if (step == 0) {
		// initialize integrator with initial conditions from model
		try {
			m_integrator->init(m_model, m_model->t0(), m_model->y0(), m_lesSolver, m_precondInterface, m_jacobianInterface);
		}
		catch (IBK::Exception & ex) {
			throw IBK::Exception(ex, "Initialization of Integrator failed.", FUNC_ID);
		}
		if (m_stopAfterSolverInit) {
			IBK::IBK_Message("Stopping after successful initialization of integrator.\n", IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
			return;
		}

		double t = m_model->t0();
		// clear restart file
#if defined(_MSC_VER)
		std::ofstream out(m_restartFilename.wstr().c_str(), std::ios_base::binary);
#else // _WIN32
		std::ofstream out(m_restartFilename.str().c_str(), std::ios_base::binary);
#endif // _WIN32
		out.close();
		run(t);
		// we are allready finished here, so we will return
		return;
	}
	std::vector<double> tmp(m_model->n() + m_model->serializationSize()/sizeof(double));
	double t;
	// create restart file to hold all previous restart data up to and including 'step'
	IBK::Path tmpfile_name = m_restartFilename;
	tmpfile_name.addExtension("tmp");
#if defined(_MSC_VER)
	std::ofstream tmp_restartfile(tmpfile_name.wstr().c_str(), std::ios_base::binary);
#else
	std::ofstream tmp_restartfile(tmpfile_name.str().c_str(), std::ios_base::binary);
#endif
	bool success = readRestartFile(step, 0, t, &tmp[0], &tmp_restartfile);
	tmp_restartfile.close();
	if (!success)
		throw IBK::Exception("Error reading restart data.", FUNC_ID);
	// replace restart files

	// Note: in case of RestartFromAll, we had stored many restart time data sets, for example for 30 time points
	//       If we now restart from the 10th point, we copy in function readRestartFile() the first 10 restart points
	//       to a temporary file. And now we copy the tempary file over the original restart file, so that it appears
	//       is if we had continued from the end. This ensures, that the next added restart time data sets are always
	//       consecutively following the old restart points.
	bool copy_success;
	{
#if defined(_MSC_VER)
		std::ofstream out(m_restartFilename.wstr().c_str(), std::ios_base::binary);
		std::ifstream in(tmpfile_name.wstr().c_str(), std::ios_base::binary);
#else
		std::ofstream out(m_restartFilename.str().c_str(), std::ios_base::binary);
		std::ifstream in(tmpfile_name.str().c_str(), std::ios_base::binary);
#endif
		out << in.rdbuf();
		copy_success = out.good();
	}
	if (!copy_success) {
		throw IBK::Exception( IBK::FormatString("Couldn't copy temporary file '%1' to '%2'.")
							  .arg(tmpfile_name).arg(m_restartFilename), FUNC_ID);
	}

	// initialize integrator with read solution
	try {
		// the first model->n() doubles in the tmp vector are the states used by the integrator
		m_integrator->init(m_model, t, &tmp[0], m_lesSolver, m_precondInterface, m_jacobianInterface);
	}
	catch (IBK::Exception & ex) {
		throw IBK::Exception(ex, "Initialization of Integrator failed.", FUNC_ID);
	}

	// now pass serialization data to model as well
	double * modelDataPtr = tmp.data() + m_model->n();
	void * modPtr = (void*)modelDataPtr;
	// check if enough data is available - i.e. source data memory is large enough for target memory in model
	unsigned int dataSize = m_model->serializationSize()/sizeof(double); // number of doubles
	if (tmp.size() - m_model->n() != dataSize)
		throw IBK::Exception("Invalid data size in restart file. You MUST NOT add/remove "
							 "outputs with time integration/averaging when restarting!", FUNC_ID);
	m_model->deserialize(modPtr);

	if (m_stopAfterSolverInit) {
		IBK::IBK_Message("Stopping after successful initialization of integrator.\n", IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
		return;
	}
	run(t);
}


void SolverControlFramework::printRestartFileInfo(const IBK::Path & restartFilePath) {
	FUNCID(SolverControlFramework::printRestartFileInfo);

#if defined(_MSC_VER)
	std::ifstream in(restartFilePath.wstr().c_str(), std::ios_base::binary);
#else //_WIN32
	std::ifstream in(restartFilePath.str().c_str(), std::ios_base::binary);
#endif // _WIN32
	if (!in) {
		IBK::IBK_Message( IBK::FormatString("Cannot open restart file '%1' for writing.").arg(restartFilePath), IBK::MSG_ERROR);
		return;
	}

	// read in binary mode unless debug option is set
	double t;
	unsigned int n;
	in.read((char *)&t, sizeof(double));
	in.read((char *)&n, sizeof(unsigned int) );
	if (!in) {
		IBK::IBK_Message( IBK::FormatString("Failed to read first block in restart file."), IBK::MSG_ERROR);
		return;
	}

	// compute block size
	unsigned int bs = sizeof(double) + sizeof(unsigned int) + n*sizeof(double);

	// now loop over all blocks
	IBK::IBK_Message( IBK::FormatString("Time points stored in restart file:\n"), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
	unsigned int block = 0;
	while (in.seekg(block*bs, std::ios_base::beg)) {
		in.read((char *)&t, sizeof(double));
		if (!in)
			break;
		std::stringstream strm;
		strm << std::setw(10) << std::right << format_time_difference(t) << "\n";
		IBK::IBK_Message( strm.str(), IBK::MSG_PROGRESS);
		++block;
	}

}


// *** STATIC FUNCTIONS ***

void SolverControlFramework::printVersionInfo() {
	FUNCID(SolverControlFramework::printVersionInfo);
	IBK::IBK_Message( IBK::FormatString("Integrator Framework Version                     %1\n").arg(LONG_VERSION), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD );
	IBK::IBK_Message( IBK::FormatString("SUNDIALS Release Version                         %1\n").arg(SUNDIALS_PACKAGE_VERSION), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD );
}

// *** PRIVATE FUNCTIONS ***

void SolverControlFramework::run(double t) {

	FUNCID(SolverControlFramework::run);
	if (m_model == nullptr || m_integrator == nullptr || m_outputScheduler == nullptr)
		throw IBK::Exception("Missing model, integrator or outputScheduler.", FUNC_ID);

	IBK::IBK_Message( "\n", IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD );
	std::string dateTimeStr = m_model->simTime2DateTimeString(t);
	IBK::IBK_Message( IBK::FormatString("Running solver from time t=%1 (%2)\n").arg(t).arg(dateTimeStr), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD );

	double t_end = m_model->tEnd();
	double t_out = m_outputScheduler->nextOutputTime(t);

	m_stopWatch.start();

	try {

		// write initial output, only if we start from begin
		bool restarting =  (t != m_model->t0());

		/// \todo Check if initial call to stepCompleted is really necessary

		// stepCompleted() is called with the current combination of t, y and ydot as
		// they are stored in the integrator object
		const double * y_initial = m_integrator->yOut( m_integrator->t() );
		m_model->stepCompleted(m_integrator->t(), y_initial);

		// outputs are only written if we start the simulation from begin, therefore
		// we pass t0 and y0 for the initial model evaluation within writeOutputs()
		if (!restarting) {
			m_model->writeOutputs( m_model->t0(), m_model->y0());
		}

		// write statistics files
		m_integrator->writeStatisticsHeader(m_logDirectory, restarting);
		if (m_lesSolver != nullptr)
			m_lesSolver->writeStatisticsHeader(m_logDirectory, restarting);
		if (m_precondInterface != nullptr)
			m_precondInterface->writeStatisticsHeader(m_logDirectory, restarting);

		bool hadRestartAtFinalOutput = false; // this flag is set to true, when the final restart file had been written
											  // as part of the output writing loop; if true, the final writing of restart
											  // file at end of simulation (at t_end) will be supressed.

		// integration loop
		while (t < t_end) {

			// (contains parallel code)
			IntegratorInterface::StepResultType res = m_integrator->step();
			if (res != IntegratorInterface::StepSuccess)
				throw IBK::Exception("Error during integrator step call.", FUNC_ID);

			// get new time point and time step size
			t = m_integrator->t();

			// notify model of completed step (contains parallel code)
			const double * y_current = m_integrator->yOut( t );

			// WARNING THIS CODE IS BARRIER RELEVANT, especially when AIR is computed
			SUNDIALS_TIMED_FUNCTION( SUNDIALS_TIMER_STEP_COMPLETED,
				m_model->stepCompleted(t, y_current);
			);

			// write all outputs scheduled within last step
			bool had_output = false;
			// The define OUTPUT_AFTER_EACH_STEP must only be set in either
			// the Delphin6Session.pro file (for debug builds),
			// or in the global /build/cmake/CMakeLists.txt file!
#ifdef OUTPUT_AFTER_EACH_STEP
			// tell m_model to write outputs
			SUNDIALS_TIMED_FUNCTION( SUNDIALS_TIMER_WRITE_OUTPUTS,
				m_model->writeOutputs( t, y_current)
			);
#else
			// Note: in the check below we add a small value to ensure that for
			//       t_out = 2.000000001 and t = 2.0 we still get an output at simulation end
			while (t_out <= t + 1e-8) {

				// retrieve interpolated states at output time point (contains parallel code)
				const double * y_out = m_integrator->yOut(t_out);
				double t_outNext = 0.0;

				// tell m_model to write outputs
				SUNDIALS_TIMED_FUNCTION( SUNDIALS_TIMER_WRITE_OUTPUTS,
					m_model->writeOutputs( t_out, y_out);
				);

				// ensure that we write the final restart info _exactly_ at end of the simulation
				// we add a small time difference to ensure that we are actually at end of simulation
				if (t_out + 1e-8 > t_end) {
					appendRestartInfo(t_end, y_out);
					hadRestartAtFinalOutput = true;
				}

				writeProgress(t_out, true); // only progress output of integrator framework itself, currently disabled since models provide own progress reporting
				had_output = true;

				// retrieve new output time point
				t_outNext = m_outputScheduler->nextOutputTime(t_out);
				if (t_outNext <= t_out)
					throw IBK::Exception(IBK::FormatString("Output scheduler returned output time %1, which is <= last output time %2.")
						.arg(t_outNext).arg(t_out), FUNC_ID);

				// store next output time point
				t_out = t_outNext;

				// If t_outNext is > t_end (including some rounding error compensation), we break the loop
				// to avoid writing outputs beyond simulation end.
				// The fuzzy extension of simulation end is needed because t_out could be computed for example
				// with t_out = 300.000001 and simulation end could be t_end = 300.
				if (t_outNext > t_end + 1e-8)
					break;

			} // while (t_out <= t)
#endif

			// write statistics if we had an output, or if we had step-by-step statistics enabled
			if (had_output || m_useStepStatistics) {
				m_integrator->writeStatistics();
				if (m_lesSolver != nullptr)
					m_lesSolver->writeStatistics(t);
				if (m_precondInterface != nullptr)
					m_precondInterface->writeStatistics(t);
			}

			// write restart info after completed step
			writeProgress(t, false);

			// Check if we have have written outputs (in which case we also must write restart info).
			// However, we do not want to write restart info at end twice, so we check for hadRestartAtFinalOutput also
			if (!hadRestartAtFinalOutput && (had_output || t >= t_end)) {
				if (t >= t_end) {
					const double * y_out = m_integrator->yOut(t_end);
					appendRestartInfo(t_end, y_out);
				}
				else {
					// if user only wants restart info at end of simulation (i.e. WRM optimization scenario)
					// and the flag RestartInfoOnlyAtSimulationEnd has been set via command line, do not write
					// restart info
					if (m_restartMode != RestartInfoOnlyAtSimulationEnd)
						appendRestartInfo(t, y_current);
				}
			}

		} // while (t < t_end)

		writeProgress(t, false);

		// Ask model to write out any last outputs to files. Most models won't need this, but if value
		// caching is implemented, this ensures that there is a mechanism to flush the cache to files.
		SUNDIALS_TIMED_FUNCTION( SUNDIALS_TIMER_WRITE_OUTPUTS,
			m_model->writeFinalOutputs();
		);
		m_stopWatch.stop();

	}
	catch (IBK::Exception & ex) {
		throw IBK::Exception(ex, "Error during integration.", FUNC_ID);
	}
	catch (std::exception & ex) {
		throw IBK::Exception( IBK::FormatString("%1\nError during integration.").arg(ex.what()), FUNC_ID);
	}
}


void SolverControlFramework::appendRestartInfo(double t, const double * y) const {
	FUNCID(SolverControlFramework::appendRestartInfo);

	// do nothing if no filename is set
	if (m_restartFilename.str().empty()) return;

	// if restart file exists, rename it to bak, but only once every 10 minutes - this is
	// for really long simulations, where we accept potentially duplicate output steps
	// in output files (when we restart to a time point past that we had already several
	// outputs written), in order to save a lot of simulation time
	if (m_restartFileRenameWatch.difference() > 600*1000) {
		if (m_restartFilename.isFile()) {
			IBK::Path::move(m_restartFilename, m_restartFilename + ".bak"); // restart.bak
			const_cast<SolverControlFramework*>(this)->m_restartFileRenameWatch.start(); // reset timer
		}
	}

	// try to open file for writing
	std::ofstream out;
	if (m_restartMode == RestartFromAll) {
		// append to existing file
#if defined(_MSC_VER)
		out.open(m_restartFilename.wstr().c_str(), std::ios_base::app | std::ios_base::binary);
#else
		out.open(m_restartFilename.c_str(), std::ios_base::app | std::ios_base::binary);
#endif
	}
	else {
		// create new file
#if defined(_MSC_VER)
		out.open(m_restartFilename.wstr().c_str(), std::ios_base::binary);
#else
		out.open(m_restartFilename.c_str(), std::ios_base::binary);
#endif
	}
	if (!out)
		throw IBK::Exception( IBK::FormatString("Cannot open restart file '%1' for writing.").arg(m_restartFilename), FUNC_ID);
	// write in binary mode unless debug option is set
	out.write((const char *)&t, sizeof(double) );

	// collect and write total number of doubles in double-value data block
	unsigned int n = m_model->n();
	unsigned int dataSize = m_model->serializationSize() / sizeof(double);
	unsigned int totalNumberOfDoubles = n + dataSize;
	out.write((const char *)&totalNumberOfDoubles, sizeof(unsigned int) );

	// write integrator memory
	out.write((const char *)y, sizeof(double)*n);

	// write model memory
	if (dataSize != 0) {
		std::vector<double> tmpData(dataSize);
		void * dataPtr = (void *)tmpData.data();
		m_model->serialize(dataPtr);
		out.write((const char *)tmpData.data(), sizeof(double)*dataSize);
	}

}



bool SolverControlFramework::readRestartFile(int step,
	double t_restart,
	double & t, double * integratorModelData,
	std::ostream * restartFileCopy) const
{
	// open restart file for reading
#if defined(_MSC_VER)
	std::ifstream in(m_restartFilename.wstr().c_str(), std::ios_base::binary);
#else // _WIN32
	std::ifstream in(m_restartFilename.str().c_str(), std::ios_base::binary);
#endif // _WIN32
	if (!in) {
		IBK::IBK_Message( IBK::FormatString("Cannot open restart file '%1' for reading.").arg(m_restartFilename), IBK::MSG_ERROR);
		return false;
	}
	// read in binary mode unless debug option is set
	unsigned int n;
	in.read((char *)&t, sizeof(double));
	in.read((char *)&n, sizeof(unsigned int) );
	if (!in) {
		IBK::IBK_Message( IBK::FormatString("Failed to read first block in restart file."), IBK::MSG_ERROR);
		return false;
	}
	std::vector<double> tmp(n);
	in.read((char *)&tmp[0], sizeof(double)*n);
	if (!in) {
		IBK::IBK_Message( IBK::FormatString("Failed to read first block in restart file."), IBK::MSG_ERROR);
		return false;
	}
	unsigned int numberOfDoubles = m_model->n() + m_model->serializationSize()/sizeof(double);
	if (n != numberOfDoubles) {
		IBK::IBK_Message( IBK::FormatString("Size mismatch between restart file (n = %1) and "
			"size returned by model (n = %2)").arg(n).arg(m_model->n()), IBK::MSG_ERROR);
		return false;
	}

	// compute block size (must be int variable)
	long bs = sizeof(double) + sizeof(unsigned int) + n*sizeof(double);
	// if step == -1, read last solution
	if (step == -1) {
		in.seekg(-bs, std::ios_base::end);
		if (!in)
			IBK::IBK_Message( IBK::FormatString("Error seeking."), IBK::MSG_ERROR);
		in.read((char *)&t, sizeof(double));
		in.read((char *)&n, sizeof(unsigned int) );
		in.read((char *)integratorModelData, sizeof(double)*n);
		if (!in) {
			IBK::IBK_Message( IBK::FormatString("Error reading last block in restart file."), IBK::MSG_ERROR);
			return false;
		}
		if (restartFileCopy != nullptr) {
			// since we continue from last step, we now need to copy the whole file into our temporary file
			in.seekg(0, std::ios_base::beg);
			*restartFileCopy << in.rdbuf();
		}
		return true;
	}

	// now loop over all blocks
	int block = 0;
	double last_successful_t = t;
	while (in.seekg(block*bs, std::ios_base::beg)) {
		++block;
		if (block == step)
			return true;		// abort criteria #1: stop after successful step # step
		last_successful_t = t;
		// read next time point
		in.read((char *)&t, sizeof(double));
		// special case: step == -2, we use t_restart
		if (step == -2 && t > t_restart - 1e-10) {
			// use values still stored in last_successful_t and tmp
			std::memcpy(integratorModelData, &tmp[0], sizeof(double)*n);
			t = last_successful_t;
			return true;		// abort criteria #2: passed restart time point
		}
		in.read((char *)&n, sizeof(unsigned int) );
		in.read((char *)&tmp[0], sizeof(double)*n);
		if (!in) {
			IBK::IBK_Message( IBK::FormatString("Error reading block after time t = %1 in restart file.").arg(last_successful_t), IBK::MSG_ERROR);
			return false;
		}
		// write previously read data to temporary file, if given
		if (restartFileCopy != nullptr) {
			restartFileCopy ->write((const char *)&t, sizeof(double) );
			restartFileCopy ->write((const char *)&n, sizeof(unsigned int) );
			restartFileCopy ->write((const char *)&tmp[0], sizeof(double)*n);
		}
	}
	// if block == step we have found the current step
	if (block == step)
		return true;

	IBK::IBK_Message( IBK::FormatString("Restart file has only %1 steps stored, so we cannot continue from step #%2").arg(block).arg(step), IBK::MSG_ERROR);
	return false;
}


void SolverControlFramework::writeMetrics() {
	FUNCID(SolverControlFramework::writeStatistics);
	// First write overall simulation time, then timings obtained by integrator, les-solver, jacobian interface and pre-conditioner.
	IBK::IBK_Message( IBK::FormatString("\nSolver statistics\n"), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
	IBK::IBK_Message( IBK::FormatString("------------------------------------------------------------------------------\n"), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
	double wct = m_stopWatch.difference()*1e-3; // in seconds
	// determine suitable unit
	std::string ustr = IBK::Time::suitableTimeUnit(wct);
	IBK::IBK_Message( IBK::FormatString("Wall clock time                            = %1\n").arg(IBK::Time::format_time_difference(wct, ustr, true),13),
		IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
	IBK::IBK_Message( IBK::FormatString("------------------------------------------------------------------------------\n"), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);

#if __cplusplus > 199711L
	std::unique_ptr<std::ofstream> of_ptr;
#else
	std::auto_ptr<std::ofstream> of_ptr;
#endif
	std::ofstream * of = nullptr;
	if (m_logDirectory.isValid()) {
		of_ptr.reset( IBK::create_ofstream(m_logDirectory / "summary.txt") );
		of = of_ptr.get();
		*of << "WallClockTime=" << wct << std::endl;
	}

	double toutputeval = TimerSum(SUNDIALS_TIMER_WRITE_OUTPUTS);
	IBK::IBK_Message( IBK::FormatString("Framework: Output writing                  = %1 (%2 %%)    \n")
		.arg(IBK::Time::format_time_difference(toutputeval, ustr, true),13)
		.arg(toutputeval/wct*100, 5, 'f', 2),
//		.arg(m_statNumRHSEvals,8),
		IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
	double tstepcompletedeval = TimerSum(SUNDIALS_TIMER_STEP_COMPLETED);
	IBK::IBK_Message( IBK::FormatString("Framework: Step-completed calculations     = %1 (%2 %%)    \n")
		.arg(IBK::Time::format_time_difference(tstepcompletedeval, ustr, true),13)
		.arg(tstepcompletedeval/wct*100, 5, 'f', 2),
//		.arg(m_statNumRHSEvals,8),
		IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
	if (of != nullptr) {
		*of << "FrameworkTimeWriteOutputs=" << toutputeval << std::endl;
		*of << "FrameworkTimeStepCompleted=" << tstepcompletedeval << std::endl;
	}
	m_integrator->writeMetrics(wct, of);

	if (m_lesSolver != nullptr)
		m_lesSolver->writeMetrics(wct, of);

	m_model->writeMetrics(wct, of);

	IBK::IBK_Message( IBK::FormatString("------------------------------------------------------------------------------\n"), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
}


} // namespace SOLFRA
