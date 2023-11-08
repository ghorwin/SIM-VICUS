#ifndef SVZONEINFORMATIONDIALOG_H
#define SVZONEINFORMATIONDIALOG_H

#include <QDialog>

#include "VICUS_Project.h"

namespace Ui {
class SVZoneInformationDialog;
}

/*! A Dialog that shows additional zone information.

*/
class SVZoneInformationDialog : public QDialog {
	Q_OBJECT

	enum SurfaceColumns {
		ColID,
		ColSurfaceName,
		ColArea,
		ColComponentName,
		ColConstructionName,
		ColBoundaryConditionSiteA,
		ColBoundaryConditionSiteB,
		NumCol
	};

public:
	explicit SVZoneInformationDialog(QWidget *parent = nullptr);
	~SVZoneInformationDialog();

	static void showZoneInformation(const QString & title, const VICUS::Project &prj, unsigned int zoneId);

private:
	Ui::SVZoneInformationDialog *m_ui;
};

#endif // SVZONEINFORMATIONDIALOG_H
