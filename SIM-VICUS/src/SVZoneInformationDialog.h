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

	/*! Shows Information of current Room. */
	void showZoneInformation(const VICUS::Project &prj, unsigned int zoneId);

private slots:
	void on_lineEditHeatCapacityFinishedSuccessfully();
	void on_lineEditAreaFinishedSuccessfully();
	void on_lineEditVolumeFinishedSuccessfully();

private:
	/*! Pointer to Ui. */
	Ui::SVZoneInformationDialog			*m_ui;

	/*! Current room selection. */
	VICUS::Room							*m_currentRoom = nullptr;

};

#endif // SVZONEINFORMATIONDIALOG_H
