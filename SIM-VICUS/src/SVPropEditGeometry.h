#ifndef SVPropEditGeometryH
#define SVPropEditGeometryH

#include <QWidget>

#include <IBKMK_Vector3D.h>

#include <VICUS_Surface.h>

#include <Vic3DTransform3D.h>

namespace Vic3D {
	class Transform3D;
}

namespace VICUS {
	class Project;
	class Room;
	class Surface;
}

namespace Ui {
	class SVPropEditGeometry;
}

namespace QtExt {
	class ValidatingLineEdit;
}

class QLineEdit;

class SVProjectHandler;
class SVUndoModifySurfaceGeometry;
class ModificationInfo;


/*! This widget is shown when the scene is put into geometry editing mode.

	This widget handles quite a lot of operations, mainly

	a) adding geometry,
	b) modifying geometry,
	c) adding existing geometry again with geometric transformations applied (copy)

	Adding new geometry is done in a separate tab.
	Modifying geometry requires a valid selection - hence, the tab is disabled when there is no selection
*/
class SVPropEditGeometry : public QWidget {
	Q_OBJECT

public:
	enum Operation {
		O_AddGeometry,
		O_EditGeometry
	};

	enum ModificationType {
		MT_Translate,
		MT_Rotate,
		MT_Scale
	};

	enum ModificationState {
		MS_Absolute,
		MS_Relative,
		MS_Local
	};

	explicit SVPropEditGeometry(QWidget *parent = nullptr);
	~SVPropEditGeometry() override;

	/*! Sets the current tab index to the TabState specified */
	void setCurrentPage(const Operation & op);

	/*! Sets the Coordinates of the Center of the local Coordinate System
		(called directly from the local coordinate system when its position changes)
	*/
	void setCoordinates(const Vic3D::Transform3D &t);

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

	/*! Applies current translation (from selected geometry object) into project. */
	void translate();

	/*! Applies current scaling (from selected geometry object) into project. */
	void scale();

	/*! Applies current rotation (from selected geometry object) into project. */
	void rotate();

public slots:

	/*! Connected to SVProjectHandler::modified() */
	void onModified(int modificationType, ModificationInfo * );

	/*! Connected to SVViewStateManager::viewStateChanged() and used to enable/disable
		the edit widget when there is no selection.
	*/
	void onViewStateChanged();

private slots:

	/*! Event Filter: Needed for all scrolling specific inputs */
	bool eventFilter(QObject *target, QEvent *event) override;

	// All push buttons specific functions

	void on_pushButtonAddPolygon_clicked();
	void on_pushButtonAddRect_clicked();
	void on_pushButtonAddZoneBox_clicked();

	// all line edit specific functions

	void on_lineEditX_editingFinished();
	void on_lineEditY_editingFinished();
	void on_lineEditZ_editingFinished();

	void on_lineEditX_returnPressed();
	void on_lineEditY_returnPressed();
	void on_lineEditZ_returnPressed();

	void on_lineEditX_textChanged(const QString &);
	void on_lineEditY_textChanged(const QString &);
	void on_lineEditZ_textChanged(const QString &);


	void on_lineEditOrientation_returnPressed();
	void on_lineEditInclination_returnPressed();

	void on_lineEditOrientation_textChanged(const QString &);
	void on_lineEditInclination_textChanged(const QString &);

	void on_lineEditOrientation_editingFinished();
	void on_lineEditInclination_editingFinished();

	void on_lineEditXCopy_editingFinished();
	void on_lineEditYCopy_editingFinished();
	void on_lineEditZCopy_editingFinished();

	/*! ComboBox Functions */
	void on_comboBox_activated(int index);

	/*! All tool button specific functions */
	void on_toolButtonTrans_clicked();
	void on_toolButtonRotate_clicked();
	void on_toolButtonScale_clicked();

	/*! Triggered when anything changes in one of the line edits X, Y or Z */
	void onLineEditTextChanged(QtExt::ValidatingLineEdit * lineEdit);

	void on_pushButtonCopyRooms_clicked();
	void on_pushButtonCopySurfaces_clicked();
	void on_pushButtonCopyBuildingLvls_clicked();

	void on_pushButtonAdd_clicked();

	void on_pushButtonEdit_clicked();

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

	/*! Initilizes the Copy Group Box */
	void initializeCopy();

	/*! Depending on the selected operation, we change the look of the local coordinate system object. */
	void updateCoordinateSystemLook();

	/*! Identifies which transformation operation is currently selected and is updated,
		whenever an operation button is toggled.
	*/
	ModificationType					m_modificationType = MT_Translate;

	/*! For each transformation operation we cache the current mode choice from the combo box.
		The value is updated when user changes the combo-box, and when operation is changed,
		the value is used to update the combo box's current index.
	*/
	ModificationState					m_modificationState[MS_Local+1];

	/*! Contains position and rotation of local coordinate system object. */
	Vic3D::Transform3D					m_localCoordinatePosition;

	/*! This is the dimension of the bounding box (dx, dy, dz). */
	IBKMK::Vector3D						m_boundingBoxDimension;

	/*! Cached center point of boinding box. */
	IBKMK::Vector3D						m_boundingBoxCenter;

	/*! Cached normal for absolute rotation */
	IBKMK::Vector3D						m_normal;

	/*! Cached initial values to be used when user had entered invalid values.
		These values depend on current modification type and state.
	*/
	IBKMK::Vector3D						m_originalValues;

	/*! Cached Translation vector for copy operations */
	IBKMK::Vector3D						m_translation;

//	std::set<const VICUS::Object*>		m_selBuild;
//	std::set<const VICUS::Object*>		m_selBuildLvls;
	std::vector<const VICUS::Room*>		m_selRooms;
	std::vector<const VICUS::Surface*>	m_selSurfaces;


	/*! Pointer to UI */
	Ui::SVPropEditGeometry				*m_ui;
};

#endif // SVPropEditGeometryH
