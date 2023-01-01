#ifndef SVDatabasePluginInterfaceH
#define SVDatabasePluginInterfaceH

#include "SVCommonPluginInterface.h"

class QWidget;
class SVDatabase;

/*! Interface for a plugin that provides VICUS database elements. */
class SVDatabasePluginInterface : public SVCommonPluginInterface {
public:

	/*! This function needs to be implemented by the database plugin to populate the database with its own data.
		The function will get two database arguments. The first is the current database in the application (that
		may already been augmented by some other plugin). The second is the writable db object, which is always
		empty at first and may be populated by the DB plugin.

		\param currentDB Reference to the current database in SIM-VICUS; plugin may use this to avoid ID duplication.
		\param additionalDBElemnts Database object populated by the plugin, initially empty, will be added to currentDB.
		\return Returns true, if plugin successfully modified the database. If false is returned, some error occurred
			and the user interface silently ignores the augmentedDB variable.
	*/
	virtual bool retrieve(const SVDatabase & currentDB, SVDatabase & additionalDBElemnts) = 0;
};

#define SVDatabasePluginInterface_iid "ibk.sim-vicus.Plugin.DatabaseInterface/1.0"

Q_DECLARE_INTERFACE(SVDatabasePluginInterface, SVDatabasePluginInterface_iid)

#endif // SVDatabasePluginInterfaceH
