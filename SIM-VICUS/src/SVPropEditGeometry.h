#ifndef SVPropEditGeometryH
#define SVPropEditGeometryH

#include <QWidget>

#include <IBKMK_Vector3D.h>

namespace Vic3D {
class Transform3D;
}

namespace VICUS {
class Project;
}

namespace Ui {
class SVPropEditGeometry;
}

class SVProjectHandler;
class SVUndoModifySurfaceGeometry;

/*! This widget is shown when the scene is put into geometry editing mode. */
class SVPropEditGeometry : public QWidget {
	Q_OBJECT


public:
	enum TabState {
		TS_AddGeometry,
		TS_EditGeometry,
		NUM_TS
	};

	explicit SVPropEditGeometry(QWidget *parent = nullptr);
	~SVPropEditGeometry();

	/*! Sets the current tab index to the TabState specified
	*/
	void setCurrentTab(const TabState &state);

	/*! Sets the Coordinates of the Center Point of the local
		Coordinate System
	*/
	void setCoordinates(const Vic3D::Transform3D &t);

	/*! Sets the Bounding Box Measurements of the selected surfaces
	 * if absolute mode is clicked in scale groupbox
	*/
	void setBoundingBox(const IBKMK::Vector3D &v);

private slots:
	void on_pushButtonAddPolygon_clicked();
	void on_pushButtonAddRect_clicked();
	void on_pushButtonAddZoneBox_clicked();

	void on_pushButtonTranslate_clicked();

	void on_pushButtonScale_clicked();

	void on_pushButtonRotate_clicked();

	void on_radioButtonScaleAbsolute_toggled(bool checked);


private:
	Ui::SVPropEditGeometry *m_ui;
};

#endif // SVPropEditGeometryH
