/*	The NANDRAD data model library.

	Copyright (c) 2012-today, Institut für Bauklimatik, TU Dresden, Germany

	Primary authors:
	  Andreas Nicolai  <andreas.nicolai -[at]- tu-dresden.de>
	  Anne Paepcke     <anne.paepcke -[at]- tu-dresden.de>

	This library is part of SIM-VICUS (https://github.com/ghorwin/SIM-VICUS)

	This library is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.
*/

#include "NANDRAD_SolverParameter.h"
#include "NANDRAD_Constants.h"
#include "NANDRAD_KeywordList.h"

#include <IBK_Exception.h>
#include <IBK_StringUtils.h>
#include <IBK_algorithm.h>

#include <tinyxml.h>

namespace NANDRAD {


void SolverParameter::initDefaults() {
	m_para[P_RelTol].set( KeywordList::Keyword("SolverParameter::para_t", P_RelTol),												1e-5, IBK::Unit("---"));
	m_para[P_AbsTol].set( KeywordList::Keyword("SolverParameter::para_t", P_AbsTol),												1e-10, IBK::Unit("---"));
	m_para[P_MaxTimeStep].set( KeywordList::Keyword("SolverParameter::para_t", P_MaxTimeStep),										1, IBK::Unit("h"));
	m_para[P_MinTimeStep].set( KeywordList::Keyword("SolverParameter::para_t", P_MinTimeStep),										1e-8, IBK::Unit("s"));
	m_para[P_InitialTimeStep].set( KeywordList::Keyword("SolverParameter::para_t", P_InitialTimeStep),								0.1, IBK::Unit("s"));

	m_para[P_NonlinSolverConvCoeff].set( KeywordList::Keyword("SolverParameter::para_t", P_NonlinSolverConvCoeff),					0.1, IBK::Unit("---"));
	m_para[P_IterativeSolverConvCoeff].set( KeywordList::Keyword("SolverParameter::para_t", P_IterativeSolverConvCoeff),			0.05, IBK::Unit("---"));

	m_intPara[IP_MaxOrder].set( KeywordList::Keyword("SolverParameter::intPara_t", IP_MaxOrder),									5);
	m_intPara[IP_MaxNonlinIter].set( KeywordList::Keyword("SolverParameter::intPara_t", IP_MaxNonlinIter),							3);
	m_intPara[IP_MaxKrylovDim].set( KeywordList::Keyword("SolverParameter::intPara_t", IP_MaxKrylovDim),							50);
	m_intPara[IP_PreILUWidth].set( KeywordList::Keyword("SolverParameter::intPara_t", IP_PreILUWidth),								3);

	m_para[P_DiscMinDx].set( KeywordList::Keyword("SolverParameter::para_t", P_DiscMinDx),											2, IBK::Unit("mm"));
	m_para[P_DiscStretchFactor].set( KeywordList::Keyword("SolverParameter::para_t", P_DiscStretchFactor),							4, IBK::Unit("---"));
	m_intPara[IP_DiscMaxElementsPerLayer].set( KeywordList::Keyword("SolverParameter::intPara_t", IP_DiscMaxElementsPerLayer),		20);

	m_para[P_ViewfactorTileWidth].set( KeywordList::Keyword("SolverParameter::para_t", P_ViewfactorTileWidth),						50, IBK::Unit("cm"));
	m_para[P_SurfaceDiscretizationDensity].set( KeywordList::Keyword("SolverParameter::para_t", P_SurfaceDiscretizationDensity),	1, IBK::Unit("---"));
	m_para[P_ControlTemperatureTolerance].set(KeywordList::Keyword("SolverParameter::para_t", P_ControlTemperatureTolerance),		0.00001, IBK::Unit("K"));

	m_para[P_HydraulicNetworkAbsTol].set( KeywordList::Keyword("SolverParameter::para_t", P_HydraulicNetworkAbsTol),				0.001, IBK::Unit("---"));
	m_para[P_HydraulicNetworkMassFluxScale].set( KeywordList::Keyword("SolverParameter::para_t", P_HydraulicNetworkMassFluxScale),	1000, IBK::Unit("---"));

	m_flag[F_DetectMaxTimeStep].set( KeywordList::Keyword("SolverParameter::flag_t", F_DetectMaxTimeStep), true );
	m_flag[F_KinsolDisableLineSearch].set(KeywordList::Keyword("SolverParameter::flag_t", F_KinsolDisableLineSearch), false);
}


void SolverParameter::checkParameters() const {
	FUNCID(SolverParameter::checkParameters);
	if (m_intPara[IP_PreILUWidth].value <= 0)
		throw IBK::Exception("PreILUWidth must be > 0.", FUNC_ID);
}


bool SolverParameter::operator!=(const SolverParameter & other) const {
	if (m_integrator != other.m_integrator) return true;
	if (m_lesSolver != other.m_lesSolver) return true;
	if (m_preconditioner != other.m_preconditioner) return true;

	for (unsigned int i=0; i<NUM_P; ++i)
		if (m_para[i] != other.m_para[i])
			return true;

	for (unsigned int i=0; i<NUM_F; ++i)
		if (m_flag[i] != other.m_flag[i])
			return true;

	return false; // this and other hold the same data
}


} // namespace NANDRAD

