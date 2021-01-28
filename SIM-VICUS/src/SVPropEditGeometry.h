#ifndef SVPropEditGeometryH
#define SVPropEditGeometryH

#include <QWidget>

#include <IBKMK_Vector3D.h>

#include <VICUS_Surface.h>

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
class ModificationInfo;


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
		if absolute mode is clicked in scale groupbox
	*/
	void setBoundingBox(const IBKMK::Vector3D &v);

	/*! Sets the Rotation and Inclination of the selected surfaces
		if more then one surface is selected it takes the z-value of the local coordinate system
		as normal
	*/
	void setRotation(const IBKMK::Vector3D &normal);

public slots:

	/*! Connected to SVProjectHandler::modified() */
	void onModified(int modificationType, ModificationInfo * );


private slots:

	void on_pushButtonAddPolygon_clicked();

	void on_pushButtonAddRect_clicked();

	void on_pushButtonAddZoneBox_clicked();

	void on_pushButtonTranslate_clicked();

	void on_pushButtonScale_clicked();

	void on_pushButtonRotate_clicked();

	void on_radioButtonScaleAbsolute_toggled(bool absScale);

	void on_radioButtonRotateAbsolute_toggled(bool absRotate);

	void on_radioButtonAbsolute_toggled(bool checked);

private:
	/*! Updates the property widget regarding to all geometry data
		Takes a vector of pointers to all selected surfaces
	*/
	void update();

	Ui::SVPropEditGeometry *m_ui;
};

#endif // SVPropEditGeometryH
