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

#ifndef NM_StateModelGroupH
#define NM_StateModelGroupH

#include "NM_AbstractModel.h"
#include "NM_AbstractStateDependency.h"
#include "NM_SteadyStateSolver.h"

#include <ZEPPELIN_DependencyGroup.h>

#include <nvector/nvector_serial.h>

namespace NANDRAD {
	class SolverParameter;
}

namespace SOLFRA {
	class JacobianInterface;
}

namespace IBKMK {
	class SparseMatrixPattern;
}

namespace NANDRAD_MODEL {

/*!	\brief Declaration for class StateModelGroup
	\author Andreas Nicolai <andreas.nicolai -[at]- tu-dresden.de>

	This class contains a group of models with objects of type
	'AbstractStateDependency' and is generated
	from an DependencyGroup. It optimises the model
	update procedure for dependency groups of type 'SEQUENTIAL' and 'CYCLIC'.
	It can optionally be constructed after the creation of a dependency graph from all models.
	Use the DependencyGroup objects that are stored in the 'm_orderedGroups' vector or the
	'm_orderedParallelGroups' vector for the constructor call.

	...
	DependencyGraph	modelDependencies;
	// set all abstract state objects in graph and compute evaluation order
	modelDependencies.setObjects( m_abstractStateObjects );
	// retreive ordered group vector
	std::vector<DependencyGroup*> orderedGroups = modelDependencies.orderedGroups();

	// construct a new group from each DependencyGroup
	... // for(unsigned int i; ...)
				StateModelGroup stateObjectGroup(id,name);
				stateObjectGroup.init(group)
				m_stateObjectGroups.push_back(stateObjectGroup);
	...

	\note This class only accepts dependency groups with objects that are derived from
	'AbstractStateDependency'. Otherwise an exception will be thrown.
	\note The copy-constructor only copies DependencyGroup information and allocates
	space for the AbstractStateModelGroupImpl object.
	Always call init() after construction!

	The update procedure for the current group:
	Groups of type 'SEQUENTIAL' update all models in sequence (from the last to the first
	element of the model vector).
	Groups of type 'CYCLIC' update all models by a Newton-Raphson procedure using the sundials
	solver 'KinSparse'.
	In order to get access to the class facilities you need to call the update function for the
	AbstractStateModelGroup stack instead of updating each model directly:

	...
	...// for(unsigned int i; ...)
		m_stateObjectGroups[i]->update();
	...

*/

#define EliminateSubgroups

class StateModelGroup: public AbstractStateDependency, public SteadyStateSolver {
public:

	/*! Default constructor. */
	StateModelGroup();

	/*! Default destructor. */
	~StateModelGroup() override;

	/********* Functions inherited from AbstractStateDependency. ********/

	/*! Reference descriptions are empty.*/
//	virtual void inputReferenceDescriptions(std::vector<QuantityDescription> & /*refDesc*/) const { }

	/*! Return an empty vector.*/
	virtual void inputValueRefs(std::vector<const double *> & valueRefs) const override { (void)valueRefs; }

	/*! Re-implement this function in derived classes and handle all your update-functionality here. */
	virtual int update() override;

	/*! Returns a pattern of direkt computational dependencies between input values
	and result values of the current model group (the first pair entry correspondss
	to the input value, the second entry to the result value).
	*/
	virtual void stateDependencies(std::vector< std::pair<const double *, const double *> > &resultInputValueReferences) const override;


	/********* Functions inherited from ModelInterface. ********/

	/*! Initial condition vector, size n.*/
	virtual const double * y0() const;
	/*! We ignore setTime. */
	virtual SOLFRA::ModelInterface::CalculationResult setTime(double /*t*/) { return SOLFRA::ModelInterface::CalculationSuccess; }
	/*! Update state of model to new set of unknowns.
	*/
	virtual SOLFRA::ModelInterface::CalculationResult setY(const double * y);
	/*! Stores the computed derivatives of the solution variables in the vector ydot.
	\param ydot Pointer to linear memory array of size n() holding the time derivatives for each solution variable.
	*/
	virtual SOLFRA::ModelInterface::CalculationResult ydot(double * ydot);


	/********* Individual functions. ********/

	/*! Number of states. */
	unsigned int nStates() const;

	/*! Returns all models.*/
	const std::vector<AbstractStateDependency *> &models() const {return m_models;}

	/*! Returns group type.*/
	ZEPPELIN::DependencyGroup::Type groupType() const { return m_groupType; }

	/*! Constant access to all result references.*/
	void resultValueRefs(std::vector<const double *> &res);

	/*! Initializes group (non-linear solver) from a dependency group.*/
	void init(const ZEPPELIN::DependencyGroup &group, const NANDRAD::SolverParameter &solverPara);

	/*! Initializes all states and y-values.*/
	int initStates();

	/*! Updates all internal model states.
	*/
	void updateStates();


protected:

	/********* Functions inherited from AbstractStateDependency. ********/

	/*! Does nothing.*/
	virtual void inputReferences(std::vector<InputReference>  & /* inputRefs*/) const override { }
	/*! Does nothing.*/
	virtual void setInputValueRef(const InputReference &/*inputRef*/, const QuantityDescription & /*resultDesc*/, const double */*resultValueRef*/) override {}


private:
	/*! Generation of all value references to all models.*/
	void initResultValueRefs();
	/*! Generation of all value references to all models.*/
	void initConstraints();
	/*! Initialition of sparse jacobian.*/
	void initSparseSolver(IBKMK::SparseMatrixPattern &pattern,
		IBKMK::SparseMatrixPattern &patternTranspose);
	/*! Initilization of dense jacobian.*/
	void initDenseSolver();
	/*! Penalty function for enforcing a maximum value*/
	double penalty(double value, double maxValue);

	/*! Pointer to global solver parameter. */
	const NANDRAD::SolverParameter			*m_solverParameter;
	/*! Vector of model objects. */
	std::vector<AbstractStateDependency *>	m_models;
	/*! Set storing all models with implicit dependencies
		that should be updated at each calculation start. */
	//std::set<unsigned int>					m_modelsNeedUpdateAtCalculationStart;
	/*! Dependency type of the model objects. */
	ZEPPELIN::DependencyGroup::Type			m_groupType;
	/*! Vector of references result values that are part of a cyclic group.*/
	std::vector<double *>					m_resultValueRefs;
	//	/*! Constraints vector for all results: lenght 0 == no constraints. */
	std::vector<int>						m_constraints;
	/*! Offset of the result quantities for each model inside resultsRef vector,
	size = number of models + 1. */
	std::vector<unsigned int>				m_modelResultsOffset;
	/*! Index inside resultsRef vector for a given y index. */
	std::vector<unsigned int>				m_resultIndex;
	/*! Success flag of the last kinsol call. */
	int										m_successOfLastKinsolCall;
	/*! Vector including offsets for result values from y-vector. */
	std::vector<double>						m_resultValueOffset;
	/*! Vector with maximum result values: inf() if undefined. */
	std::vector<double>						m_maximumResultValue;
	/*! Used to store y elements for Matrix setup and Newton iteration. */
	std::vector<double>						m_y;
	/*! Used to store ydot elements for Matrix setup and Newton iteration. */
	std::vector<double>						m_ydot;
	/*! Previously calculated solution. */
	std::vector<double>						m_yPrev;
	/*! Flag deciding whether we apply component wise scaling to Newton method. */
	bool									m_scalePerValue;
	/*! Solver tolerance (weighted with y-value). */
	double									m_tolerance;
	/*! Solver safety band for y-values close to zero. */
	double									m_absoluteToleranceBand;
	/*! Relaxation vector for nonlinear solver. */
	std::vector<double>						m_sc;

	/*! Sparse matrix indices. */
	std::vector<unsigned int>				m_ia;
	std::vector<unsigned int>				m_ja;
	/*! Transpose sparse matrix indices. */
	std::vector<unsigned int>				m_iaT;
	std::vector<unsigned int>				m_jaT;

	friend class NandradModelImpl;
};

} // namespace NANDRAD_MODEL

#endif // NM_StateModelGroupH
