#ifndef NM_OutputFileH
#define NM_OutputFileH

#include <string>
#include <vector>

#include <NANDRAD_OutputDefinition.h>
#include "NM_AbstractModel.h"
#include "NM_AbstractStateDependency.h"
#include "NM_AbstractTimeDependency.h"

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

	// *** Re-implemented from AbstractModel

	/*! Not needed, output files do not generate results. */
	virtual NANDRAD::ModelInputReference::referenceType_t referenceType() const override {
		return NANDRAD::ModelInputReference::MRT_GLOBAL;
	}

	/*! Return unique class ID name of implemented model. */
	virtual const char * ModelIDName() const override { return "OutputFile"; }

	/*! Not implemented, since not needed. */
	virtual unsigned int id() const override { return 0; }

	/*! Not implemented, since not needed. */
	virtual const char * displayName() const override { return "OutputFile"; }

	/*! Not implemented, since not needed. */
	virtual void resultDescriptions(std::vector<QuantityDescription> & /*resDesc*/) const override {}

	/*! Not implemented, since not needed. */
	virtual const double * resultValueRef(const QuantityName & /*quantityName*/) const override { return nullptr; }

	/*! Not implemented, since not needed. */
	virtual void resultValueRefs(std::vector<const double *> & /*res*/) const override {}

	/*! Not implemented, since not needed. */
	virtual void initResults(const std::vector<AbstractModel*> & /* models */) override {}



	// *** Re-implemented from AbstractTimeDependency

	/*! Not implemented, since not needed. */
	virtual int setTime(double /*t*/) override {}

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
	virtual void setInputValueRef(const InputReference &inputRef, const double *resultValueRef) override;

	/*! We have nothing to do here, output handling is done outside the actual evaluation. */
	virtual int update() override { return 0; }


	/*! In this function the output definitions are expanded into scalar variables and corresponding
		input references.
	*/
	void createInputReferences();

private:

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

	/*! Pointers to variables to monitor. */
	std::vector<const double*>					m_valueRefs;

	friend class OutputHandler;
};

} // namespace NANDRAD_MODEL


#endif // NM_OutputFileH
