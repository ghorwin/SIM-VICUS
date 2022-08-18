#include "VICUS_utilities.h"

namespace VICUS {

// Generates a new unique name in format "basename" or "basename (<nr>)" with increasing numbers until
//	the name no longer exists in set existingNames.
QString uniqueName(const QString & baseName, const std::set<QString> & existingNames) {
	// generate new unique object/surface name
	QString name = baseName.trimmed();
	unsigned int index = 1; // default starting index for counting up

	// remove potentially existing (num) suffix
	if (name.trimmed().endsWith(")") ) {
		unsigned int start = name.lastIndexOf("(");
		unsigned int end = name.indexOf(")", start+1);

		QString numString = name.mid(start+1, end-start-1);
		name = name.left(start-1).trimmed();

		bool isNumber;
		unsigned int tempNum = numString.toUInt(&isNumber);
		if (isNumber)
			index = tempNum; // use this as starting index
	}
	// name is now the name without number and trimmed
	QString trialName = name;
	for (;;++index) {
		// if we did already have an index in the name, already generate the name with index suffix
		if (index != 1)
			trialName = QString("%1 (%2)").arg(name).arg(index);

		// process all existing names and check if we have generated a name not yet in the set
		if (existingNames.find(trialName) == existingNames.end())
			break;
	}
	return trialName;
}


QString camelCase2ReadableString(const std::string & original) {
	QString readableString;
	for (char s: original){
		if (isupper(s))
			readableString += " ";
		readableString += s;
	}
	return readableString;
}

} // namespace VICUS
