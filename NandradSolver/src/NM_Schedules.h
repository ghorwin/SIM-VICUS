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

#ifndef NM_SchedulesH
#define NM_SchedulesH

#include <string>
#include <map>

#include <NANDRAD_LinearSplineParameter.h>

#include "NM_AbstractModel.h"
#include "NM_AbstractTimeDependency.h"

namespace NANDRAD {
	class Project;
	class Schedules;
}

namespace NANDRAD_MODEL {

class InputReference;


/*! Provides access to scheduled quantities.
	Schedules provide by default only time-dependent values with no dependence on other
	objects. Also, the evaluation is guaranteed to be done prior to any other models.

	During initialization, the data from NANDRAD::Schedules is cached in this object such, that is can
	be accessed by other models very efficiently. During the initialization, it is checked that:
	- all schedules have definitions for a full time range (i.e. all days types are covered)
	- each object list is referenced only once
*/
class Schedules : public AbstractTimeDependency {
public:
	// *** PUBLIC MEMBER FUNCTIONS

	/*! Generate variable reference list from defined schedules. */
	void setup(const NANDRAD::Project & project);


	/*! Retrieves reference pointer to a value with given quantity ID name.
		\param quantityName An identifier for the quantity.
		\return Returns a persistent pointer to the storage location or nullptr, if the requested
			variable is not provided. If not a nullptr, the details of the requested variable
			are stored in quantityDesc.
	*/
	const double * resolveResultReference(const InputReference & valueRef, QuantityDescription & quantityDesc) const;


	// *** Re-implemented from AbstractTimeDependency

	/*! Updates the state of the schedules object to the time point stored in AbstractTimeStateObject::m_t.
		This function updates all internally cached results to match the new time point.
		Afterwards, these time points can be retrieved very efficiently several times
		through the various access functions.

		\note The simulation time is shifted by the offset from start year and start time to get an
			  absolute time reference. Then it is passed to the climate calculation module.
	*/
	virtual int setTime(double t) override;

	// *** Other public member functions

	/*! Returns start value of a quantity. Throws an exception, if quantity does not exist. */
	double startValue(const QuantityName & quantityName) const;

private:
	/*! Utility function that retrieves an object list object for a given name (from schedule group). */
	const NANDRAD::ObjectList * objectListByName(const std::string & objectListName) const;


	/*! Year of simulation. */
	int												m_year = 0;
	/*! Time from the beginning of the year in [s]. */
	double											m_startTime = 0;

	/*! Pointer to all object lists defined in the project (initialized in setup). */
	const std::vector<NANDRAD::ObjectList>			*m_objectLists = nullptr;
	/*! This vector indicates which object lists are referenced by schedules. */
	std::vector<unsigned int>						m_objectListIndexes;

	/*! Pointer to the schedules data structure. */
	const NANDRAD::Schedules						*m_schedules;


	// The following vectors have all the same size and are resized in setup()

	/*! Names of variables in format '<objectListName>::<VariableName>' matching the results in m_results. */
	std::vector<std::string>						m_variableNames;
	/*! Corresponding input/output units (Note: results are always stored in the respective base SI unit). */
	std::vector<IBK::Unit>							m_variableUnits;
	/*! Linear splines with the actual data, x values are time points in [s], y values are the data values
		in base SI units.
	*/
	std::vector<IBK::LinearSpline>					m_valueSpline;
	/*! Variables, computed/updated during the calculation.	*/
	std::vector<double>								m_results;
};


} // namespace NANDRAD_MODEL

#endif // NM_SchedulesH
