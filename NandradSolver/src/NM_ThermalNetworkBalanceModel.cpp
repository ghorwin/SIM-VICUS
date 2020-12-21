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

#include "NM_ThermalNetworkBalanceModel.h"

#include <NANDRAD_ModelInputReference.h>

#include "NM_KeywordList.h"


namespace NANDRAD_MODEL {

void ThermalNetworkBalanceModel::setup(ThermalNetworkStatesModel * statesModel) {
	m_statesModel = statesModel;
	// TODO: implement
}


void ThermalNetworkBalanceModel::resultDescriptions(std::vector<QuantityDescription> & resDesc) const {
	// TODO: implement
}


void ThermalNetworkBalanceModel::resultValueRefs(std::vector<const double *> &res) const {
	// first seach in m_results vector
	res.clear();
	// TODO: implement
}


const double * ThermalNetworkBalanceModel::resultValueRef(const QuantityName & quantityName) const {
	// TODO: implement
	return nullptr;
}


int ThermalNetworkBalanceModel::priorityOfModelEvaluation() const {
	// TODO: implement
	return -1;
}


void ThermalNetworkBalanceModel::initInputReferences(const std::vector<AbstractModel *> & models) {
	// TODO: implement
}


void ThermalNetworkBalanceModel::inputReferences(std::vector<InputReference> & inputRefs) const {
	// TODO: implement
}


void ThermalNetworkBalanceModel::setInputValueRefs(const std::vector<QuantityDescription> & /*resultDescriptions*/,
										 const std::vector<const double *> & resultValueRefs)
{
	// TODO: implement
}


void ThermalNetworkBalanceModel::stateDependencies(std::vector<std::pair<const double *, const double *> > & resultInputValueReferences) const {
	// TODO: implement
}


int ThermalNetworkBalanceModel::update() {

	// TODO: implement
	// signal success
	return 0;
}


int ThermalNetworkBalanceModel::ydot(double* ydot) {
	// copy values to ydot
	std::memcpy(ydot, &m_ydot[0], m_ydot.size() * sizeof (double));
	// signal success
	return 0;
}


} // namespace NANDRAD_MODEL

