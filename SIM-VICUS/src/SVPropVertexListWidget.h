#ifndef SVPropVertexListWidgetH
#define SVPropVertexListWidgetH

#include <QWidget>

namespace Ui {
	class SVPropVertexListWidget;
}

namespace IBKMK {
	class Vector3D;
}

/*! The widget with newly placed vertexes while constructing a new primitive/object. */
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

private slots:
	void on_pushButtonDeleteLast_clicked();

	void on_tableWidgetVertexes_itemSelectionChanged();

private:
	Ui::SVPropVertexListWidget	*m_ui;
};


#endif // SVPropVertexListWidgetH
