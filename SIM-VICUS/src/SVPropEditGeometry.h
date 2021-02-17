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

	/*! Sets all states of tool buttons and updates comboBox, also calls updateInputs().
		\param type - specifies the type of which is toggled
		\param state - specifies the modification state
	*/
	void setState(const ModificationType &type, const ModificationState &state);

	/*! Depending on currently selected modification type and state, the line edits and labels are updated accordingly. */
	void updateInputs();

	/*! Checks/unchecks the tool buttons depending on the specified type.
		Has no side-effects.
	*/
	void setToolButton(const ModificationType &type);

	/*! Sets the items of the comboBox */
	void setComboBox(const ModificationType &type, const ModificationState &state);

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
		\param copyVec - Translation Vector
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

	void on_lineEditCopyX_editingFinished();

	void on_lineEditCopyY_editingFinished();

	void on_lineEditCopyZ_editingFinished();

	/*! Triggered when anything changes in one of the line edits X, Y or Z */
	void onLineEditTextChanged(QtExt::ValidatingLineEdit * lineEdit);

	void on_lineEditX_textChanged(const QString &);
	void on_lineEditY_textChanged(const QString &);
	void on_lineEditZ_textChanged(const QString &);

private:
	/*! Updates the property widget regarding to all geometry data.
		This function is called whenever the selection has changed, and when surface geometry (of selected surfaces)
		has changed.

		This function switches also between AddGeometry and EditGeometry mode, when first selection is made or
		everything is deselected.
	*/
	void update();

	/*! Increases/decreases value in line edit depending on scroll wheel. */
	void onWheelTurned(double offset, QtExt::ValidatingLineEdit * lineEdit);

	/*! Identifies which transformation operation is currently selected and is updated,
		whenever an operation button is toggled.
	*/
	ModificationType					m_modificationType = MT_Translate;

	/*! For each transformation operation we cache the current mode choice from the combo box.
		The value is updated when user changes the combo-box, and when operation is changed,
		the value is used to update the combo box's current index.
	*/
	ModificationState					m_modificationState[NUM_MT];

	/*! Contains position and rotation of local coordinate system object. */
	Vic3D::Transform3D					m_localCoordinatePosition;

	double								m_orientation = 0.0;
	double								m_inclination = 0.0;

	/*! Cached initial values for translation line edits. */
	QVector3D							m_transValue;

	// storing all the rotation values
	double								m_xRotaValue = 0.0;
	double								m_yRotaValue = 0.0;
	double								m_zRotaValue = 0.0;

	/*! This is the dimension of the bounding box (dx, dy, dz). */
	IBKMK::Vector3D						m_scaleValue;

	// storing all the copiing values
	double								m_xCopyValue = 0.0;
	double								m_yCopyValue = 0.0;
	double								m_zCopyValue = 0.0;

	// storing the original measurements for scaling if wheel event is used
	std::vector<VICUS::Surface>			m_relScaleSurfaces;

	Ui::SVPropEditGeometry				*m_ui;
};

#endif // SVPropEditGeometryH
