/*	The NANDRAD data model library.
Copyright (c) 2012, Institut fuer Bauklimatik, TU Dresden, Germany

Written by
A. Nicolai		<andreas.nicolai -[at]- tu-dresden.de>
A. Paepcke		<anne.paepcke -[at]- tu-dresden.de>
St. Vogelsang	<stefan.vogelsang -[at]- tu-dresden.de>
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

#include "NANDRAD_SolverParameter.h"
#include "NANDRAD_Constants.h"
#include "NANDRAD_KeywordList.h"

#include <IBK_Exception.h>
#include <IBK_StringUtils.h>
#include <IBK_algorithm.h>

#include <tinyxml.h>

namespace NANDRAD {



SolverParameter::SolverParameter():
	m_integrator(NUM_I),
	m_lesSolver(NUM_LES),
	m_preconditioner(NUM_PRE)
{
}


void SolverParameter::initDefaults() {
	m_para[SP_RELTOL].set( KeywordList::Keyword("SolverParameter::para_t", SP_RELTOL),									1e-5, IBK::Unit("---"));
	m_para[SP_ABSTOL].set( KeywordList::Keyword("SolverParameter::para_t", SP_ABSTOL),									1e-10, IBK::Unit("---"));
	m_para[SP_MAX_DT].set( KeywordList::Keyword("SolverParameter::para_t", SP_MAX_DT),									   1, IBK::Unit("h"));
	m_para[SP_MIN_DT].set( KeywordList::Keyword("SolverParameter::para_t", SP_MIN_DT),									   1e-12, IBK::Unit("s"));
	m_para[SP_INITIAL_DT].set( KeywordList::Keyword("SolverParameter::para_t", SP_INITIAL_DT),							 0.1, IBK::Unit("s"));

	m_para[SP_NONLINSOLVERCONVCOEFF].set( KeywordList::Keyword("SolverParameter::para_t", SP_NONLINSOLVERCONVCOEFF),		0.1, IBK::Unit("---"));
	m_para[SP_ITERATIVESOLVERCONVCOEFF].set( KeywordList::Keyword("SolverParameter::para_t", SP_ITERATIVESOLVERCONVCOEFF),	0.05, IBK::Unit("---"));

	m_para[SP_MAX_ORDER].set( KeywordList::Keyword("SolverParameter::para_t", SP_MAX_ORDER),							   5, IBK::Unit("---"));
	m_para[SP_MAX_KRYLOV_DIM].set( KeywordList::Keyword("SolverParameter::para_t", SP_MAX_KRYLOV_DIM),					 50, IBK::Unit("---"));
	m_para[SP_DISCRETIZATION_MIN_DX].set( KeywordList::Keyword("SolverParameter::para_t", SP_DISCRETIZATION_MIN_DX),	   2, IBK::Unit("mm"));
	m_para[SP_DISCRETIZATION_DETAIL].set( KeywordList::Keyword("SolverParameter::para_t", SP_DISCRETIZATION_DETAIL),	   2, IBK::Unit("---"));
	m_para[SP_VIEW_FACTOR_TILE_WIDTH].set( KeywordList::Keyword("SolverParameter::para_t", SP_VIEW_FACTOR_TILE_WIDTH),	   50, IBK::Unit("cm"));
	m_para[SP_SURFACE_DISCRETIZATION_DENSITY].set( KeywordList::Keyword("SolverParameter::para_t", SP_SURFACE_DISCRETIZATION_DENSITY),	   1, IBK::Unit("---"));
	m_para[SP_CONTROL_TEMPERATURE_TOLERANCE].set(KeywordList::Keyword("SolverParameter::para_t", SP_CONTROL_TEMPERATURE_TOLERANCE), 0.00001, IBK::Unit("K"));
	m_para[SP_INTEGRAL_WEIGHTS_FACTOR].set(KeywordList::Keyword("SolverParameter::para_t", SP_INTEGRAL_WEIGHTS_FACTOR), 1e-05, IBK::Unit("---"));

	m_flag[SF_DETECT_MAX_DT].set( KeywordList::Keyword("SolverParameter::flag_t", SF_DETECT_MAX_DT), true );
	m_flag[SF_KINSOL_DISABLE_LINE_SEARCH].set(KeywordList::Keyword("SolverParameter::flag_t", SF_KINSOL_DISABLE_LINE_SEARCH), false);

	// no defaults: SP_LES_BANDWIDTH, SP_PRE_BANDWIDTH, SP_PRE_ILUWIDTH, because default values for these parameters are determined
	//              with an algorithm inside the NandradModel/Solver

	// this is not working with IBK::clear!!!!which sets values to 0
}


void SolverParameter::readXML(const TiXmlElement * element) {

	const char * const FUNC_ID = "[SolverParameter::readXML]";

	// read all parameters
	const TiXmlElement * e;
	try {

		// read sub-elements
		for ( e = element->FirstChildElement(); e; e = e->NextSiblingElement()) {

			// determine data based on element name
			std::string ename = e->Value();
			if (ename == "Integrator") {
				std::string solverTypeName = e->GetText();

				if (! KeywordList::KeywordExists("SolverParameter::integrator_t", solverTypeName ) ) {
					throw IBK::Exception( IBK::FormatString(XML_READ_ERROR).arg(e->Row()).arg(
						IBK::FormatString("Unknown Integrator '%1'.").arg(solverTypeName)
						), FUNC_ID);
				}

				m_integrator = (integrator_t)KeywordList::Enumeration("SolverParameter::integrator_t", solverTypeName);
			}
			else if (ename == "LESSolver") {
				std::string solverTypeName = e->GetText();

				if (! KeywordList::KeywordExists("SolverParameter::lesSolver_t", solverTypeName ) ) {
					throw IBK::Exception( IBK::FormatString(XML_READ_ERROR).arg(e->Row()).arg(
						IBK::FormatString("Unknown LESSolver '%1'.").arg(solverTypeName)
						), FUNC_ID);
				}

				m_lesSolver = (lesSolver_t)KeywordList::Enumeration("SolverParameter::lesSolver_t", solverTypeName);
			}
			else if (ename == "Preconditioner") {
				std::string name = e->GetText();

				if (! KeywordList::KeywordExists("SolverParameter::precond_t", name ) ) {
					throw IBK::Exception( IBK::FormatString(XML_READ_ERROR).arg(e->Row()).arg(
						IBK::FormatString("Unknown Preconditioner '%1'.").arg(name)
						), FUNC_ID);
				}

				m_preconditioner = (precond_t)KeywordList::Enumeration("SolverParameter::precond_t", name);
			}
			else if (ename == "IBK:Parameter") {
				// use utility function to read parameter
				std::string namestr, unitstr;
				double value;
				TiXmlElement::readIBKParameterElement(e, namestr, unitstr, value);

				// determine type of parameter
				if (! KeywordList::KeywordExists("SolverParameter::para_t", namestr ) ) {
					throw IBK::Exception( IBK::FormatString(XML_READ_ERROR).arg(e->Row()).arg(
						IBK::FormatString("Unknown parameter '%1'.").arg(namestr)
						), FUNC_ID);
				}

				para_t t = (para_t)KeywordList::Enumeration("SolverParameter::para_t", namestr);
				if (!unitstr.empty()) {
					m_para[t].set(namestr, value, unitstr);

					// check parameter unit
					std::string paraUnit = KeywordList::Unit("SolverParameter::para_t", t);
					if (unitstr != paraUnit) {
						try {
							m_para[t].get_value(paraUnit);
						}
						catch (IBK::Exception &ex) {
							throw IBK::Exception(ex, IBK::FormatString(XML_READ_ERROR).arg(e->Row()).arg(
								IBK::FormatString("Invalid unit '#%1' of parameter #%2!")
								.arg(paraUnit)
								.arg(namestr)
								), FUNC_ID);
						}
					}
				}
				else
					m_para[t].set(namestr, value);
			}
			else if (ename == "IBK:Flag") {
				// use utility function to read flag
				std::string namestr;
				std::string flag;
				TiXmlElement::readSingleAttributeElement(e, "name", namestr, flag);

				// determine type of flag
				if (! KeywordList::KeywordExists("SolverParameter::flag_t", namestr ) ) {
					throw IBK::Exception( IBK::FormatString(XML_READ_ERROR).arg(e->Row()).arg(
						IBK::FormatString("Unknown flag '%1'.").arg(namestr)
						), FUNC_ID);
				}

				flag_t t = (flag_t)KeywordList::Enumeration("SolverParameter::flag_t", namestr);
				m_flag[t].set(namestr, (flag == "true" || flag == "1"));
			}
			else {
				throw IBK::Exception( IBK::FormatString(XML_READ_ERROR).arg(e->Row()).arg(
					IBK::FormatString("Unknown xml tag '%1'.").arg(ename)
					), FUNC_ID);
			}
		}
	}
	catch (IBK::Exception & ex) {
		throw IBK::Exception(ex, IBK::FormatString("Error reading 'SolverParameter' element."), FUNC_ID);
	}
	catch (std::exception & ex2) {
		throw IBK::Exception(IBK::FormatString("%1\nError reading 'SolverParameter' element.").arg(ex2.what()), FUNC_ID);
	}
}

void SolverParameter::writeXML(TiXmlElement * parent, bool detailedOutput) const {
	SolverParameter tmp;
	tmp.initDefaults();
	if (*this == tmp && !detailedOutput)
		return;

	TiXmlComment::addComment(parent,
		"Solver settings.");
	TiXmlElement * e = new TiXmlElement("SolverParameter");
	parent->LinkEndChild(e);

	// add sub-elements
	if (m_integrator != NUM_I)
		TiXmlElement::appendSingleAttributeElement(e, "Integrator",
												   NULL, std::string(),
												   KeywordList::Keyword("SolverParameter::integrator_t", m_integrator));
	if (m_lesSolver != NUM_LES)
		TiXmlElement::appendSingleAttributeElement(e, "LESSolver",
												   NULL, std::string(),
												   KeywordList::Keyword("SolverParameter::lesSolver_t", m_lesSolver));
	if (m_preconditioner != NUM_PRE)
		TiXmlElement::appendSingleAttributeElement(e, "Preconditioner",
												   NULL, std::string(),
												   KeywordList::Keyword("SolverParameter::precond_t", m_preconditioner));

	// write parameter
	for (unsigned int i=0; i<NUM_SP; ++i) {
		if (m_para[i].name.empty()) continue;
		// don't write is value matches default
		if (detailedOutput)
			TiXmlComment::addComment(e,KeywordList::Description("SolverParameter::para_t",i));
		if (m_para[i] != tmp.m_para[i])
			TiXmlElement::appendIBKParameterElement(e, m_para[i].name, m_para[i].IO_unit.name(), m_para[i].get_value());
	}
	// write flags
	for (int i=0; i<NUM_SF; ++i) {
		if (m_flag[i].name().empty()) continue;
		if (detailedOutput)
			TiXmlComment::addComment(e,KeywordList::Description("SolverParameter::flag_t",i));
		if (m_flag[i] != tmp.m_flag[i])
			TiXmlElement::appendSingleAttributeElement(e, "IBK:Flag", "name", m_flag[i].name(),
													   m_flag[i].isEnabled() ? "true" : "false");
	}

	TiXmlComment::addSeparatorComment(parent);
}


bool SolverParameter::operator!=(const SolverParameter & other) const {
	if (m_integrator != other.m_integrator) return true;
	if (m_lesSolver != other.m_lesSolver) return true;
	if (m_preconditioner != other.m_preconditioner) return true;

	for (unsigned int i=0; i<NUM_SP; ++i)
		if (m_para[i] != other.m_para[i])
			return true;

	for (unsigned int i=0; i<NUM_SF; ++i)
		if (m_flag[i] != other.m_flag[i])
			return true;

	return false; // this and other hold the same data
}


} // namespace NANDRAD

