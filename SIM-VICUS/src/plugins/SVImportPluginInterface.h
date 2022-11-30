#ifndef SVImportPluginInterfaceH
#define SVImportPluginInterfaceH

#include "SVCommonPluginInterface.h"

class QWidget;

/*! Interface for a plugin that populates VICUS project data from some
	external source, typically by reading some external BIM data format and
	converting the data (interactively) to VICUS data.
*/
class SVImportPluginInterface : public SVCommonPluginInterface {
public:

	/*! Returns the already-translated menu caption to be inserted into the File->Import menu. */
	virtual QString importMenuCaption() const = 0;

	/*! This is the central import function, that is executed when the user
		selects the respective menu action.
		\param parent Parent widget pointer, to be used as parent for modal dialogs.
		\param p The VICUS project data as text.

		\return Returns true if the import was succcessful and SIM-VICUS shall use the populated VICUS-project
			data (either as new project or merged into/added to the existing project). If false is returned,
			the import is considered to be aborted by user or through some error.

		\note A well-behaving plugin will clear any allocated resources when returning from this function.

		\note Do not allow exceptions to leave this function, so wrap everything into a try-catch clause!
	*/
	virtual bool import(QWidget * parent, QString & p) = 0;
};


#define SVImportPluginInterface_iid "ibk.sim-vicus.Plugin.ImportInterface/1.0"

Q_DECLARE_INTERFACE(SVImportPluginInterface, SVImportPluginInterface_iid)

#endif // SVImportPluginInterfaceH
