#ifndef SVDatabasePluginInterfaceH
#define SVDatabasePluginInterfaceH

#include <QString>

#include "SVCommonPluginInterface.h"

class QWidget;
class SVDatabase;

/*! Interface for a plugin that provides VICUS database elements. */
class SVDatabasePluginInterface : public SVCommonPluginInterface {
public:

	/*! This function needs to be implemented by the database plugin to populate the database with its own data.
		The function will get two database arguments. The first is the current database in the application (that
		may already been augmented by some other plugin). The second is the writable db object, which is always
		a copy of the current DB, where the plugin can add data to.

		\param currentDB Reference to the current database in SIM-VICUS
		\param augmentedDB Database object to be modified by the plugin.
		\return Returns true, if plugin successfully modified the database. If false is returned, some error occurred
			and the user interface silently ignores the augmentedDB variable.

		\note The SIM-VICUS user-interface will prevent the plugin from removing/altering already existing data. So, if the plugin
			(by programming error or by design) removes existing data from the database, such changes will be silently
			discarded.
	*/
	virtual bool retrieve(const SVDatabase & currentDB, SVDatabase & augmentedDB) = 0;
};

#endif // SVDatabasePluginInterfaceH
