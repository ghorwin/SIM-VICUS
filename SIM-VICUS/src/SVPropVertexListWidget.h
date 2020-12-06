#ifndef SVPropVertexListWidgetH
#define SVPropVertexListWidgetH

#include <QWidget>

namespace Ui {
	class SVPropVertexListWidget;
}

/*! The widget with newly placed vertexes while constructing a new primitive/object. */
class SVPropVertexListWidget : public QWidget {
	Q_OBJECT

public:
	explicit SVPropVertexListWidget(QWidget *parent = nullptr);
	~SVPropVertexListWidget();

private:
	Ui::SVPropVertexListWidget *m_ui;
};


#endif // SVPropVertexListWidgetH
