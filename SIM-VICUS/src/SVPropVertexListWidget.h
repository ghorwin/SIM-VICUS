#ifndef SVPropVertexListWidgetH
#define SVPropVertexListWidgetH

#include <QWidget>

namespace Ui {
	class SVPropVertexListWidget;
}

namespace IBKMK {
	class Vector3D;
}

/*! The widget with newly placed vertexes while constructing a new primitive/object.
	It directly communicates with the new geometry object.

	The appearance of the place vertex list widget depends on the type of geometry
	currently being added.
*/
class SVPropVertexListWidget : public QWidget {
	Q_OBJECT

public:
	explicit SVPropVertexListWidget(QWidget *parent = nullptr);
	~SVPropVertexListWidget();

	/*! Appends a new vertex to the list of vertexes in the table widget. */
	void addVertex(const IBKMK::Vector3D & p);

public slots:

	/*! Called, when user starts with a new polygon/geometry. */
	void onNewVertexListStart();

	void on_pushButtonFinish_clicked();

private slots:
	void on_pushButtonDeleteLast_clicked();

	void on_tableWidgetVertexes_itemSelectionChanged();

	void on_pushButtonCancel_clicked();

	void on_pushButtonDeleteSelected_clicked();


private:
	Ui::SVPropVertexListWidget	*m_ui;
};


#endif // SVPropVertexListWidgetH
