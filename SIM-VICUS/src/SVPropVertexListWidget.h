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

	/*! Updates the component combo boxes (this needs to be done whenever the component DB has been changed. */
	void updateComponentComboBoxes();

	/*! Appends a new vertex to the list of vertexes in the table widget.
		Called from NewGeometryObject.
	*/
	void addVertex(const IBKMK::Vector3D & p);

	/*! Removes selected index from table widget.
		Called from NewGeometryObject.
	*/
	void removeVertex(unsigned int idx);

public slots:

	/*! Called, when user starts with a new polygon/geometry. */
	void clearPolygonVertexList();

	/*! Finishes the geometry and creates the undo action to modify the
		project.
	*/
	void on_pushButtonFinish_clicked();

private slots:
	void on_pushButtonDeleteLast_clicked();

	void on_tableWidgetVertexes_itemSelectionChanged();

	void on_pushButtonCancel_clicked();

	void on_pushButtonDeleteSelected_clicked();

	void onEditComponents();

private:
	void reselectById(QComboBox * combo, int id) const;

	Ui::SVPropVertexListWidget	*m_ui;
};


#endif // SVPropVertexListWidgetH
