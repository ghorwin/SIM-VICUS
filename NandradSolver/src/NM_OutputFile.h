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
*/
class OutputFile : public AbstractModel, public AbstractStateDependency, public AbstractTimeDependency {
public:

	/*! D'tor, released allocated memory. */
	~OutputFile();

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

private:

	// *** Private member functions

	/*! In this function the output definitions are expanded into scalar variables and corresponding
		input references.
	*/
	void createInputReferences();

	/*! Creates/re-opens output file and writes initial values.
		Time-integral values will get 0 entries as initial value.

		\param t_out The time since begin of the start year already converted to the output unit.
		\param restart If true, the existing output file should be appended, rather than re-created
		\param binary If true, files are written in binary mode
		\param timeUnit Output time unit, needed to compose output time header
		\param
	*/
	void createFile(double t_out, bool restart, bool binary, const std::string & timeColumnLabel, const IBK::Path * outputPath);

	/*! Retrieves current output values and appends values to cache.
		This function only caches current output values. The data is written to file in the next call to flushCache().

		\param t_out The time since begin of the start year already converted to the output unit.
	*/
	void cacheOutputs(double t_out);

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
	*/
	std::vector<unsigned int>					m_outputDefMap;

	/*! Pointers to variables to monitor (same size and order as m_inputRefs). */
	std::vector<const double*>					m_valueRefs;
	/*! Corresponding quantity descriptions (same size and order as m_inputRefs). */
	std::vector<QuantityDescription>			m_quantityDescs;

	/*! Number of columns with actual values in the output file.
		Can (remain) 0 if non of the requested variables for this file are available from the model.
		In this case the file is not created and writing outputs does nothing.
	*/
	unsigned int								m_numCols = 0;

	/*! The actual data cache. */
	std::vector< std::vector<double> >			m_cache;

	/*! The integral values (updated in each stepCompleted() call). */
	std::vector<double>							m_integrals;
	/*! The current values (fluxes), updated in each stepCompleted() call. */
	std::vector<double>							m_currentVals;

	/*! Output file stream (owned and initialized in createFile()). */
	std::ofstream								*m_ofstream = nullptr;

	friend class OutputHandler;
};

} // namespace NANDRAD_MODEL


#endif // NM_OutputFileH
