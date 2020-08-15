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


#include "NM_ConstructionBalanceModel.h"

namespace NANDRAD_MODEL {


void ConstructionBalanceModel::setup(const NANDRAD::ConstructionInstance & con,
									 const NANDRAD::SimulationParameter & simPara,
									 NANDRAD_MODEL::ConstructionStatesModel * statesModel)
{

}

void ConstructionBalanceModel::resultDescriptions(std::vector<QuantityDescription> & resDesc) const
{

}

void ConstructionBalanceModel::resultValueRefs(std::vector<const double *> & res) const
{

}

const double * ConstructionBalanceModel::resultValueRef(const QuantityName & quantityName) const
{

}

void ConstructionBalanceModel::initInputReferences(const std::vector<AbstractModel *> &)
{

}

void ConstructionBalanceModel::inputReferences(std::vector<InputReference> & inputRefs) const
{

}

const std::vector<const double *> & ConstructionBalanceModel::inputValueRefs() const
{

}

void ConstructionBalanceModel::setInputValueRef(const InputReference & inputRef, const QuantityDescription & resultDesc, const double * resultValueRef)
{

}

int ConstructionBalanceModel::update()
{

}

int ConstructionBalanceModel::ydot(double * ydot)
{

}



} // namespace NANDRAD_MODEL

