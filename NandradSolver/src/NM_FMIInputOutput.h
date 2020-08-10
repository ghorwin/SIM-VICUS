/*	The Nandrad model library.

Copyright (c) 2020, Institut fuer Bauklimatik, TU Dresden, Germany

Written by
A. Nicolai		<andreas.nicolai -[at]- tu-dresden.de>
A. Paepcke		<anne.paepcke -[at]- tu-dresden.de>
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

#ifndef NM_FMUInputOutputH
#define NM_FMUInputOutputH

#include "NM_InputReference.h"
#include "NM_AbstractModel.h"
#include "NM_AbstractTimeDependency.h"
#include "NM_AbstractStateDependency.h"

namespace NANDRAD {
	class Project;
}

namespace NANDRAD_MODEL {

/*! Central interface class for FMU export/import of the *entire* NANDRAD model.

	This class provides a set of input variables (basically fakes variable results as
	created by other models) and collects output quantities (just as regular outputs).

	The FMUInputOutput object is special and not handled as other models - both in initialization
	(variable lookup) as in calculation. Its state changes descretely only through calls from
	the FMI master. Output variables are retrieved only when requested by the master.

	\note This object only implements AbstractModel to avoid code duplication in NandradModel::initModelDependencies().
		The functions are not implemented and should/must never be called.
*/
class FMIInputOutput : public AbstractTimeDependency, public AbstractStateDependency, public AbstractModel {
public:

	/*! Data initialization.
		\todo Add arguments as necessary and implement.
	*/
	void setup(const NANDRAD::Project & prj);

	// *** Re-implemented from AbstractModel

	/*! Not needed, FMI input vars are "overrding" variables and reference type is not really meaningful here. */
	virtual NANDRAD::ModelInputReference::referenceType_t referenceType() const override {
		return NANDRAD::ModelInputReference::MRT_GLOBAL;
	}

	/*! Return unique class ID name of implemented model. */
	virtual const char * ModelIDName() const override { return "FMIInputOutput"; }

	/*! Not implemented, since not needed. */
	virtual unsigned int id() const override { return 0; }

	/*! Not implemented, since not needed. */
	virtual const char * displayName() const override { return "FMI interface model"; }

	/*! Not implemented, since not needed. */
	virtual void resultDescriptions(std::vector<QuantityDescription> & /*resDesc*/) const override {}

	/*! Not implemented, since not needed (FMI result vars are handled in a special way). */
	virtual const double * resultValueRef(const QuantityName & /*quantityName*/) const override { return nullptr; }

	/*! Not implemented, since not needed (FMI result vars are handled in a special way). */
	virtual void resultValueRefs(std::vector<const double *> & /*res*/) const override {}

	/*! Not implemented, since not needed. */
	virtual void initResults(const std::vector<AbstractModel*> & /* models */) override {}


	// *** Re-implemented from AbstractTimeDependency

	/*! Updates the state of the loads object to the time point stored in DefaultTimeStateModel::m_t.
		This function updates all internally cached results to match the new time point.
		Afterwards, these time points can be retrieved very efficiently several times
		through the various access functions.

		\param t The simulation time (relative to simulation start) in s.

		\note The simulation time is shifted by the offset from start year and start time to get an
			  absolute time reference. Then it is passed to the climate calculation module.
	*/
	virtual int setTime(double t) override;


	// *** Re-implemented from AbstractStateDependency

	/*! Returns vector with model input references.
		Implicit models must generate their own model input references and populate the
		vector argument.
		\note This function is not the fastest, so never call this function from within the solver
		(except maybe for output writing).
	*/
	virtual void inputReferences(std::vector<InputReference>  & inputRefs) const override;

	/*! Returns vector with pointers to memory locations matching input value references. */
	virtual const std::vector<const double *> & inputValueRefs() const override { return m_valueRefs; }

	/*! Sets all object dependencies.
		Called when all model results have been initialized (i.e. the function
		initResults() was called in all model objects).

		Re-implement this function and provide all information, so that function
		inputReferences() can be called afterwards (e.g. resize and fill a vector
		of Type std::vector<ModelInputReference>).
	*/
	virtual void initInputReferences(const std::vector<AbstractModel*> & /* models */) override;

	/*! Sets a single input value reference (persistent memory location) that refers to the requested input reference.
		\param inputRef An input reference from the previously published list of input references.
		\param resultValueRef Persistent memory location to the variable slot.
	*/
	virtual void setInputValueRef(const InputReference &inputRef, const double *resultValueRef) override;

	/*! Since FMI export only pulls data when master requests, we have nothing to do here. */
	virtual int update() override { return 0; }


	// *** Other public member functions

	/*! Retrieves reference pointer to a requested input reference.

		This function looks through the list of published FMI input variables and parameters and
		checks, if any of these matches the requested variable.

		For example, if the input variable name is "Zone[14].AirTemperature", this corresponds to
		an input references of:
		- reference type = MRT_ZONE
		- id = 14
		- variable name = AirTemperature

		If you want to provide an input variable for an object list, you can write: "All zones.HeatingSetPoint"
		where an object list exists with name "All zones", referenzing zones with any ID (*). Then, the variable will
		be matched by any request with:
		- reference type = MRT_ZONE
		- id = * (any)
		- variable name = HeatingSetPoint

		Hereby, a potentially existing HeatingSetPoint schedule can be overwritten (or provided, if missing).

		\return Returns a persistent pointer to the storage location or nullptr, if the requested
			variable is not provided as FMU input/parameter variable.
	*/
	const double * resultValueRef(const InputReference & valueRef) const;

private:

	/*! Stored value references (pointers to result variables exported via FMI). */
	std::vector<const double *>		m_valueRefs;

	/*! Cached current values, updated in setTime().
		These values will be updated based on cached FMI variable input data.
	*/
	std::vector<double>				m_results;
};

} // namespace NANDRAD_MODEL


#endif // NM_FMUInputOutputH
