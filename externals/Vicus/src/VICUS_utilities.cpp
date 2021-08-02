#include "VICUS_utilities.h"

namespace VICUS {

/*! Generates a new unique name in format "basename" or "basename [<nr>]" with increasing numbers until
	the name no longer exists in set existingNames.
*/
QString uniqueName(const QString & baseName, const std::set<QString> & existingNames) {
	// generate new unique object/surface name
	unsigned int count = 1;
	QString name = baseName;
	for (;;) {
		// process all surfaces and check if we have already a new surface with our current name
		if (existingNames.find(name) == existingNames.end())
			break;
		name = QString("%1 [%2]").arg(baseName).arg(++count);
	}
	return name;
}

} // namespace VICUS
