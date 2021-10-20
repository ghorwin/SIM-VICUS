/*	SIM-VICUS - Building and District Energy Simulation Tool.

	Copyright (c) 2020-today, Institut für Bauklimatik, TU Dresden, Germany

	Primary authors:
	  Andreas Nicolai  <andreas.nicolai -[at]- tu-dresden.de>
	  Dirk Weiss  <dirk.weiss -[at]- tu-dresden.de>
	  Stephan Hirth  <stephan.hirth -[at]- tu-dresden.de>
	  Hauke Hirsch  <hauke.hirsch -[at]- tu-dresden.de>

	  ... all the others from the SIM-VICUS team ... :-)

	This program is part of SIM-VICUS (https://github.com/ghorwin/SIM-VICUS)

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.
*/

#ifndef SVPropEditGeometryH
#define SVPropEditGeometryH

#include <QWidget>

#include <IBKMK_Vector3D.h>

#include <VICUS_Surface.h>

#include <Vic3DTransform3D.h>
#include <Vic3DCoordinateSystemObject.h>

namespace Vic3D {
	class Transform3D;
}

namespace VICUS {
	class Project;
	class Room;
	class Surface;
	class Building;
	class BuildingLevel;
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
		MT_Scale,
		NUM_MT
	};

	enum ModificationState {
		MS_Absolute,
		MS_Relative,
		NUM_MS
	};

	enum RotationState {
		RS_Normal,
		RS_XAxis,
		RS_YAxis,
		RS_ZAxis,
		NUM_RS
	};

	enum OrientationMode {
		OM_Local,
		OM_Global,
		NUM_OM
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
	void setToolButton();

	/*! Checks/unchecks the tool buttons depending on the specific absolute/local mode
	*/
	void setToolButtonAbsMode();

	/*! Checks/unchecks the tool buttons for the specific absolute rotation mode */
	void setToolButtonsRotationState();

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

	/*! Pops up a question dialog and asks user to choose one of two opertions or cancel.
		Function returns 1 for button1, 2 for button 2 and -1 if dialog was canceled.
	*/
	static int requestCopyOperation(QWidget * parent, const QString & title, const QString & text,
									const QString & button1, const QString & button2);

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
	void on_pushButtonAddZone_clicked();
	void on_pushButtonAddRoof_clicked();
	void on_pushButtonAddWindow_clicked();

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


	/*! All tool button specific functions */
	void on_toolButtonTrans_clicked();
	void on_toolButtonRotate_clicked();
	void on_toolButtonScale_clicked();

	/*! Triggered when anything changes in one of the line edits X, Y or Z */
	void onLineEditTextChanged(QtExt::ValidatingLineEdit * lineEdit);

	void on_pushButtonCopyRooms_clicked();
	void on_pushButtonCopySurfaces_clicked();

	void on_pushButtonAdd_clicked();

	void on_pushButtonEdit_clicked();

	void on_pushButtonThreePointRotation_clicked();

	void on_pushButtonFlipNormals_clicked();

	void on_toolButtonLocalCoordinateOrientation_clicked(bool checked);

	void on_pushButtonCopySubSurfaces_clicked();

	void on_toolButtonAbs_clicked(bool);

	void on_toolButtonRel_clicked(bool);

	void on_toolButtonNormal_clicked();

	void on_toolButtonZ_clicked();

	void on_toolButtonX_clicked();

	void on_toolButtonY_clicked();

private:
	/*! Updates the property widget regarding to all geometry data.
		This function is called whenever the selection has changed, and when surface geometry (of selected surfaces)
		has changed.

		This function switches also between AddGeometry and EditGeometry mode, when first selection is made or
		everything is deselected.
	*/
	void updateUi();

	/*! Updates every specific to the orientation mode stored in m_useLocalCoordOrientation
		false: use global coordinate system orientation
		true: use local coordinate system orientation
	*/
	void updateOrientationMode();

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
	ModificationState					m_modificationState[NUM_MT];

	/*! Implies whether the copy mode is active. */
	bool								m_copyMode = false;

	/*! Implies whether local coordinate system rotation should be used
		OM_Global:	use global coordinate system orientation
		OM_Local:	use local coordinate system orientation
	*/
	OrientationMode						m_orientationMode = OM_Global;

	/*! Contains position and rotation of local coordinate system object. */
	Vic3D::Transform3D					m_localCoordinatePosition;

	/*! Pointer to LocalCoordinateSystemObject */
	Vic3D::CoordinateSystemObject		*m_cso;

	/*! This is the dimension of the bounding box (dx, dy, dz) in local/global coordinate system orientation. */
	IBKMK::Vector3D						m_bbDim[NUM_OM];

	/*! Cached center point of boinding box in local/global Orientation. */
	IBKMK::Vector3D						m_bbCenter[NUM_OM];


	/*! Cached normal for absolute rotation */
	IBKMK::Vector3D						m_normal;

	/*! Cached state of abs roation mode */
	RotationState						m_rotationState;

	/*! Cached initial values to be used when user had entered invalid values.
		These values depend on current modification type and state.
	*/
	IBKMK::Vector3D						m_originalValues;


	/*! Cached Translation vector for copy operations. */
	IBKMK::Vector3D						m_translation;

	std::vector<const VICUS::Building*>			m_selBuildings;
	std::vector<const VICUS::BuildingLevel*>	m_selBuildingLevels;
	std::vector<const VICUS::Room*>				m_selRooms;
	std::vector<const VICUS::Surface*>			m_selSurfaces;
	std::vector<const VICUS::SubSurface*>		m_selSubSurfaces;

	/*! We take all selected names. */
	std::set<QString>					m_subSurfNames;
	std::set<QString>					m_surfNames;
	std::set<QString>					m_roomNames;
	std::set<QString>					m_buildingLevelNames;
	std::set<QString>					m_buildingNames;

	/*! Pointer to UI */
	Ui::SVPropEditGeometry				*m_ui;
};

#endif // SVPropEditGeometryH
