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

#include "NANDRAD_WindowGlazingSystem.h"

namespace NANDRAD {

void WindowGlazingSystem::checkParameters() {
	// TODO

}

void WindowGlazingSystem::computeSolarFluxDensity(double qDir, double qDiff, double incidenceAngle,
											 double &solarLeft, double &solarRight,
											 bool solarLoadsLeft) const{
	solarLeft = 0;
	solarRight= 0;

	// for simple model we calculate solar flux
	if(m_modelType == NANDRAD::WindowGlazingSystem::MT_Simple) {
		// we have an SHGC value
		if(!m_splinePara[NANDRAD::WindowGlazingSystem::SP_SHGC].m_name.empty()) {
		   // values were checked already
			double SHGC = m_splinePara[NANDRAD::WindowGlazingSystem::SP_SHGC].m_values.value(incidenceAngle);
			double SHGCHemis = m_splinePara[NANDRAD::WindowGlazingSystem::SP_SHGC].m_values.value(0.0);
			// calculate flux density from left to right
			double solarDensity = 0.0;
			if(solarLoadsLeft) {
				solarDensity = SHGC * qDir + SHGCHemis * qDiff;
			}
			else {
				solarDensity = -(SHGC * qDir + SHGCHemis * qDiff);
			}
			// flux towards window surface
			solarLeft = solarDensity;
			// we assume thermal equilibrium
			solarRight = -solarLeft;
		}
	}
	// otherwise we ignore solar flux
}

void WindowGlazingSystem::computeHeatConductionFluxDensity(double deltaT, double alphaLeft, double alphaRight,
														   double &heatCondLeft, double &heatCondRight) const {

	heatCondLeft = 0;
	heatCondRight = 0;

	// simple model:
	if(m_modelType == NANDRAD::WindowGlazingSystem::MT_Simple) {
		// values were checked already
		double alpha = m_para[NANDRAD::WindowGlazingSystem::P_ThermalTransmittance].value;
		// non-implemented model or inactive heat conduction
		if(alpha > 0.0) {

			// compute mean resistance
			double thermalResistance = 1/alpha;

			// add surface resistances
			if(alphaLeft > 0)
				thermalResistance += 1.0/alphaLeft;
			if(alphaRight > 0)
				thermalResistance += 1.0/alphaRight;

			// compute flux
			IBK_ASSERT(thermalResistance > 0);

			// heat conduction density from left to right (A to B)
			double heatCondDensity = 1/thermalResistance * deltaT;

			// flux towards window surface
			heatCondLeft = heatCondDensity;
			// we assume thermal equilibrium
			heatCondRight = -heatCondLeft;
		}
	}
	// otherwise ignore heat transfer
}


} // namespace NANDRAD

