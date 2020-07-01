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

#ifndef NandradModelH
#define NandradModelH

#include <string>
#include <vector>

#include <SOLFRA_ModelInterface.h>
#include <SOLFRA_IntegratorInterface.h>
#include <SOLFRA_LESInterface.h>

#include <NANDRAD_ArgsParser.h>

#include "NM_Directories.h"

#define MODIFY_WEIGHT_FACTORS

namespace NANDRAD {
	class Project;
}

namespace SOLFRA {
	class ModelInterface;
	class FMUModelInterface;
}

namespace NANDRAD_MODEL {

class NandradModelImpl;
class FMU2ExportModel;
class FMU2ImportModel;
class OutputFile;

/*! A model implementation of the multi-zone model Nandrad.
	This class implements the ModelInterface and can be integrated using the
	integrator framework.
	The actual implementation is done in the implementation class NandradModelImpl
	which hides the details of the implementation and all dependencies from users
	of the model.
*/
class NandradModel : public SOLFRA::ModelInterface {
public:

	typedef unsigned char BYTE;

	/*! Default constructor. */
	NandradModel();
	/*! Destructor. */
	virtual ~NandradModel();

	/*! Initializes model. */
	void init(const NANDRAD::ArgsParser & args);

	/*! Number of unknowns/states, size n. */
	virtual unsigned int n() const;

	/*! Initial condition vector, size n.
		\return Returns a pointer to a linear memory array with the
				initial states.

		\warning The lifetime of the pointer may be limited. Specifically,
				 the pointer may be invalidated in a call to readRestartInfo().
	*/
	virtual const double * y0() const;
	/*! Start time point in [s]. */
	virtual double t0() const;
	/*! Initial time step. */
	virtual double dt0() const;
	/*! End time point in [s]. */
	virtual double tEnd() const;

	/*! Current ime point in [s]. */
	double t() const;
	/*! Current states vector. */
	const double * y() const;

	/*! Gives access to the project file. */
	const NANDRAD::Project & project() const ;

	/*! Update state of model to new time point. */
	virtual CalculationResult setTime(double t);

	/*! Update state of model to new set of unknowns. */
	virtual CalculationResult setY(const double * y);

	/*! Stores the computed derivatives of the solution results in the vector ydot. */
	virtual CalculationResult ydot(double * ydot);

	/*! Writes outputs at time t_out using the states y_out.
		This function is called for ODE-type models.
		The pointer y_out has limited lifetime and should only be used in this
		function and never be kept for later calls.
		This function may alter the state of the model implementation object to match
		the output time and solution.
		Default implementation does nothing.
		\param t_out Output time point.
		\param y_out Pointer to linear memory array of size n() containing the corresponding solution.
		\param ydot_out Pointer to linear memory array of size n() containing the corresponding
			time derivatives (NULL Pointer for ODE-type models).
	*/
	virtual void writeOutputs(double t_out, const double * y_out);

	/*! Function to convert relative simulation time t into a date/time string representation. */
	virtual std::string simTime2DateTimeString(double t) const;

	/*! Informs the model that a step was successfully completed.
		The time point and value vector passed to the function correspond to
		the current state in the integrator object.
		This function can be used to write restart info.
		Default implementation does nothing.
	*/
	virtual void stepCompleted(double t, const double * y);

	/*! Sets up directories so that log files and output files can be written.
		Throws an exception if the project file doesn't exist or the directories
		cannot be created.
		\param args Command-line arguments.
		\param executablePath Full/relative path to solver executable.
		\param projectFile full/relative path to project file.

		The paths are stored in the respective member variables of the solver object.
	*/
	void setupDirectories(const NANDRAD::ArgsParser & args,
		const IBK::Path & projectFile);

	/*! Returns solver/project directories, initialized in init(). */
	const Directories & dirs() const;

	/*! Returns linear equation system solver. */
	virtual SOLFRA::LESInterface * lesInterface();

	/*! Returns model-specific Jacobian matrix generation method for use with iterative LES solvers.
		Default implementation returns NULL (no jacobian generator).
	*/
	virtual SOLFRA::JacobianInterface * jacobianInterface();

	/*! Returns integrator to be used for this model.
		Default implementation returns NULL, which results in CVODE integrator for
		ODE type models and IDA for DAE-type models.
	*/
	virtual SOLFRA::IntegratorInterface * integratorInterface();

	/*! Returns model-specific pre-conditioner.
		Default implementation returns NULL (no preconditioner).
	*/
	virtual SOLFRA::PrecondInterface * preconditionerInterface();

	/*! Returns output scheduler to be used with the framework. */
	virtual SOLFRA::OutputScheduler * outputScheduler();

#ifdef MODIFY_WEIGHT_FACTORS
	/*! Returns error weights to the integrator if defined. Otherwise NULL. */
	virtual CalculationResult calculateErrorWeights(const double *y, double *weights);

	/*! Informs the integrator whether the model owns an error weighting function. */
	virtual bool hasErrorWeightsFunction();
#endif

	/*! Computes and returns serialization size, by default returns 0 which means feature not supported. */
	virtual std::size_t serializationSize() const;

	/*! Stores content at memory location pointed to by dataPtr and increases
		pointer afterwards to point just behind the memory occupied by the copied data.*/
	virtual void serialize(void* & dataPtr) const;

	/*! Restores content from memory at location pointed to by dataPtr and increases
		pointer afterwards to point just behind the memory occupied by the copied data.*/
	virtual void deserialize(void* & dataPtr);

	/*! Writes currently collected solver metrics/statistics to output.
	\param simtime Totel elapsed wall clock time of simulation in [s] (needed for percentage calculation)
	\param metricsFile If not NULL, computer-readible metrics are written to the file.
	*/
	virtual void writeMetrics(double simtime, std::ostream * metricsFile = NULL);

	/*! Prints version strings of used libraries to the IBK message handler. */
	static void printVersionStrings();

	/*! Creates a project file for heating design day calculation
		\param targetFile Filename with path for the original project file.
	*/
	void createHeatingDesignDayCalculationProject() const;

	/*! Creates a project file for cooling design day calculation
		\param targetFile Filename with path for the original project file.
	*/
	void createCoolingDesignDayCalculationProject() const;

	/*! Creates a modelica fmu wrapper for a given FMU file
		\param targetFile Filename with path for the wrapper file.
	*/
	void createModelicaFMUAdapterAndWrapper(const IBK::Path & fmuFile);

	/*! Writes model decription file
		\param targetFile Filename with path for the FMU file.
	*/
	void export2FMU(const IBK::Path & installDir, const IBK::Path & projectFile, const IBK::Path & targetFile,
		const std::string &reportFileExtension = "tsv");

	/*! Writes summary file for a heating design day calculation
	*/
	void writeDesignDaySummary() const;

	/*! Writes report for current FMU
		\param targetDirectory target directory of the report
	*/
	void writeFMUReport(const IBK::Path & targetDirectory, char delimiter) const;


	/*! Compresses all generated FMU files using zlibrary (minizip).
		This function may be substituted by an external function later.
		\param files List of absolute file paths for all files that should be compressed
		\param outputFile Absolute path to zip-file
	*/
	void compressFMUFiles(std::vector<IBK::Path> &files,
							const IBK::Path& outputFile,
							const IBK::Path& rootDir,
							int compressionLevel, bool saveFullPath)  const;

protected:
	/*! Returns reference to the FMUExportModel. */
	FMU2ExportModel * fmu2ExportModel();

	/*! Returns reference to the FMUImportModel. */
	FMU2ImportModel * fmu2ImportModel();

	/*! Returns reference to all output file objects. */
	std::vector<OutputFile*> &outputFiles();

	/*! Pointer to private implementation class. */
	NandradModelImpl		*m_impl;
};

} // namespace NANDRAD_MODEL

#endif // NandradModelH
