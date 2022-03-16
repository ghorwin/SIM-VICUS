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

#include "NM_StateModelGroup.h"

#include "NM_DefaultStateDependency.h"
//#include "NM_HydraulicCircuidModel.h"
//#include "NM_DAEMatrixPattern.h"
//#include "NM_DetailedWindowModel.h"
//#include "NM_AbstractODEBalanceModel.h"

#include <NANDRAD_SolverParameter.h>

//#include <SOLFRA_JacobianDense.h>
#include <SOLFRA_JacobianSparseCSR.h>

#include <IBK_assert.h>
#include <IBK_math.h>
#include <IBK_messages.h>

#include <IBKMK_SparseMatrixPattern.h>

#include <limits>

namespace NANDRAD_MODEL {


// *** Implementation of StateModelGroup ***

StateModelGroup::StateModelGroup( ) :
	m_solverParameter(nullptr),
	m_successOfLastKinsolCall(SOLFRA::ModelInterface::CalculationSuccess),
	m_scalePerValue(true),
	m_tolerance(1e-09),
	m_absoluteToleranceBand(0.1)
{
	AbstractStateDependency::m_modelTypeId = 0;
}


StateModelGroup::~StateModelGroup(){
}


void StateModelGroup::init(const ZEPPELIN::DependencyGroup &group,
	const NANDRAD::SolverParameter &solverPara) {

	// set solver parameter
	m_solverParameter = &solverPara;

	FUNCID(StateModelGroup::init);
	// fill the model vector
	for( unsigned int i = 0; i < group.dependencyObjects().size(); ++i)
	{
		// ensure that model is also time dependend
		AbstractStateDependency* abstractStateObject = dynamic_cast<AbstractStateDependency*>
			(group.dependencyObjects()[i]) ;
		// error: implemented from wrong model type
		if(abstractStateObject == nullptr)
		{
			throw IBK::Exception(IBK::FormatString("Error initializing StateModelGroup: "
										"DependencyObjects must derive from 'AbstractStateDependency' !"), FUNC_ID);
		}
		// store inside dependency container
		m_models.push_back(abstractStateObject);

		if (dynamic_cast<SOLFRA::ModelInterface*>(group.dependencyObjects()[i]) != nullptr)
			m_modelTypeId |= DefaultStateDependency::ODE;
	}
	// retrieve type
	m_groupType = group.type();

	m_modelTypeId |= DefaultStateDependency::SteadyState;
	if (m_groupType == ZEPPELIN::DependencyGroup::CYCLIC)
		m_modelTypeId |= DefaultStateDependency::CyclicGroup;

	// retrieve optional settings for relative and absolute tolerance
	if (!solverPara.m_para[NANDRAD::SolverParameter::P_KinsolRelTol].name.empty()) {
		double relTol = solverPara.m_para[NANDRAD::SolverParameter::P_KinsolRelTol].value;
		// we need to define a integrator realtive tolerance
		IBK_ASSERT(!solverPara.m_para[NANDRAD::SolverParameter::P_RelTol].name.empty());
		// solver relative tolerance gives an upper limit for Kinsol tolerance
		double integratorRelTol = solverPara.m_para[NANDRAD::SolverParameter::P_RelTol].value;
		// check validity
		if (relTol <= 0 || relTol > integratorRelTol) {
			throw IBK::Exception(IBK::FormatString("Error initializing StateModelGroup: "
				"Mismatching value of parameter 'KinsolRelTol' (must be in (0,%#1])!")
				.arg(relTol), FUNC_ID);
		}
		// copy value
		m_tolerance = relTol;
	}
	if (!solverPara.m_para[NANDRAD::SolverParameter::P_KinsolAbsTol].name.empty()) {
		double absTol = solverPara.m_para[NANDRAD::SolverParameter::P_KinsolAbsTol].value;
		if (absTol < 0) {
			throw IBK::Exception(IBK::FormatString("Error initializing StateModelGroup: "
				"Mismatching value of parameter 'KinsolAbsTol' (must >= 0)!"), FUNC_ID);
		}
		// copy value
		m_absoluteToleranceBand = absTol/m_tolerance;
	}
	if (!solverPara.m_intPara[NANDRAD::SolverParameter::IP_KinsolMaxNonlinIter].name.empty()) {
		m_maximumNonlinearIterations = solverPara.m_intPara[NANDRAD::SolverParameter::IP_KinsolMaxNonlinIter].toUInt(true,
			"Mismatching value of parameter 'KinsolMaxNonlinIter' (must >= 0)!");
	}
}


void StateModelGroup::stateDependencies(std::vector< std::pair<const double *, const double *> > &resultInputValueReferences) const {

	// first clear all previous entries
	resultInputValueReferences.clear();
	// select all dependencies from local models and transport to outside
	for (unsigned int i = 0; i < m_models.size(); ++i) {
		std::vector< std::pair<const double *, const double *> > modelResultInputValueReferences;
		m_models[i]->stateDependencies(modelResultInputValueReferences);
		resultInputValueReferences.insert(resultInputValueReferences.end(), modelResultInputValueReferences.begin(),
			modelResultInputValueReferences.end());
	}
}


void StateModelGroup::initResultValueRefs() {
#if 0
	m_modelResultsOffset.push_back(0);
	for (std::vector<AbstractStateDependency*>::const_iterator it = m_models.begin();
		it != m_models.end(); ++it)
	{
		// do not allow empty model references
		IBK_ASSERT(*it != nullptr);
		// vector of local dependencies
		std::vector< std::pair<const double *, const double *> > dependencies;
		(*it)->stateDependencies(dependencies);
		// skip empty references
		if(dependencies.empty())
			continue;

		// exclude all y and ydot references
		std::vector<const double *> valueRefsY, valueRefsYdot;
		// cast to ODE balance model
		const AbstractODEBalanceModel *balanceModel = dynamic_cast<AbstractODEBalanceModel*>(*it);
		if (balanceModel != nullptr) {

			const double *valueRefYStart = balanceModel->ODEStatesValueRef();
			const double *valueRefYdotStart = balanceModel->ODEDivergencesValueRef();

			for (unsigned int i = 0; i < balanceModel->n(); ++i) {
				valueRefsY.push_back(valueRefYStart + i);
				valueRefsYdot.push_back(valueRefYdotStart + i);
			}
		}


		// sort all value references into rows and calculate an offset for each reference
		std::vector<double *> localResultValueRefs;
		// insert all references (and cluster all dependency to one resukt value reference)
		for(std::vector< std::pair<const double *,const double *> >::const_iterator
			refIt = dependencies.begin();
			refIt != dependencies.end(); ++refIt)
		{
			// exclude references for y and ydot
			double * resultValueRef = const_cast<double *>(refIt->first);
			if (std::find(valueRefsY.begin(), valueRefsY.end(), resultValueRef) !=
				valueRefsY.end())
				continue;
			if (std::find(valueRefsYdot.begin(), valueRefsYdot.end(), resultValueRef) !=
				valueRefsYdot.end())
				continue;
			// check if result value reference already exists (row)
			std::vector< double *>::iterator valueRefIt = std::find(localResultValueRefs.begin(),
				localResultValueRefs.end(), resultValueRef);
			// we already registered a reference for the corresponding result value
			if (valueRefIt != localResultValueRefs.end())
				continue;
			// register new result value referemce
			localResultValueRefs.push_back(resultValueRef);
		}
		// copy into rows vector
		m_resultValueRefs.insert(m_resultValueRefs.end(), localResultValueRefs.begin(), localResultValueRefs.end() );
		// store offset for next model results
		m_modelResultsOffset.push_back((unsigned int) m_resultValueRefs.size());
	}
#endif
}


void StateModelGroup::initConstraints() {

	IBK_ASSERT(!m_resultValueRefs.empty());

	//	initialize constraints vector
	m_constraints.resize(m_resultValueRefs.size(), 0);

	// resize result value offsets
	m_resultValueOffset.resize(m_resultValueRefs.size(), 0.0);
	m_maximumResultValue.resize(m_resultValueRefs.size(), std::numeric_limits<double>::max());

	std::map<const double*, std::pair<double, double> > constraintsPerValueRef;

	for (std::vector<AbstractStateDependency*>::const_iterator it = m_models.begin();
		it != m_models.end(); ++it)
	{
		// do not allow empty model references
		IBK_ASSERT(*it != nullptr);
		// vector of local dependencies
		std::map< const double *, std::pair<double, double> > localConstraintsPerValueRef;
		(*it)->constraints(localConstraintsPerValueRef);
		// skip empty references
		if (localConstraintsPerValueRef.empty())
			continue;
		// copy into rows vector
		constraintsPerValueRef.insert(localConstraintsPerValueRef.begin(),
			localConstraintsPerValueRef.end());
	}
	// no constraints
	if (constraintsPerValueRef.empty()) {
		return;
	}
	// enable or disable constraints
	for (unsigned int i = 0; i < m_resultValueRefs.size(); ++i) {
		const double *valuePtr = m_resultValueRefs[i];
		IBK_ASSERT(valuePtr != nullptr);
		std::map<const double*, std::pair<double, double> >::const_iterator
			it = constraintsPerValueRef.find(valuePtr);
		// change value for defined constraints
		if (it != constraintsPerValueRef.end() && it->second.first != -std::numeric_limits<double>::max()) {
			m_constraints[i] = 1;
			m_resultValueOffset[i] = it->second.first;
		}
		if (it != constraintsPerValueRef.end()) {
			m_maximumResultValue[i] = it->second.second;
		}
	}
}


void StateModelGroup::initSparseSolver(IBKMK::SparseMatrixPattern &pattern,
	IBKMK::SparseMatrixPattern &patternTranspose)
{
#if 0
	FUNCID(StateModelGroup::initSparseSolver);
	if (m_resultValueRefs.empty()) {
		// generate all dependency information
		initResultValueRefs();
		// generate constraints vector
		initConstraints();
	}

	// check which equations to remove
	std::set<unsigned int> removedIndexes;
	std::set<unsigned int> allColumns;

	// ********** remove all non cyclic connected quantities ****************

	// create an pattern for full elimination
	IBKMK::SparseMatrixPattern cyclicPattern(nStates()), cyclicPatternTranspose(nStates());

	// copy original pattern
	for (unsigned int i = 0; i < nStates(); ++i) {
		std::vector<unsigned int> columns;
		pattern.indexesPerRow(i, columns);
		for (unsigned jIdx = 0; jIdx < columns.size(); ++jIdx) {
			unsigned int j = columns[jIdx];
			cyclicPattern.set(i, j);
			cyclicPatternTranspose.set(j, i);
		}
	}

	// perform full elimination in order to filter cyclic dependencies
	DAEMatrixPattern::calculateTransitiveClosure(cyclicPattern, cyclicPatternTranspose,
		nStates());

	// now filter all equations without diagonal entry
	for (unsigned int i = 0; i < nStates(); ++i) {
		// skip larger indexes
		if(!cyclicPattern.test(i,i)) {
			removedIndexes.insert(i);
		}
	}

#ifdef EliminateSubgroups
	// ********** remove subgroups ****************
	IBK_ASSERT(m_models.size() + 1 == m_modelResultsOffset.size());
	for (unsigned int modelIdx = 0; modelIdx < m_models.size();
		++modelIdx)
	{
		// an internal model group has its own solver and therefore is eliminated
		// from current pattern
		if (dynamic_cast<StateModelGroup *>(m_models[modelIdx]) != nullptr
			|| dynamic_cast<HydraulicCircuidModel *>(m_models[modelIdx]) != nullptr) {
			for (unsigned int i = m_modelResultsOffset[modelIdx];
				i < m_modelResultsOffset[modelIdx + 1]; ++i) {
				removedIndexes.insert(i);
				// store information
				//m_modelsNeedUpdateAtCalculationStart.insert(modelIdx);
			}
		}
	}
#endif

	for (std::set<unsigned int>::const_iterator
		it = removedIndexes.begin();
		it != removedIndexes.end(); ++it) {
		DAEMatrixPattern::eliminateEquation(pattern, patternTranspose, nStates(), *it);
	}

	// copy data
	if (!m_ia.empty())
		m_ia.clear();
	if (!m_ja.empty())
		m_ja.clear();

	// create CSR pattern from sparse matrix information
	DAEMatrixPattern::patternCSR(pattern, nStates(), removedIndexes, m_ia, m_ja, m_resultIndex);

	// no rows
	if (m_ia.size() == 1) {
		std::string modelIdStr;
		for (unsigned int modelIdx = 0; modelIdx < m_models.size();
			++modelIdx)
		{
			if (modelIdx > 0)
				modelIdStr += std::string(", ");
			const AbstractModel * model = dynamic_cast<const AbstractModel *>(m_models[modelIdx]);
			IBK_ASSERT(model != nullptr);

			modelIdStr += model->ModelIDName() + std::string("[id=") +
				IBK::val2string<unsigned int>(model->id()) + std::string("]");
		}
		throw IBK::Exception(IBK::FormatString("Model implementation error. "
			"Invalid prognosis of a cyclic connection for models #%1. "
			"Individual model results are not connected in a cyclic way! "
			"Maybe there is an error in the implementation of 'stateDependencies'-function?")
			.arg(modelIdStr), FUNC_ID);
	}

	// we need a matrix with diagonal entries
	std::vector<unsigned int>::iterator jaIt = m_ja.begin();
	for (unsigned int i = 0; i < m_ia.size() - 1; ++i) {
		unsigned int jIdxStart = m_ia[i];
		unsigned int jIdxEnd = m_ia[i + 1];
		unsigned int jIdx = jIdxStart;
		// search for diagonal element
		for (; jIdx < jIdxEnd; ++jIdx, ++jaIt) {
			if (*jaIt >= i)
				break;
		}
		// no diagonal element found
		if (jIdx == jIdxEnd || jaIt == m_ja.end() || *jaIt > i) {
			// insert missing element
			jaIt = m_ja.insert(jaIt, i);
			// increase row counters
			for (unsigned int k = i + 1; k < m_ia.size(); ++k)
				++m_ia[k];
			// increase end counter for current row
			jIdxEnd = m_ia[i + 1];
		}
		// set iterator
		for (; jIdx < jIdxEnd; ++jIdx) {
			++jaIt;
		}
	}

	// resize all vectors
	m_y.resize(m_ia.size() - 1, 0.0);
	m_ydot.resize(m_ia.size() - 1, 0.0);
	m_yPrev.resize(m_ia.size() - 1, 0.0);

	// initialize all model states
	initStates();
	// initialize local solution
	std::memcpy(&m_yPrev[0], &m_y[0], n() * sizeof(double));

	// resize scaling vectors
	m_sc.resize(m_ia.size() - 1, 1.0);

	// resize constraints for new set of results
	std::vector<int> constraints = m_constraints;
	m_constraints.resize(m_ia.size() - 1);

	IBK_ASSERT(m_resultIndex.size() == m_ia.size() - 1);
	for (unsigned int i = 0; i < m_ia.size() - 1; ++i) {
		unsigned int idx = m_resultIndex[i];
		m_constraints[i] = constraints[idx];
	}

	IBK_ASSERT(m_solverParameter != nullptr);
	// initialize sparse solver
	SteadyStateSolver::initSparseSolver(m_ia, m_ja, m_iaT, m_jaT);
	SteadyStateSolver::setConstraints(&m_constraints[0]);
	// set tolerance and options

	int options = 0;
	SteadyStateSolver::setTolerance(m_tolerance);
	// add line search to solver options
	if (m_solverParameter->m_flag[NANDRAD::SolverParameter::SF_KINSOL_DISABLE_LINE_SEARCH].name().empty()
		|| !m_solverParameter->m_flag[NANDRAD::SolverParameter::SF_KINSOL_DISABLE_LINE_SEARCH].isEnabled()) {
		options |= KinsolLineSearch;
	}
	// add strict Newton to solver options
	if (!m_solverParameter->m_flag[NANDRAD::SolverParameter::SF_KINSOL_STRICT_NEWTON].name().empty()
		&& m_solverParameter->m_flag[NANDRAD::SolverParameter::SF_KINSOL_STRICT_NEWTON].isEnabled()) {
		options |= KinsolStrictNewton;
	}

	SteadyStateSolver::setOptions(options);
#endif
}


void StateModelGroup::initDenseSolver( )
{

	if (m_resultValueRefs.empty()) {
		// generate all dependency information
		initResultValueRefs();
		// generate constraints vector
		initConstraints();
	}
	// copy offset values
	for (unsigned int i = 0; i < m_resultValueRefs.size(); ++i)
		m_resultIndex.push_back(i);

	// resize all vectors
	m_y.resize(m_resultValueRefs.size(), 0.0);
	m_ydot.resize(m_resultValueRefs.size(), 0.0);
	m_yPrev.resize(m_resultValueRefs.size(), 0.0);
	// initialize all model states
	initStates();

	// resize scaling vectors
	m_sc.resize(m_resultValueRefs.size(), 1.0);

	// resize constraints for new set of results
	std::vector<int> constraints = m_constraints;
	m_constraints.resize(m_resultValueRefs.size());

	for (unsigned int i = 0; i < m_resultValueRefs.size(); ++i) {
		m_constraints[i] = constraints[i];
	}
	// initialize dense solver
	SteadyStateSolver::initDenseSolver((unsigned int) m_resultValueRefs.size());
	SteadyStateSolver::setConstraints(&m_constraints[0]);
	// set tolerance and options
	SteadyStateSolver::setTolerance(m_tolerance);

	int options = 0;
	// add line search to solver options
	if (m_solverParameter->m_flag[NANDRAD::SolverParameter::F_KinsolDisableLineSearch].name().empty()
		|| !m_solverParameter->m_flag[NANDRAD::SolverParameter::F_KinsolDisableLineSearch].isEnabled()) {
		options |= KinsolLineSearch;
	}
	// add strict Newton to solver options
	if (!m_solverParameter->m_flag[NANDRAD::SolverParameter::F_KinsolStrictNewton].name().empty()
		&& m_solverParameter->m_flag[NANDRAD::SolverParameter::F_KinsolStrictNewton].isEnabled()) {
		options |= KinsolStrictNewton;
	}
	SteadyStateSolver::setOptions(options);
}


int StateModelGroup::update() {

	FUNCID(StateModelGroup::update);

	switch(m_groupType) {
		case ZEPPELIN::DependencyGroup::SEQUENTIAL : {
			int calculationResultFlag = 0;
			for (std::vector<AbstractStateDependency*>::const_iterator it = m_models.begin();
				it != m_models.end(); ++it)
			{
				// do not allow empty model references
				IBK_ASSERT(*it != nullptr);
				calculationResultFlag |= (*it)->update();
			}
			// enforce solver abort
			if (calculationResultFlag & 2)
				return 2;
			// ask solver to try again with smaller time step
			if (calculationResultFlag & 1)
				return 1;
			// signal success
			return 0;
		}

		case ZEPPELIN::DependencyGroup::CYCLIC : {

			// if no jacobian is defined set jacobian and solver to dense
			if (jacobianInterface() == nullptr) {
				initDenseSolver();
			}

			// if call was not successful, return an invalid solution
			if (m_successOfLastKinsolCall != CalculationSuccess) {
				// copy solution to model variables
				std::memcpy(&m_y[0], &m_yPrev[0], n() * sizeof(double));
				// set states to previous solution
				updateStates();
			}
			else {
				// compose an initial solution from current internal model states
				int result = initStates();
				// enforce solver abort
				if (result > 1)
					return result;
				// transfer initial solution for kinsol run
				std::memcpy(&m_yPrev[0], &m_y[0], n() * sizeof(double));
			}

			if (m_scalePerValue) {
				// calculate new scaling factors
				IBK_ASSERT(!m_sc.empty());
				// calculate new factors
				for (unsigned int i = 0; i < n(); ++i) {
					m_sc[i] = std::min(std::max(1.0 / (std::fabs(m_y[i]) + m_absoluteToleranceBand),
						1e-07), 1e+07);
				}
			}

			// start newton iteration
			m_successOfLastKinsolCall = newtonIteration(&m_sc[0], &m_sc[0]);

			// enforce solver abort
			if (m_successOfLastKinsolCall == CalculationAbort)
				return 2;
			// ask solver to try again with smaller time step
			else if (m_successOfLastKinsolCall == CalculationRecoverableError) {
				IBK::IBK_Message(IBK::FormatString("Convergence error of cyclic StateModelGroup. "
					"Try again with updated Jacobain or reduce time step.\n"),
					IBK::MSG_WARNING, FUNC_ID);
				// signal error
				return 1;
			}
			// set states to new solution
			updateStates();
			// transfer initial solution for kinsol run
			std::memcpy(&m_yPrev[0], &m_y[0], n() * sizeof(double));
			// signal success
			return 0;
		}
	}
	return 0;
}


unsigned int StateModelGroup::nStates() const {
	return (unsigned int) m_resultValueRefs.size();
}


const double * StateModelGroup::y0() const {
	// reset input values after each call and check which quantity has changed
	return &m_y[0];
}


SOLFRA::ModelInterface::CalculationResult StateModelGroup::setY(const double * y) {
	// copy y vector
	std::memcpy(&m_y[0], y, m_y.size() * sizeof(double));

	return SOLFRA::ModelInterface::CalculationSuccess;
}


SOLFRA::ModelInterface::CalculationResult StateModelGroup::ydot(double * ydot) {

	// calculate residuals with updated y-vector
	unsigned int rowIdx = 0;

//	int calculationResultFlag = 0;
	// with the current values we start a second call of the update
	for (unsigned int modelIdx = 0; modelIdx < m_models.size(); ++modelIdx) {
		// do not allow empty model references

//
//#ifdef EliminateSubgroups
//		// initializer additional information
//		if (m_modelsNeedUpdateAtCalculationStart.find(modelIdx)
//			!= m_modelsNeedUpdateAtCalculationStart.end()) {
//			calculationResultFlag |= model->update();
//		}
//		if (calculationResultFlag != 0) {
//			if (calculationResultFlag & 2)
//				return SOLFRA::ModelInterface::CalculationAbort;
//			else
//				return SOLFRA::ModelInterface::CalculationRecoverableError;
//		}
//#endif
//
		// reset input values after each call and check which quantity has changed
		for (unsigned int resIdx = m_modelResultsOffset[modelIdx];
			resIdx < m_modelResultsOffset[modelIdx + 1]; ++resIdx)
		{
			// special case: last quantity is not condiered inside system
			// (and row may exceed valid index)
			if (rowIdx == m_y.size())
				break;
			IBK_ASSERT(rowIdx < m_resultIndex.size());
			// skip non existent entries (these are
			// current because of model update)
			if (resIdx < m_resultIndex[rowIdx])
				continue;
			// retrieve the current row
			double *valuePtr = m_resultValueRefs[resIdx];
			IBK_ASSERT(valuePtr != nullptr);
			// and set quantity
			*valuePtr = m_y[rowIdx] + m_resultValueOffset[resIdx];
			// correct row index
			++rowIdx;
		}
	}
#if 0
	if (dynamic_cast<SOLFRA::JacobianDense*> (jacobianInterface()) != nullptr) {

		// we frequently update all models and compare values afterward
		for (unsigned int modelIdx = 0; modelIdx < m_models.size(); ++modelIdx) {
			// do not allow empty model references
			AbstractStateDependency *model = m_models[modelIdx];
			IBK_ASSERT(model != nullptr);
			calculationResultFlag |= model->update();
		}
		// reset input values after each call and check which quantity has changed
		for (unsigned int rowIdx = 0; rowIdx < m_y.size(); ++rowIdx)
		{
			// retrieve the current row
			double *valuePtr = m_resultValueRefs[rowIdx];
			IBK_ASSERT(valuePtr != nullptr);
			// retrieve error function for row rowIdx: F(y) = y - f(y)
			m_ydot[rowIdx] = m_y[rowIdx] + m_resultValueOffset[rowIdx] - *valuePtr;

			// add penalty term
			double maxValue = m_maximumResultValue[rowIdx];
			if(maxValue != std::numeric_limits<double>::max())
				m_ydot[rowIdx] += penalty(*valuePtr, maxValue);

			// reset value
			*valuePtr = m_y[rowIdx] + m_resultValueOffset[rowIdx];
		}
		if (calculationResultFlag != 0) {
			if (calculationResultFlag & 2)
				return SOLFRA::ModelInterface::CalculationAbort;
			else
				return SOLFRA::ModelInterface::CalculationRecoverableError;
		}
	}
	else {
		IBK_ASSERT(m_models.size() + 1 == m_modelResultsOffset.size());

		unsigned int rowIdx = 0;
		// with the current values we start a second call of the update
		for (unsigned int modelIdx = 0; modelIdx < m_models.size(); ++modelIdx) {
			// do not allow empty model references
			AbstractStateDependency *model = m_models[modelIdx];
			IBK_ASSERT(model != nullptr);

			calculationResultFlag |= model->update();

			// reset input values after each call and check which quantity has changed
			for (unsigned int resIdx = m_modelResultsOffset[modelIdx];
				resIdx < m_modelResultsOffset[modelIdx + 1]; ++resIdx)
			{
				// special case: last quantity is not condiered inside system
				// (and row may exceed valid index)
				if (rowIdx == m_y.size())
					break;
				IBK_ASSERT(rowIdx < m_resultIndex.size());
				// skip non existent entries
				if (resIdx < m_resultIndex[rowIdx])
					continue;

				// retrieve the current row
				double *valuePtr = m_resultValueRefs[resIdx];
				IBK_ASSERT(valuePtr != nullptr);
				// retrieve error function for row rowIdx: F(y) = y - f(y)
				m_ydot[rowIdx] = m_y[rowIdx] + m_resultValueOffset[resIdx] - *valuePtr;

				// add penalty term
				double maxValue = m_maximumResultValue[resIdx];
				if (maxValue != std::numeric_limits<double>::max())
					m_ydot[rowIdx] += penalty(*valuePtr, maxValue);

				// reset value
				*valuePtr = m_y[rowIdx] + m_resultValueOffset[resIdx];
				// correct row index
				++rowIdx;
			}
		}
		if (calculationResultFlag != 0) {
			if (calculationResultFlag & 2)
				return SOLFRA::ModelInterface::CalculationAbort;
			else
				return SOLFRA::ModelInterface::CalculationRecoverableError;
		}
	}
#endif
	// copy values
	std::memcpy(ydot, &m_ydot[0], n() * sizeof(double));

	return SOLFRA::ModelInterface::CalculationSuccess;
}


int StateModelGroup::initStates() {

	unsigned int rowIdx = 0;

	int calculationResultFlag = 0;
	// with the current values we start a second call of the update
	for (unsigned int modelIdx = 0; modelIdx < m_models.size(); ++modelIdx) {
		// do not allow empty model references
		AbstractStateDependency *model = m_models[modelIdx];
		IBK_ASSERT(model != nullptr);
		calculationResultFlag |= model->update();

		// reset input values after each call and check which quantity has changed
		for (unsigned int resIdx = m_modelResultsOffset[modelIdx];
			resIdx < m_modelResultsOffset[modelIdx + 1]; ++resIdx)
		{
			// special case: last quantity is not condiered inside system
			// (and row may exceed valid index)
			if (rowIdx == m_y.size())
				break;
			IBK_ASSERT(rowIdx < m_resultIndex.size());
			// skip non existent entries
			if (resIdx < m_resultIndex[rowIdx])
				continue;
			// retrieve the current row
			double *valuePtr = m_resultValueRefs[resIdx];
			IBK_ASSERT(valuePtr != nullptr);
			// retrieve error function for row rowIdx: F(y) = y - f(y)
			m_ydot[rowIdx] = m_y[rowIdx] + m_resultValueOffset[resIdx] - *valuePtr;
			// and set quantity
			//m_newtonStates[rowIdx] = *valuePtr - m_resultValueOffset[resIdx];
			m_y[rowIdx] = *valuePtr - m_resultValueOffset[resIdx];
			// correct row index
			++rowIdx;
		}
	}
	return calculationResultFlag;
}


void StateModelGroup::updateStates() {
	//std::memcpy(&m_newtonStates[0], &m_y[0], n() * sizeof(double));

	unsigned int rowIdx = 0;
	// with the current values we start a second call of the update
	for (unsigned int modelIdx = 0; modelIdx < m_models.size(); ++modelIdx) {

		// reset input values after each call and check which quantity has changed
		for (unsigned int resIdx = m_modelResultsOffset[modelIdx];
			resIdx < m_modelResultsOffset[modelIdx + 1]; ++resIdx)
		{
			// special case: last quantity is not condiered inside system
			// (and row may exceed valid index)
			if (rowIdx == m_y.size())
				break;
			IBK_ASSERT(rowIdx < m_resultIndex.size());
			// skip non existent entries (these are
			// current because of model update)
			if (resIdx < m_resultIndex[rowIdx])
				continue;
			// retrieve the current row
			double *valuePtr = m_resultValueRefs[resIdx];
			IBK_ASSERT(valuePtr != nullptr);
			// and set quantity
			//*valuePtr = m_newtonStates[rowIdx] + m_resultValueOffset[resIdx];
			*valuePtr = m_y[rowIdx] + m_resultValueOffset[resIdx];
			// correct row index
			++rowIdx;
		}
	}
}


double StateModelGroup::penalty(double value, double maxValue) {
	double alpha = 100.0;
	double beta  = 200.0;

	return alpha * IBK::f_exp(beta * (value - maxValue)/std::fabs(maxValue));
}


} // namespace NANDRAD_MODEL

