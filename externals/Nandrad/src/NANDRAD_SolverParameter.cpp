/*	The NANDRAD data model library.

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

#include "NANDRAD_SolverParameter.h"
#include "NANDRAD_Constants.h"
#include "NANDRAD_KeywordList.h"

#include <IBK_Exception.h>
#include <IBK_StringUtils.h>
#include <IBK_algorithm.h>

#include <tinyxml.h>

namespace NANDRAD {


void SolverParameter::initDefaults() {
	m_para[SP_RELTOL].set( KeywordList::Keyword("SolverParameter::para_t", SP_RELTOL),																		1e-5, IBK::Unit("---"));
	m_para[SP_ABSTOL].set( KeywordList::Keyword("SolverParameter::para_t", SP_ABSTOL),																		1e-10, IBK::Unit("---"));
	m_para[SP_MAX_DT].set( KeywordList::Keyword("SolverParameter::para_t", SP_MAX_DT),																		1, IBK::Unit("h"));
	m_para[SP_MIN_DT].set( KeywordList::Keyword("SolverParameter::para_t", SP_MIN_DT),																		1e-12, IBK::Unit("s"));
	m_para[SP_INITIAL_DT].set( KeywordList::Keyword("SolverParameter::para_t", SP_INITIAL_DT),																0.1, IBK::Unit("s"));

	m_para[SP_NONLINSOLVERCONVCOEFF].set( KeywordList::Keyword("SolverParameter::para_t", SP_NONLINSOLVERCONVCOEFF),										0.1, IBK::Unit("---"));
	m_para[SP_ITERATIVESOLVERCONVCOEFF].set( KeywordList::Keyword("SolverParameter::para_t", SP_ITERATIVESOLVERCONVCOEFF),									0.05, IBK::Unit("---"));

	m_intPara[SIP_MAX_ORDER].set( KeywordList::Keyword("SolverParameter::intPara_t", SIP_MAX_ORDER),															5);
	m_intPara[SIP_MAX_NONLIN_ITER].set( KeywordList::Keyword("SolverParameter::intPara_t", SIP_MAX_NONLIN_ITER),												3);
	m_intPara[SIP_MAX_KRYLOV_DIM].set( KeywordList::Keyword("SolverParameter::intPara_t", SIP_MAX_KRYLOV_DIM),													50);

	m_para[SP_DISCRETIZATION_MIN_DX].set( KeywordList::Keyword("SolverParameter::para_t", SP_DISCRETIZATION_MIN_DX),											2, IBK::Unit("mm"));
	m_para[SP_DISCRETIZATION_STRECH_FACTOR].set( KeywordList::Keyword("SolverParameter::para_t", SP_DISCRETIZATION_STRECH_FACTOR),								4, IBK::Unit("---"));
	m_intPara[SIP_DISCRETIZATION_MAX_ELEMENTS_PER_LAYER].set( KeywordList::Keyword("SolverParameter::intPara_t", SIP_DISCRETIZATION_MAX_ELEMENTS_PER_LAYER),	10);

	m_para[SP_VIEW_FACTOR_TILE_WIDTH].set( KeywordList::Keyword("SolverParameter::para_t", SP_VIEW_FACTOR_TILE_WIDTH),											50, IBK::Unit("cm"));
	m_para[SP_SURFACE_DISCRETIZATION_DENSITY].set( KeywordList::Keyword("SolverParameter::para_t", SP_SURFACE_DISCRETIZATION_DENSITY),							 1, IBK::Unit("---"));
	m_para[SP_CONTROL_TEMPERATURE_TOLERANCE].set(KeywordList::Keyword("SolverParameter::para_t", SP_CONTROL_TEMPERATURE_TOLERANCE),							0.00001, IBK::Unit("K"));
	m_para[SP_INTEGRAL_WEIGHTS_FACTOR].set(KeywordList::Keyword("SolverParameter::para_t", SP_INTEGRAL_WEIGHTS_FACTOR),											1e-05, IBK::Unit("---"));

	m_flag[SF_DETECT_MAX_DT].set( KeywordList::Keyword("SolverParameter::flag_t", SF_DETECT_MAX_DT), true );
	m_flag[SF_KINSOL_DISABLE_LINE_SEARCH].set(KeywordList::Keyword("SolverParameter::flag_t", SF_KINSOL_DISABLE_LINE_SEARCH), false);

	// no defaults: SP_PRE_BANDWIDTH, SP_PRE_ILUWIDTH, because default values for these parameters are determined
	//              with an algorithm inside the NandradModel/Solver

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

