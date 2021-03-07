#include "VICUS_ZoneTemplate.h"



namespace VICUS {

bool ZoneTemplate::isValid() const {

	return true;
}


unsigned int ZoneTemplate::subTemplateCount() const {

	unsigned int count = 0;
	for (int i=0; i<NUM_ST; ++i)
		if (!m_idReferences[i].name.empty()) ++count;
	return count;

}


ZoneTemplate::SubTemplateType ZoneTemplate::usedReference(unsigned int index) const {
	int count = -1;
	int i=0;
	for (; i<NUM_ST && count < (int)index; ++i) {
		// increase count for each used id reference
		if (!m_idReferences[i].name.empty())
			++count;
	}
	return (ZoneTemplate::SubTemplateType)i; // if index > number of used references, we return NUM_ST here
}



} // namespace VICUS
