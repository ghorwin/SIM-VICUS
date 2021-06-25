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



	// page surfaces

	void on_toolButtonAddBuilding_clicked();

	void on_toolButtonAddBuildingLevel_clicked();

	void on_toolButtonAddZone_clicked();

	void on_checkBoxAnnonymousGeometry_stateChanged(int arg1);

	void on_comboBoxBuilding_currentIndexChanged(int index);

	void on_comboBoxBuildingLevel_currentIndexChanged(int index);

	void on_pushButtonCreateSurface_clicked();


	// page zone

	void on_lineEditZoneHeight_editingFinishedSuccessfully();

	void on_pushButtonPickZoneHeight_clicked();



private:
	/*! Returns true, if annonymous geometry is being created (i.e. checkbox is visible and checked). */
	bool createAnnonymousGeometry() const;

	/*! Updates the enabled/disable states of all labels/combo boxes and tool buttons depending on available data. */
	void updateSurfacePageState();

	bool reselectById(QComboBox * combo, int id) const;

	/*! Updates the content of the building combo box with data from the project.
		The current item is kept (identified via unique ID), if it still exists after the update.
		If the current item changed, the function updateBuildingLevelsComboBox() will be called to update
		the dependent combo boxes.
	*/
	void updateBuildingComboBox();

	/*! Updates the content of the building levels combo box with data from the project.
		The current item is kept (identified via unique ID), if it still exists after the update.
		If the current item changed, the function updateZoneComboBox() will be called to update
		the dependent combo boxes.
	*/
	void updateBuildingLevelsComboBox();

	/*! Updates the content of the building levels combo box with data from the project.
		The current item is kept (identified via unique ID), if it still exists after the update.
	*/
	void updateZoneComboBox();

	/*! Creates a room with a roof shape such as a gable roof instead of a prism (room).
	 *  In addition, you can choose whether a knee floor exists or not. */
	void createRoofZone();

	Ui::SVPropVertexListWidget	*m_ui;

	/*! What kind of geometry is being created.
		\note In two-stage processes, it does not always correspond to the geometry mode set in the
		new geometry object.
	*/
	int							m_geometryMode;
};


#endif // SVPropVertexListWidgetH
