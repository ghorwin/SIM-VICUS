#include "VICUS_AcousticSoundAbsorption.h"
#include "tinyxml.h"
#include <IBK_math.h>

namespace VICUS {

bool AcousticSoundAbsorption::isValid() const {

	for (unsigned int i=0; i<NUM_SF; ++i){
		if(m_soundAbsorption[(SoundAbsorptionFrequency)i] < 0)
			return false;
	}

	return true;
}

AbstractDBElement::ComparisonResult AcousticSoundAbsorption::equal(const AbstractDBElement *other) const
{
	const AcousticSoundAbsorption * otherSA = dynamic_cast<const AcousticSoundAbsorption*>(other);
	if(otherSA == nullptr)
		return Different;

	for(unsigned int i = 0; i < SoundAbsorptionFrequency::NUM_SF; ++i) {
		if(IBK::near_equal(m_soundAbsorption[(SoundAbsorptionFrequency)i],
						   otherSA->m_soundAbsorption[(SoundAbsorptionFrequency)i]))
			return Different;
	}

	if(m_displayName != otherSA->m_displayName)
		return AbstractDBElement::OnlyMetaDataDiffers;

	return Equal;
}

void AcousticSoundAbsorption::readXML(const TiXmlElement * element) {
	FUNCID(AcousticComponent::readXML);

	readXMLPrivate(element);

	// Now read the reverberation times and fill in
	// static array
	try {
		// search for mandatory elements
		// reading elements
		const TiXmlElement * c = element->FirstChildElement();
		while (c) {
			const std::string & cName = c->ValueStr();
			if (cName == "SoundAbsorption") {
				// read vertexes
				std::string text = c->GetText();
				text = IBK::replace_string(text, ",", " ");
				std::vector<double> vals;
				try {
					IBK::string2valueVector(text, vals);
					// must have n*2 elements
					if (vals.size() != NUM_SF)
						throw IBK::Exception("Mismatching number of values.", FUNC_ID);
					if (vals.empty())
						throw IBK::Exception("Missing values.", FUNC_ID);

					for (unsigned int i=0; i<NUM_SF; ++i){
						m_soundAbsorption[i] = vals[i];
					}

				} catch (IBK::Exception & ex) {
					throw IBK::Exception( ex, IBK::FormatString(XML_READ_ERROR).arg(element->Row()).arg(
												 IBK::FormatString("Error reading vector element '%1'.").arg("SoundAbsorption") ), FUNC_ID);
				}
			}
			c = c->NextSiblingElement();
		}
	}
	catch (IBK::Exception & ex) {
		throw IBK::Exception( ex, IBK::FormatString("Error reading 'AcousticSoundAbsorption' element."), FUNC_ID);
	}
	catch (std::exception & ex2) {
		throw IBK::Exception( IBK::FormatString("%1\nError reading 'AcousticSoundAbsorption' element.").arg(ex2.what()), FUNC_ID);
	}

}


TiXmlElement * AcousticSoundAbsorption::writeXML(TiXmlElement * parent) const {
	TiXmlElement * e = writeXMLPrivate(parent);

	TiXmlElement * childElement = new TiXmlElement("SoundAbsorption");
	e->LinkEndChild(childElement);

	std::stringstream vals;
	for (unsigned int i=0; i<NUM_SF; ++i) {
		vals << m_soundAbsorption[i];
		if (i < NUM_SF-1)
			vals << ", ";
	}
	TiXmlText * text = new TiXmlText( vals.str() );
	childElement->LinkEndChild( text );
	return e;
}


} // namespace VICUS
