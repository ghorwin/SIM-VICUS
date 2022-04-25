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

#ifndef SVPropVertexListWidgetH
#define SVPropVertexListWidgetH

#include <QWidget>
#include <VICUS_PlaneGeometry.h>

namespace Ui {
	class SVPropVertexListWidget;
}

namespace IBKMK {
	class Vector3D;
}

class QComboBox;
class ModificationInfo;


/*! The widget with newly placed vertexes while constructing a new primitive/object.
	It directly communicates with the new geometry object.

	The appearance of the place vertex list widget depends on the type of geometry
	currently being added.

	Currently, this widget also holds properties of newly created geometry. Maybe this
	widget should be renamed "SVPropNewObjectWidget".

	Button state update mechanism:

	- the buttons for adding building levels and zones change state through undo-actions
	  and when the selection in a combo box further up needs an update
	- labels, push buttons and combo boxes are enabled/disabled in updateButtonStates(), which
	  is called from onModified() (whenever project is modified/undo-action executed)
	  and from the on_comboBoxXXX_currentIndexChanged()

	Note: property widgets are created on demand, so it may be that we load a project (with the
		  AllModified modification signal), but since the widget hasn't been created yet, we do not
		  get the modification signal. Hence, we update the button states also from the constructor
		  of the widget.
*/
class SVPropVertexListWidget : public QWidget {
	Q_OBJECT

public:
	explicit SVPropVertexListWidget(QWidget *parent = nullptr);
	~SVPropVertexListWidget();

	/*! Sets up the widget to be used for creating geometry of a given type.
		\param newGeometryType A type as declared in NewGeometryObject::NewGeometryMode
	*/
	void setup(int newGeometryType);

	/*! Updates the component combo boxes (this needs to be done whenever the component DB has been changed). */
	void updateComponentComboBoxes();

	/*! Appends a new vertex to the list of vertexes in the table widget.
		Called from NewGeometryObject.
	*/
	void addVertex(const IBKMK::Vector3D & p);

	/*! Removes selected index from table widget.
		Called from NewGeometryObject.
	*/
	void removeVertex(unsigned int idx);

	/*! Called from NewGeometryWidget with new zone height, to be shown in respective line widget. */
	void setZoneHeight(double dist);

	/*! If we are in "draw polygon mode" and completing is possible, do it (same as clicking on "Complete polygon" button).
		This function is called globally, when user presses Return while drawing.
	*/
	bool completePolygonIfPossible();

public slots:

	/*! Connected to SVProjectHandler::modified().
		Updates the building, building level and zone combo boxes with data from the project, if these were only
		renamed. Otherwise the new geometry operation is aborted.
	*/
	void onModified( int modificationType, ModificationInfo * data );


private slots:

	// page vertexes

	void on_pushButtonDeleteLast_clicked();
	void on_tableWidgetVertexes_itemSelectionChanged();
	void on_pushButtonDeleteSelected_clicked();
	void on_pushButtonCompletePolygon_clicked();

	/*! This slot is called from all cancel buttons in all widgets. It basically
		aborts the current operation, clears the new geometry widget and aborts the widget. */
	void onCancel();

	/*! This slot is called from all edit component tool buttons.
		It updates all component combo boxes after DB dialog has finished.
	*/
	void onEditComponents();

	/*! This slot is called from all edit sub surface component tool buttons.
		It updates all sub surface component combo boxes after DB dialog has finished.
	*/
	void onEditSubSurfaceComponents();



	// page surfaces

	void on_toolButtonAddBuilding_clicked(); // also used by zone/roof pages
	void on_toolButtonAddBuildingLevel_clicked(); // also used by zone/roof pages
	void on_toolButtonAddZone_clicked();
	void on_checkBoxAnnonymousGeometry_stateChanged(int arg1);
	void on_comboBoxBuilding_currentIndexChanged(int index);
	void on_comboBoxBuildingLevel_currentIndexChanged(int index);
	void on_pushButtonCreateSurface_clicked();


	// page zone

	void on_lineEditZoneHeight_editingFinishedSuccessfully();
	void on_pushButtonPickZoneHeight_clicked();
	void on_comboBoxBuilding2_currentIndexChanged(int index);
	void on_comboBoxBuildingLevel2_currentIndexChanged(int index);
	void on_pushButtonCreateZone_clicked();


	// page roof

	void on_comboBoxBuilding3_currentIndexChanged(int index);
	void on_comboBoxBuildingLevel3_currentIndexChanged(int index);
	void on_lineEditRoofHeight_editingFinishedSuccessfully();
	void on_comboBoxRoofType_currentIndexChanged(int index);
	void on_radioButtonRoofHeight_toggled(bool checked);
	void on_checkBoxFlapTile_toggled(bool checked);
	void on_pushButtonCreateRoof_clicked();

	void on_pushButtonRotateFloorPolygon_clicked();

	void on_toolButtonEditSubSurfComponents_clicked();

	void on_checkBoxSubSurfaceGeometry_stateChanged(int arg1);

	void on_checkBoxSelectedSurfaces_stateChanged(int arg1);

private:
	/*! Returns true, if annonymous geometry is being created (i.e. checkbox is visible and checked). */
	bool createAnnonymousGeometry() const;

	/*! Returns true, if sub surface geometry is being created (i.e. checkbox is visible and checked). */
	bool createSubSurfaceGeometry() const;

	/*! Updates the enabled/disable states of all labels/combo boxes and tool buttons depending on available data. */
	void updateButtonStates();

	/*! Populates the combo box with components of matching type:
		0 - wall
		1 - floor
		2 - roof/ceiling
		-1 - all
	*/
	void updateComponentComboBox(QComboBox * combo, int type);

	/*! Populates the combo box with sub surface components. */
	void updateSubSurfaceComponentComboBox(QComboBox * combo);

	bool reselectById(QComboBox * combo, int id) const;

	/*! Updates the content of the building combo box with data from the project.
		The current item is kept (identified via unique ID), if it still exists after the update.
		You should call updateBuildingLevelsComboBox afterwards to update the available options.
	*/
	void updateBuildingComboBox(QComboBox * combo);

	/*! Updates the content of the building levels combo box with data from the project.
		The current item is kept (identified via unique ID), if it still exists after the update.
	*/
	void updateBuildingLevelsComboBox(QComboBox * combo, const QComboBox * buildingCombo);

	/*! Updates the content of the building levels combo box with data from the project.
		The current item is kept (identified via unique ID), if it still exists after the update.
	*/
	void updateZoneComboBox(QComboBox * combo, const QComboBox * buildingLevelCombo);

	/*! Updates the content of the building levels combo box with data from the project.
		The current item is kept (identified via unique ID), if it still exists after the update.
	*/
	void updateSurfaceComboBox(QComboBox * combo, bool onlySelected=true);

	/*! Takes modified input data from widget and transfers it to the new geometry object. */
	void updateRoofGeometry();

	Ui::SVPropVertexListWidget	*m_ui;

	/*! What kind of geometry is being created.
		\note In two-stage processes, it does not always correspond to the geometry mode set in the
		new geometry object.
	*/
	int							m_geometryMode;

	/*! If true, user requested rotation of polygon. */
	bool						m_polygonRotation = false;

	/*! All points of the polyline are held here. When switching from Complex to other roof shapes,
	 *   only the first three points are used.*/
	std::vector<IBKMK::Vector3D>	m_roofPolygon;

	/*! Current starting index for creating roof shapes. It is not used in COMPLEX SHAPE. */
	unsigned int					m_currentIdxOfStartpoint = 0;

};


#endif // SVPropVertexListWidgetH
