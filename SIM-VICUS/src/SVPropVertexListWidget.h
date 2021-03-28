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

	/*! Transfers the distance into the respective line widget. */
	void setExtrusionDistance(double dist);

public slots:

	/*! Connected to SVProjectHandler::modified().
		Updates the building, building level and zone combo boxes with data from the project.
		This function does nothing, if the widget is currently inactive/invisible.
	*/
	void onModified( int modificationType, ModificationInfo * data );

	/*! Called, when user starts with a new polygon/geometry. */
	void clearPolygonVertexList();

	/*! Finishes the geometry and creates the undo action to modify the
		project.
	*/
	void on_pushButtonFinish_clicked();

	/*! Switches from floor polygon mode to extrusion mode. */
	void on_pushButtonFloorDone_clicked();

private slots:
	void on_pushButtonDeleteLast_clicked();

	void on_tableWidgetVertexes_itemSelectionChanged();

	void on_pushButtonCancel_clicked();

	void on_pushButtonDeleteSelected_clicked();

	void onEditComponents();

	void on_toolButtonAddBuilding_clicked();

	void on_toolButtonAddBuildingLevel_clicked();

	void on_toolButtonAddZone_clicked();

	void on_checkBoxAnnonymousGeometry_stateChanged(int arg1);

	void on_comboBoxBuilding_currentIndexChanged(int index);

	void on_comboBoxBuildingLevel_currentIndexChanged(int index);


	void on_lineEditZoneHeight_editingFinishedSuccessfully();

	void on_pushButtonPickZoneHeight_clicked();

private:
	/*! Returns true, if annonymous geometry is being created (i.e. checkbox is visible and checked). */
	bool createAnnonymousGeometry() const;

	/*! Updates the enabled/disable states of all labels/combo boxes and tool buttons depending on available data. */
	void updateEnabledStates();

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

	Ui::SVPropVertexListWidget	*m_ui;
};


#endif // SVPropVertexListWidgetH
