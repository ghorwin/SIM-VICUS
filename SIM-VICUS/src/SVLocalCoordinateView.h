#ifndef SVLocalCoordinateViewH
#define SVLocalCoordinateViewH

#include <QWidget>

#include <Vic3DTransform3D.h>

namespace Ui {
class SVLocalCoordinateView;
}

/*! A widget that shows the location and orientation of the local coordinate system. */
class SVLocalCoordinateView : public QWidget {
	Q_OBJECT
public:
	explicit SVLocalCoordinateView(QWidget *parent = nullptr);
	~SVLocalCoordinateView();

	/*! Sets the Coordinates of the Center Point of the local Coordinate System. */
	void setCoordinates(const Vic3D::Transform3D &t);

	/*! Called from GeometryView when view state changes. */
	void setAlignCoordinateSystemButtonChecked(bool checked);

private slots:
	void on_toolButtonAlignCoordinateSystem_clicked();

private:
	Ui::SVLocalCoordinateView *m_ui;

	QVector3D				m_xAxis;
	QVector3D				m_yAxis;
	QVector3D				m_zAxis;
};

#endif // SVLocalCoordinateViewH
