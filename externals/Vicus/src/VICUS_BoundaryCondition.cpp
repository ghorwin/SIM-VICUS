#include "VICUS_BoundaryCondition.h"

namespace VICUS {

bool BoundaryCondition::isValid() const {
	if (m_id == VICUS::INVALID_ID)
		return false;

	try {
		m_heatConduction.checkParameters();
		m_longWaveEmission.checkParameters();
		m_solarAbsorption.checkParameters();
	} catch (...) {
		return false;
	}

	return true;
}


QString BoundaryCondition::htmlDescription() const {
	QString html = "<html><body>";

	if (!isValid())
		html += tr("<p><span style=\" color:#a40000;\">Invalid parameter definition found.</span></p>");

	html += tr("<p><b>Parameters:</b></p><ul>");
	if (m_heatConduction.m_modelType != NANDRAD::InterfaceHeatConduction::NUM_MT) {
		QString heatCondInfo;
		switch (m_heatConduction.m_modelType) {
			case NANDRAD::InterfaceHeatConduction::MT_Constant:
				heatCondInfo = tr("Constant, heat transfer coefficient = %1 W/m2K").arg(m_heatConduction.m_para[NANDRAD::InterfaceHeatConduction::P_HeatTransferCoefficient].get_value("W/m2K"));
			break;
			case NANDRAD::InterfaceHeatConduction::NUM_MT: break;
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

	//first check critical data

	//check parameters

	if(m_heatConduction != otherBC->m_heatConduction ||
			m_longWaveEmission != otherBC->m_longWaveEmission ||
			m_solarAbsorption != otherBC->m_solarAbsorption ||
			m_vaporDiffusion != otherBC->m_vaporDiffusion ||
			m_airFlow != otherBC->m_airFlow)
		return Different;

	//check meta data

	if(m_displayName != otherBC->m_displayName ||
			m_color != otherBC->m_color)
		return OnlyMetaDataDiffers;

	return Equal;
}


} // namespace VICUS
