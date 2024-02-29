#ifndef SVExportPluginInterfaceH
#define SVExportPluginInterfaceH

#include "SVCommonPluginInterface.h"

class QWidget;

/*! Interface for a plugin that populates VICUS project data from some
	external source, typically by reading some external BIM data format and
	converting the data (interactively) to VICUS data.
*/
class SVExportPluginInterface : public SVCommonPluginInterface {
public:

	/*! Returns the already-translated menu caption to be inserted into the File->Export menu. */
	virtual QString exportMenuCaption() const = 0;

	/*! This is the central Export function, that is executed when the user
		selects the respective menu action.
		\param parent Parent widget pointer, to be used as parent for modal dialogs.
		\param projectText The VICUS project data as xml text to be populated.

		\return Returns true if the Export was succcessful and SIM-VICUS shall use the populated VICUS-project
			data (either as new project or merged into/added to the existing project). If false is returned,
			the Export is considered to be aborted by user or through some error.

		\note A well-behaving plugin will clear any allocated resources when returning from this function.

		\note Do not allow exceptions to leave this function, so wrap everything into a try-catch clause!
	*/
	virtual bool getProject(QWidget * parent, const QString& projectText) = 0;

	virtual bool exportGEGFile(const QString& filename) = 0;
};


#define SVExportPluginInterface_iid "ibk.sim-vicus.Plugin.ExportInterface/1.0"

Q_DECLARE_INTERFACE(SVExportPluginInterface, SVExportPluginInterface_iid)

#endif // SVExportPluginInterfaceH
