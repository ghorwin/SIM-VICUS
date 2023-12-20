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

#include "VICUS_AcousticBoundaryCondition.h"

namespace VICUS {

bool AcousticBoundaryCondition::isValid(const VICUS::Database<AcousticSoundAbsorption> &soundAbsDB) const {
	if (m_id == VICUS::INVALID_ID)
		return false;

        if(m_acousticSoundAbsorptionPartitions.empty()){
		m_errorMsg = "Sound absorption layer count is not valid!";
		return false;
	}

        for(const AcousticSoundAbsorptionPartition &layer : m_acousticSoundAbsorptionPartitions){
		if(!layer.isValid(soundAbsDB)){
			m_errorMsg = "Sound absorption ist not valid!";
			return false;
		}
	}

	return true;
}


QString AcousticBoundaryCondition::htmlDescription(const VICUS::Database<VICUS::AcousticSoundAbsorption> & soundAbsDB) const {
	QString html = "<html><body>";

	if (!isValid(soundAbsDB))
		html += tr("<p><span style=\" color:#a40000;\">Invalid parameter definition found.</span></p>");

	html += tr("<p><b>Parameters:</b></p><ul>");

	std::vector<int> freq{125,250,500,1000,2000,4000};
	std::vector<double> absorption(6,0);


	if(isValid(soundAbsDB)){
		// sum up all absorptions
            for(unsigned int i=0; i<m_acousticSoundAbsorptionPartitions.size(); ++i){
                const AcousticSoundAbsorptionPartition &layer = m_acousticSoundAbsorptionPartitions[i];

			const AcousticSoundAbsorption *soundAbs = soundAbsDB[layer.m_idSoundAbsorption];

			if(soundAbs == nullptr)
				continue;

			for(unsigned int j=0; j<AcousticSoundAbsorption::NUM_SF; ++j)
                            absorption[j] += layer.m_para[AcousticSoundAbsorptionPartition::P_AreaFraction].value * soundAbs->m_soundAbsorption[j];
		}

		html += "<table border=\"0\">";
		html += "<tr><td align=\"left\">Frequency [Hz]</td><td align=\"center\">Absorption [---]</td></tr>";

		for(unsigned int i=0; i<absorption.size(); ++i)
			html += "<tr><td align=\"left\">" + QString::number(freq[i]) + "</td><td align=\"center\">" + QString::number(absorption[i]) + "</td></tr>";

		html += "</table>";

	}

	html += "</body></html>";
	return html;
}


AbstractDBElement::ComparisonResult AcousticBoundaryCondition::equal(const AbstractDBElement *other) const {
	const AcousticBoundaryCondition * otherBC = dynamic_cast<const AcousticBoundaryCondition*>(other);
	if (otherBC == nullptr)
		return Different;

	// check parameters
        for(unsigned int i = 0; i < m_acousticSoundAbsorptionPartitions.size(); i++){
		// check if different?
	}

	// check meta data

	if (m_displayName != otherBC->m_displayName)
		return OnlyMetaDataDiffers;

	return Different;
}


} // namespace VICUS
