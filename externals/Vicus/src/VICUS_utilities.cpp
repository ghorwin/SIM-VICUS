#include "VICUS_utilities.h"

namespace VICUS {

/*! Generates a new unique name in format "basename" or "basename [<nr>]" with increasing numbers until
	the name no longer exists in set existingNames.
*/
QString uniqueName(const QString & baseName, const std::set<QString> & existingNames) {
	// generate new unique object/surface name
	unsigned int count = 1;
	QString name = baseName;
	QString tempBaseName = baseName;
	unsigned int index = 0;

	if ( baseName.trimmed().endsWith(")") ) {
		unsigned int start = baseName.lastIndexOf("(");
		unsigned int end = baseName.indexOf(")", start+1);

		tempBaseName = baseName.left(start-1).trimmed();
		bool isNumber;
		QString numString = baseName.mid(start+1, end-start-1);
		unsigned int tempNum = baseName.mid(start+1, end-1).toUInt(&isNumber);

		if(isNumber)
			index = tempNum;
	}

	for (;;) {
		// process all surfaces and check if we have already a new surface with our current name
		if (existingNames.find(name) == existingNames.end())
			break;
		name = QString("%1 (%2)").arg(tempBaseName).arg(/*index +*/ ++count);
	}
	return name;
}

} // namespace VICUS
