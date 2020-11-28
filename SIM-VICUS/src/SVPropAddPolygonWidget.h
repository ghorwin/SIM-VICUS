#ifndef SVPROPADDPOLYGONWIDGET_H
#define SVPROPADDPOLYGONWIDGET_H

#include <QWidget>

namespace Ui {
class SVPropAddPolygonWidget;
}

/*! Property widget that is shown when a new polygon is added.
	Displays list of added vertexes and confirmation button.
*/
class SVPropAddPolygonWidget : public QWidget {
	Q_OBJECT

public:
	explicit SVPropAddPolygonWidget(QWidget *parent = nullptr);
	~SVPropAddPolygonWidget();

protected:
	void keyPressEvent(QKeyEvent *event) override;

private slots:
	void on_toolButtonPlane_clicked();

	void on_toolButtonBox_clicked();

	void on_pushButtonAdd_clicked();

	void on_pushButtonCancel_clicked();

private:
	Ui::SVPropAddPolygonWidget *m_ui;
};

#endif // SVPROPADDPOLYGONWIDGET_H
