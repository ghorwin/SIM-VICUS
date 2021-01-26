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

#include <IBK_UnitList.h>

namespace NANDRAD {

void WindowGlazingSystem::checkParameters() {
	FUNCID(WindowGlazingSystem::checkParameters);

	if (m_id == INVALID_ID)
		return; // disabled/nothing to check

	switch (m_modelType) {
		case MT_Simple : {
			// check thermal transmittance parameter
			m_para[P_ThermalTransmittance].checkedValue("ThermalTransmittance", "W/m2K",
														"W/m2K", 0, true, std::numeric_limits<double>::max(), true,
														"Thermal transmittance must be >= 0 W/m2K.");

			// we need an SHGC value; in the next call the input spline will be converted to base units,
			// i.e. xUnit will be "Rad" and y-Unit will be "---".
			m_splinePara[SP_SHGC].checkAndInitialize("SHGC", IBK::Unit("Rad"), IBK::Unit("---"),
													 IBK::Unit("---"), 0, true, 1, true,
													 "SGGC values must be in the range of 0..1.");

		} break;

		case MT_Detailed : {
			// TODO

		} break;

		case NUM_MT :
			throw IBK::Exception("Missing/invalid model type.", FUNC_ID);
	} // switch

}


void WindowGlazingSystem::computeSolarFluxDensity(double qDir, double qDiff, double incidenceAngle,
											 double &solarLeft, double &solarRight,
											 bool solarLoadsLeft) const
{
	solarLeft = 0;
	solarRight= 0;

	// for simple model we calculate solar flux
	switch (m_modelType) {
		case MT_Simple : {
			// we have an SHGC value and the values were checked already
			// Note: SHGC spline x and y value vectors are converted to base SI units (rad, ---).
			double SHGC = m_splinePara[NANDRAD::WindowGlazingSystem::SP_SHGC].m_values.value(incidenceAngle);
			double SHGCHemis = m_splinePara[NANDRAD::WindowGlazingSystem::SP_SHGC].m_values.value(0.0);
			// calculate flux density from left to right [W/m2]
			solarLeft = SHGC * qDir + SHGCHemis * qDiff;
			// if not from left, invert sign
			if (!solarLoadsLeft)
				solarLeft = -solarLeft;
			// we assume thermal equilibrium
			solarRight = -solarLeft;
		} break;

		case MT_Detailed : {
			// TODO

		} break;

		case NUM_MT: ; // just to make the compiler happy
	}
	// otherwise we ignore solar flux
}


void WindowGlazingSystem::computeHeatConductionFluxDensity(double deltaT, double alphaLeft, double alphaRight,
														   double &heatCondLeft, double &heatCondRight) const {

	heatCondLeft = 0;
	heatCondRight = 0;

	switch (m_modelType) {
		case MT_Simple : {
			// simple model:
			// values were checked already
			double alpha = m_para[NANDRAD::WindowGlazingSystem::P_ThermalTransmittance].value;
			// non-implemented model or inactive heat conduction
			if (alpha > 0.0) {

				// compute mean resistance
				double thermalResistance = 1/alpha;

				// add surface resistances
				if (alphaLeft > 0)
					thermalResistance += 1.0/alphaLeft;
				if (alphaRight > 0)
					thermalResistance += 1.0/alphaRight;

				// heat conduction density from left to right (A to B)
				double heatCondDensity = 1/thermalResistance * deltaT;

				// flux towards window surface
				heatCondLeft = heatCondDensity;
				// we assume thermal equilibrium
				heatCondRight = -heatCondLeft;
			}
		} break;

		case MT_Detailed : {
			// TODO

		} break;

		case NUM_MT: ; // just to make the compiler happy
	}

	// otherwise ignore heat transfer
}


} // namespace NANDRAD

