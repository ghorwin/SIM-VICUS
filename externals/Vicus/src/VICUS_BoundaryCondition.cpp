/*	The SIM-VICUS data model library.

	Copyright (c) 2020-today, Institut für Bauklimatik, TU Dresden, Germany

	Primary authors:
	  Andreas Nicolai  <andreas.nicolai -[at]- tu-dresden.de>
	  Dirk Weiss  <dirk.weiss -[at]- tu-dresden.de>
	  Stephan Hirth  <stephan.hirth -[at]- tu-dresden.de>
	  Hauke Hirsch  <hauke.hirsch -[at]- tu-dresden.de>

	  ... all the others from the SIM-VICUS team ... :-)

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

#include "VICUS_BoundaryCondition.h"

namespace VICUS {

bool BoundaryCondition::isValid(const Database<Schedule> & scheduleDB) const {
	if (m_id == VICUS::INVALID_ID)
		return false;

	if(!m_heatConduction.isValid(scheduleDB)) {
		m_errorMsg = "Heat conduction is not valid.";
		return false;
	}

	try {
		m_longWaveEmission.checkParameters();
		// TODO : add vapor diffusion/air flow once needed
	} catch (...) {
		m_errorMsg = "Long wave emission is not valid.";
		return false;
	}
	try {
		m_solarAbsorption.checkParameters();
	} catch (...) {
		m_errorMsg = "Solar absorption is not valid.";
		return false;
	}

	return true;
}

bool BoundaryCondition::hasSetpointTemperatureForZone() const {
	if(m_heatConduction.m_otherZoneType == InterfaceHeatConduction::OZ_Scheduled ||
			m_heatConduction.m_otherZoneType == InterfaceHeatConduction::OZ_Constant)
		return true;
	return false;
}


QString BoundaryCondition::htmlDescription(const VICUS::Database<Schedule> & scheduleDB) const {
	QString html = "<html><body>";

	if (!isValid(scheduleDB))
		html += tr("<p><span style=\" color:#a40000;\">Invalid parameter definition found.</span></p>");

	html += tr("<p><b>Parameters:</b></p><ul>");
	if (m_heatConduction.m_modelType != VICUS::InterfaceHeatConduction::NUM_MT) {
		QString heatCondInfo;
		switch (m_heatConduction.m_modelType) {
			case VICUS::InterfaceHeatConduction::MT_Constant:
				heatCondInfo = tr("Constant, heat transfer coefficient = %1 W/m2K").arg(m_heatConduction.m_para[NANDRAD::InterfaceHeatConduction::P_HeatTransferCoefficient].get_value("W/m2K"));
			break;
			case VICUS::InterfaceHeatConduction::NUM_MT: break;
		}
		html += tr("<li><i>Heat conduction</i><br>%1</li>").arg(heatCondInfo);
	}
	if (m_solarAbsorption.m_modelType != NANDRAD::InterfaceSolarAbsorption::NUM_MT) {
		QString info;
		switch (m_solarAbsorption.m_modelType) {
			case NANDRAD::InterfaceSolarAbsorption::NUM_MT: break;
			case NANDRAD::InterfaceSolarAbsorption::MT_Constant:
				info = tr("Constant, adsorption coeff. = %1").arg(m_solarAbsorption.m_para[NANDRAD::InterfaceSolarAbsorption::P_AbsorptionCoefficient].value);
			break;
		}
		html += tr("<li><i>Solar adsorption</i><br>%1</li>").arg(info);
	}
	if (m_longWaveEmission.m_modelType != NANDRAD::InterfaceLongWaveEmission::NUM_MT) {
		QString info;
		switch (m_longWaveEmission.m_modelType) {
			case NANDRAD::InterfaceLongWaveEmission::NUM_MT: break;
			case NANDRAD::InterfaceLongWaveEmission::MT_Constant:
				info = tr("Constant, emissivity = %1").arg(m_longWaveEmission.m_para[NANDRAD::InterfaceLongWaveEmission::P_Emissivity].value);
			break;
		}
		html += tr("<li><i>Long wave radiation exchange</i><br>%1</li>").arg(info);
	}

	html += "</body></html>";
	return html;
}


AbstractDBElement::ComparisonResult BoundaryCondition::equal(const AbstractDBElement *other) const {
	const BoundaryCondition * otherBC = dynamic_cast<const BoundaryCondition*>(other);
	if (otherBC == nullptr)
		return Different;

	// check parameters

	if (m_heatConduction != otherBC->m_heatConduction ||
			m_longWaveEmission != otherBC->m_longWaveEmission ||
			m_solarAbsorption != otherBC->m_solarAbsorption ||
			m_vaporDiffusion != otherBC->m_vaporDiffusion ||
			m_airFlow != otherBC->m_airFlow)
		return Different;

	// check meta data

	if (m_displayName != otherBC->m_displayName)
		return OnlyMetaDataDiffers;

	return Equal;
}


} // namespace VICUS
