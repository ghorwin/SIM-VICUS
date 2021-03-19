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
				heatCondInfo = tr("Constant, alpha = %1 W/m2K").arg(m_heatConduction.m_para[NANDRAD::InterfaceHeatConduction::P_HeatTransferCoefficient].get_value("W/m2K"));
			break;
			case NANDRAD::InterfaceHeatConduction::NUM_MT: break;
		}
		html += tr("<li>Heat conduction: %1</li>").arg(heatCondInfo);
	}

	html += "</body></html>";
	return html;
}


} // namespace VICUS
