/*	NANDRAD Solver Framework and Model Implementation.

	Copyright (c) 2012-today, Institut für Bauklimatik, TU Dresden, Germany

	Primary authors:
	  Andreas Nicolai  <andreas.nicolai -[at]- tu-dresden.de>
	  Anne Paepcke     <anne.paepcke -[at]- tu-dresden.de>

	This program is part of SIM-VICUS (https://github.com/ghorwin/SIM-VICUS)

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.
*/

#ifndef NM_FMIInputOutputH
#define NM_FMIInputOutputH

#include "NM_InputReference.h"
#include "NM_AbstractModel.h"
#include "NM_AbstractTimeDependency.h"
#include "NM_AbstractStateDependency.h"

namespace NANDRAD {
	class FMIDescription;
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

	/*! Data initialization. */
	void setup(const NANDRAD::Project & prj);

	// *** Re-implemented from AbstractTimeDependency

	/*! Updates the input values (optionally using input value interpolation) to current simulation time.
		\param t Simulation time in [s] (solver time).
		\return Returns 0 when calculation was successful, 1 when a recoverable error has been detected,
			2 when something is badly wrong
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

	/*! Provides the object with references to requested input variables (persistent memory location). */
	virtual void setInputValueRefs(const std::vector<QuantityDescription> & resultDescriptions, const std::vector<const double *> & resultValueRefs) override;

	/*! Since FMI export only pulls data when master requests, we have nothing to do here. */
	virtual int update() override { return 0; }


	// *** Other public member functions

	/*! Sets a new input value for a given single id number. */
	void setFMIInputValue(unsigned int varID, double value);

	/*! Gets output value for a given single id number. */
	void getFMIOutputValue(unsigned int varID, double &value) const;

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
			variable is not provided as FMU input/parameter variable. If not a nullptr, the details of the requested variable
			are stored in quantityDesc.
	*/
	const double * resolveResultReference(const InputReference & valueRef, QuantityDescription & quantityDesc) const;

private:

	/*! Stored value references for input quantities (pointers to internal result variables that contain
		via FMI imported values), sorted via fmi id number. */
	std::map<unsigned int, double *>			m_FMIInputValueRefs;

	/*! Stored value references for output quantities (pointers to result variables exported via FMI),
		sorted via fmi id number. */
	std::map<unsigned int, const double *>		m_FMIOutputValueRefs;

	/*! Cached current values, updated in setTime().
		These values will be updated based on cached FMI variable input data.
		For all input variables with *different* value references a result value slot exists.
	*/
	std::vector<double>							m_results;

	/*! Stored constant reference to FMI description. */
	const NANDRAD::FMIDescription				*m_fmiDescription = nullptr;

	friend class NandradModelFMU;
};

} // namespace NANDRAD_MODEL


#endif // NM_FMIInputOutputH
