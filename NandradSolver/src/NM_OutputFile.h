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

#ifndef NM_OutputFileH
#define NM_OutputFileH

#include <string>
#include <vector>
#include <iosfwd>

#include <NANDRAD_OutputDefinition.h>

#include "NM_AbstractModel.h"
#include "NM_AbstractStateDependency.h"
#include "NM_AbstractTimeDependency.h"

namespace IBK {
	class Path;
}

namespace NANDRAD_MODEL {

/*! Handles writing of a single tsv output file.
	This class implements the model interface and requests input references for
	all output quantities stored in the output file managed by this class.

	The class also implements the AbstractTimeStateDependency interface, in order to
	receive stepCompleted() calls, needed for time intergration.

	To add OutputFile objects to the model container of NandradModel we need
	to derive from AbstractModel, even though output files never generate results. Hence,
	the implementation of the AbstractModel interface is just dummy code.


	Initialization of OutputFiles is a bit tricky, since not all information is available during setup:

	1. the object is created and mandatory parameters are stored (filename and output definitions)
	2. call to createInputReferences() which composes input references from output definition. One output definition
	   may expand to several input references, depending on object list ID filters. The mapping from input variable
	   vector to original output definition is stored in m_outputDefMap.
	3. the framework calls setInputValueRef(), and provides meta data about requested parameters (like the input/output unit).
	   Only now we have all information to write the file header and retrieve output values.
	   Once the last variable ref has been provided, we initialize the integral values, if integrals/mean values are requested.
	4. the framework calls stepCompleted(), where we start integrating out values.
	5. before the first call to writeOutputs(), the framework calls createFile(), where we create/reopen the file
	6. the framework calls writeOutputs(), where we cache output data
	7. the framework calls flushCache() (after some time) and we dump the collected values to file.
*/
class OutputFile : public AbstractModel, public AbstractStateDependency, public AbstractTimeDependency {
public:

	/*! D'tor, released allocated memory. */
	~OutputFile() override;

	// *** Re-implemented from AbstractModel

	virtual const char * ModelIDName() const override { return "OutputFile"; }


	// *** Re-implemented from AbstractTimeDependency

	/*! Not implemented, since not needed. */
	virtual int setTime(double /*t*/) override { return 0; }

	/*! Informs the model that a step was successfully completed.
		The time point passed to the function correspond to the current state in the integrator object.
		This function can be used to write restart info, or adjust the state of the model discretely
		between integration steps.
		Default implementation does nothing.
		\param t Simulation time in [s].
	*/
	virtual void stepCompleted(double t) override;


	// *** Re-implemented from AbstractStateDependency

	/*! Returns vector with model input references. */
	virtual void inputReferences(std::vector<InputReference>  & inputRefs) const override { inputRefs = m_inputRefs; }

	/*! Returns vector with pointers to memory locations matching input value references. */
	virtual const std::vector<const double *> & inputValueRefs() const override { return m_valueRefs; }

	/*! Not implemented, since already done in init(). */
	virtual void initInputReferences(const std::vector<AbstractModel*> & /* models */) override {}

	/*! Sets a single input value reference (persistent memory location) that refers to the requested input reference.
		\param inputRef An input reference from the previously published list of input references.
		\param resultValueRef Persistent memory location to the variable slot.
	*/
	virtual void setInputValueRef(const InputReference &inputRef, const QuantityDescription & resultDesc, const double *resultValueRef) override;

	/*! We have nothing to do here, output handling is done outside the actual evaluation. */
	virtual int update() override { return 0; }


	// *** Other member functions

	/*! Returns true if output file has at least one OTT_MEAN or OTT_INTEGRAL quantity and requires stepCompleted() calls. */
	bool haveIntegrals() const { return m_haveIntegrals; }

private:

	// *** Private member functions

	/*! In this function the output definitions are expanded into scalar variables and corresponding
		input references.
	*/
	void createInputReferences();

	/*! Creates/re-opens output file.

		\param restart If true, the existing output file should be appended, rather than re-created
		\param binary If true, files are written in binary mode
		\param timeColumnLabel Label of the time column
		\param outputPath Path to output directory.
	*/
	void createFile(bool restart, bool binary, const std::string & timeColumnLabel, const IBK::Path * outputPath);

	/*! Retrieves current output values and appends values to cache.
		This function only caches current output values. The data is written to file in the next call to flushCache().

		\param t_out The time since begin of simulation, for output integral interpolation.
		\param t_timeOfThe time since begin of the start year already converted to the output unit (this goes into the
			time column of the output file).
	*/
	void cacheOutputs(double t_out, double t_timeOfYear);

	/*! Returns number of bytes currently cached in this file object. */
	unsigned int cacheSize() const;

	/*! Called from output handler once sufficient real time has elapsed or amount of data cache exceeds
		defined limit.
	*/
	void flushCache();


	/*! Cached flag to know whether to write in binary or ASCII mode. */
	bool										m_binary;

	/*! The target file name (within output directory). */
	std::string									m_filename;

	/*! All output definitions to be handled within this output file.
		\note An output definition may expand (depending on object list filter) to many variables.
	*/
	std::vector<NANDRAD::OutputDefinition>		m_outputDefinitions;

	/*! Set to true if at least one of the output definitions uses OTT_MEAN or OTT_INTEGRAL.
		The value is initialized in createInputReferences().
		\note It is possible that the requested output quantity is not available. Then, the flag
			is cleared in function setInputValueRef(), when integral values are initialized.
	*/
	bool										m_haveIntegrals = false;

	/*! Pointer to the output grid associated with this output file.
		Outputs are only stored/written, when the output time matches an output time of this grid.
		\note This pointer is just a convenience variable, since each of the output definitions
			holds the same pointer.
	*/
	const NANDRAD::OutputGrid					*m_gridRef;


	/*! Input references for all output variables written in the file handled by this object. */
	std::vector<InputReference>					m_inputRefs;
	/*! Maps output variable (input reference) to output definition (index), this variable was created from.
		\code
		OutputDefinition of = m_outputDefinitions[ m_outputDefMap[inputRefIdx] ];
		\endcode

		This data can be used to obtain the timeTime of the output quantity.
	*/
	std::vector<unsigned int>					m_outputDefMap;

	/*! Pointers to variables to monitor (same size and order as m_inputRefs).
		Populated in setInputValueRef().
	*/
	std::vector<const double*>					m_valueRefs;
	/*! Corresponding quantity descriptions (same size and order as m_inputRefs).
		Populated in setInputValueRef().
	*/
	std::vector<QuantityDescription>			m_quantityDescs;
	/*! Vector with output units; source values are always in base SI unit (same size and order as m_inputRefs).
		Populated in setInputValueRef().
	*/
	std::vector<IBK::Unit>						m_valueUnits;

	/*! Number of columns with actual values in the output file.
		Can (remain) 0 if non of the requested variables for this file are available from the model.
		In this case the file is not created and writing outputs does nothing.
	*/
	unsigned int								m_numCols = 0;

	/*! The actual data cache.
		New values are added in cacheOutputs(). In case of current values (OTT_NONE), the values are retrieved
		from the result value references. In case of integral or mean values (OTT_MEAN and OTT_INTEGRAL), the
		value is computed from the stored integral values.
		Size of inner vector matches m_numCols+1, since time column is also added to cache as first column;
	*/
	std::vector< std::vector<double> >			m_cache;


	/*! Time point (simulation time) in [s] at previous stepCompleted() call (begin of integration interval). */
	double										m_tLast;
	/*! Time point (simulation time) in [s] at current stepCompleted() call (end of integration interval). */
	double										m_tCurrent;
	/*! The integral values (updated in each stepCompleted() call).
		m_integrals[0] holds the values at m_tLast, m_integrals[1] holds the values at m_tCurrent. Size matches m_numCols
		regardless whether the values are integral values or not.
		Integral values are always store in the base SI unit (source unit of the associated value reference) times s.
	*/
	std::vector<double>							m_integrals[2];

	/*! Output file stream (owned and initialized in createFile()). */
	std::ofstream								*m_ofstream = nullptr;

	friend class OutputHandler;
};

} // namespace NANDRAD_MODEL


#endif // NM_OutputFileH
