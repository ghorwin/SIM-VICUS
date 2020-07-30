#ifndef AbstractStateDependencyH
#define AbstractStateDependencyH

#include <ZEPPELIN_DependencyObject.h>

//#include <NANDRAD_ImplicitModelFeedback.h>

#include "NM_AbstractModel.h"
#include "NM_InputReference.h"
#include "NM_QuantityName.h"

#include <map>

namespace NANDRAD_MODEL {

/*!	Class AbstractStateDependency is the base class of all models that
	have input dependencies.

	This class declares the interface required by all state-object type sub-models.
	All state-object models must derive from this interface and must implement the
	pure virtual functions.

	The actual model evaluation code goes into function update(), which is called,
	whenever all objects have been updated, that this model depends on.

	A state-dependent model object needs to inherit both from
	AbstractModel and AbstractStateDependency.
*/
class AbstractStateDependency : public ZEPPELIN::DependencyObject {
public:

	/*! Dummy d'tor. */
	~AbstractStateDependency();

	/*! Returns a priority offset for all models at the end of model evaluation.*/
	static const int priorityOffsetTail = 100000;

	/*! Returns vector with model input references.
		Implicit models must generate their own model input references and populate the
		vector argument.
		\note This function is not the fastest, so never call this function from within the solver
		(except maybe for output writing).
	*/
	virtual void inputReferences(std::vector<InputReference>  & inputRefs) const  = 0;

	/*! Returns vector with pointers to memory locations matching input value references. */
	virtual const std::vector<const double *> & inputValueRefs() const = 0;

	/*! Returns a pattern of direkt computational dependencies between input values
		and result values of the current model (the first pair entry corresponds
		to the result value, the second one to the input value of a dependency). This pattern can be
		compared with a local jacobian pattern. Indeed, instead of indices storage
		adresses of the input values and result values are requested.
		This function is called after all result vectors are resized and the input value
		references are filled with valid pointers.
		\param resultInputValueReferences pattern of results and input values that are directly
		connected by a calculation rule
	*/
	virtual void stateDependencies(std::vector< std::pair<const double *, const double *> > &resultInputValueReferences) const = 0;

	/*! Defines constraints for all result values sorted via value reference:
		< value reference , < minimum value (-inf if undefined), maximum value (inf if undefined)>
		Default implementation return an empty map
	*/
	virtual void constraints(std::map< const double *, std::pair<double, double> > &constraintsPerValueRef) const { constraintsPerValueRef.clear(); }

	/*! Returns a priority number for the ordering in model evaluation: should only be implemented
		for implicit models and is ignored for explicit models.
		Default implementation returns -1 to indicate that ordering is unknown.
	*/
	virtual int priorityOfModelEvaluation() const { return -1; }

	/*! Sets all object dependencies.
		Called when all model results have been initialized (i.e. the function
		initResults() was called in all model objects).

		Re-implement this function and provide all information, so that function
		inputReferences() can be called afterwards (e.g. resize and fill a vector
		of Type std::vector<ModelInputReference>).
	*/
	virtual void initInputReferences(const std::vector<AbstractModel*> & /* models */) = 0;
#if 0
	/*! Called from NandradModelImpl.
		Creates a model input reference from an implicit model feedback.
		Therefore it is requested:
		\para sourceID id of the source model (reference type is always MRT_MODEL)
		\para quantity the quantity name that is referenced from the source model.
		\para targetName the quantity name that is adressed. This name usually
		does not include vector indices.
		\para operation enum indicating whether a new input reference has to be created
		or if we overwrite an existing input reference (i.e. a standart parameter
		model is substituted by a user defined model)
		return true if input reference was registered, false otherwise
	*/
	virtual bool registerInputReference(unsigned int sourceID,
		NANDRAD::ModelInputReference::referenceType_t referenceType,
		const QuantityName &quantity,
		const QuantityName &targetName,
		NANDRAD::ImplicitModelFeedback::operation_t operation) = 0;
#endif

	/*! Sets a single input value reference that refers to the requested
		quantity name. This reference can be filled from NandradModelImpl. The model
		needs to store a vector of InputValues and a mapping information. If the
		routine is called the model searches the corresponding value reference inside
		the vector. Note that the value pointer itself can only be modified by NandradModelImpl
		and is set to nullptr per default.
		If the vector is completely filled the reference values can be obtained by the model
		by calling *inputValueRef(quantity type, (id));
	*/
	virtual void setInputValueRef(const double *resultValueRef, const QuantityName &targetName) = 0;

	/*! This function is called whenever result quantities of other models change.
		Re-implement this function in derived classes and handle all your update-functionality here.
		\return Returns 0 when calculation was successful, 1 when a recoverable error has been detected,
			2 when something is badly wrong
	*/
	virtual int update() = 0;

protected:
	/*! This variable allows specifies the model type using bit-code hierarchies. */
	unsigned int m_modelTypeId;

private:

	friend class NandradModel;
	friend class StateModelGroup;
};


} // namespace NANDRAD_MODEL

#endif // AbstractStateDependencyH
