#ifndef SVPropEditGeometryH
#define SVPropEditGeometryH

#include <QWidget>

#include <QtExt_ValidatingLineEdit.h>

#include <IBKMK_Vector3D.h>

#include <VICUS_Surface.h>

#include <Vic3DTransform3D.h>

namespace Vic3D {
class Transform3D;
}

namespace VICUS {
class Project;
}

namespace Ui {
class SVPropEditGeometry;
}

class QLineEdit;

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

	enum ModificationType {
		MT_Translate,
		MT_Rotate,
		MT_Scale,
		NUM_MT
	};

	enum ModificationState {
		MS_Absolute,
		MS_Relative,
		MS_Local,
		NUM_MS
	};

	explicit SVPropEditGeometry(QWidget *parent = nullptr);
	~SVPropEditGeometry();

	/*! Sets the current tab index to the TabState specified
	*/
	void setCurrentTab(const TabState &state);

	/*! Sets the Coordinates of the Cente
class QtExt_ValidatingLineEdit;r Point of the local
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

	/*! Sets all states of tool buttons and comboBox
		\param ModificationType - specifies the type of which is toggled
		\param ModificationState - specifies the modification state
		\param updateComboxo - if true updates the combox box to the specifies type
	*/
	void setState(const ModificationType &type, const ModificationState &state,
				  const bool &updateComboBox = false);

	/*! Set up the tool button to the specified type */
	void setToolButton(const ModificationType &type);

	/*! Sets the items of the comboBox */
	void setComboBox(const ModificationType &type, const ModificationState &state);

	/*! Sets the currently selected surfaces in a vector with temporary copy of selected surfaces
		Allows to use wheel event in relative mode
	*/
	void setRelativeScalingSurfaces();

	/*! Shows all lineEdit/Label fiels that are necessary to sho absolute rotation */
	void showDeg(const bool &show=true);

	/*! Show the specified rotation/orientation of the selected surfaces */
	void showRotation(const bool &abs=true);

	/*! Translates the selected surfaces with the specified
		\param transVec - Translation Vector
	*/
	void translate(const QVector3D & transVec, const ModificationState &state);

	/*! Scales the selected surfaces with the specified
		\param scaleVec - Translation Vector
	*/
	void scale(const QVector3D & scaleVec, const ModificationState &state, const bool &wheel = false);

	/*! Translates the selected surfaces with the specified
		\param rotaVec - Translation Vector
	*/
	void rotate(const QVector3D & rotaVec, const ModificationState &state);

	/*! Translates the selected surfaces with the specified
		\param transVec - Translation Vector
	*/
	void copy(const QVector3D & transVec);

public slots:

	/*! Connected to SVProjectHandler::modified() */
	void onModified(int modificationType, ModificationInfo * );


private slots:

	void on_pushButtonAddPolygon_clicked();

	void on_pushButtonAddRect_clicked();

	void on_pushButtonAddZoneBox_clicked();



	void on_lineEditX_editingFinished();

	void on_lineEditY_editingFinished();

	void on_lineEditZ_editingFinished();


	void on_comboBox_activated(int index);


	bool eventFilter(QObject *target, QEvent *event) override;

	void on_lineEditX_returnPressed();

	void on_lineEditY_returnPressed();

	void on_lineEditZ_returnPressed();


	void on_toolButtonTrans_clicked();

	void on_toolButtonRotate_clicked();

	void on_toolButtonScale_clicked();


	void on_lineEditOrientation_returnPressed();

	void on_lineEditInclination_returnPressed();


	void on_lineEditOrientation_editingFinished();

	void on_lineEditInclination_editingFinished();



	void on_lineEditCopyX_returnPressed();

	void on_lineEditCopyY_returnPressed();

	void on_lineEditCopyZ_returnPressed();

private:
	/*! Updates the property widget regarding to all geometry data
		Takes a vector of pointers to all selected surfaces
	*/
	void update(const bool &updateScalingSurfaces);

	ModificationType					m_modificationType = MT_Translate;
	ModificationState					m_modificationState[NUM_MT];

	Vic3D::Transform3D					m_localCoordinatePosition;

	// holding all the values
	double								m_xValue[NUM_MT];
	double								m_yValue[NUM_MT];
	double								m_zValue[NUM_MT];

	double								m_orientation = 0.0;
	double								m_inclination = 0.0;

	// storing all the translation values
	double								m_xTransValue = 0.0;
	double								m_yTransValue = 0.0;
	double								m_zTransValue = 0.0;

	// storing all the rotation values
	double								m_xRotaValue = 0.0;
	double								m_yRotaValue = 0.0;
	double								m_zRotaValue = 0.0;

	// storing all the scaling values
	double								m_xScaleValue = 0.0;
	double								m_yScaleValue = 0.0;
	double								m_zScaleValue = 0.0;

	// storing the original measurements for scaling if wheel event is used
	std::vector<VICUS::Surface>	m_relScaleSurfaces;

	Ui::SVPropEditGeometry				*m_ui;
};

#endif // SVPropEditGeometryH
