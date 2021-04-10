#include "VICUS_ZoneTemplate.h"



namespace VICUS {

ZoneTemplate::ZoneTemplate() {
	for (int i = 0; i<NUM_ST; ++i)
		m_idReferences[i] = VICUS::INVALID_ID;
}


bool ZoneTemplate::isValid() const {

	// TODO : Implement
	if(m_id ==  INVALID_ID)
		return false;

	///TODO check the sub templates in SV-project with isValid()

	return true;
}


unsigned int ZoneTemplate::subTemplateCount() const {

	unsigned int count = 0;
	for (int i=0; i<NUM_ST; ++i)
		if (m_idReferences[i] != VICUS::INVALID_ID) ++count;
	return count;

}


ZoneTemplate::SubTemplateType ZoneTemplate::usedReference(unsigned int index) const {
	int count = 0;
	int i=0;
	// Example  : index = 0 and we have the first id set
	//            loop 1:   i = 0, count = 1 -> return i=0
	// Example 2: index = 1 and we have the first and third id set:
	//            loop 1:   i = 0, count = 1
	//            loop 2:   i = 1, count = 1
	//            loop 3:   i = 2, count = 2  -> return i=1
	for (; i<NUM_ST; ++i) {
		// increase count for each used id reference
		if (m_idReferences[i] != VICUS::INVALID_ID)
			++count;
		if (count > (int)index)
			break;
	}
	return (ZoneTemplate::SubTemplateType)i; // if index > number of used references, we return NUM_ST here
}



} // namespace VICUS
