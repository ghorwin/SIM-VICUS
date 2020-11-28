#ifndef SVPropEditGeometryH
#define SVPropEditGeometryH

#include <QWidget>

namespace Ui {
class SVPropEditGeometry;
}

/*! This widget is shown when the scene is put into geometry editing mode. */
class SVPropEditGeometry : public QWidget {
	Q_OBJECT

public:
	explicit SVPropEditGeometry(QWidget *parent = nullptr);
	~SVPropEditGeometry();

private slots:
	void on_pushButtonAddPolygon_clicked();

private:
	Ui::SVPropEditGeometry *m_ui;
};

#endif // SVPropEditGeometryH
