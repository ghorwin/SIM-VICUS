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

	/*! Sets the Coordinates of the Center Point of the local
		Coordinate System
	*/
	void setCoordinates(const Vic3D::Transform3D &t);

	/*! Sets the Orienation of an given Axis in regard to the
		absolute coordinate system

		TODO : Remove?
	*/
	void setOrientation(const QVector3D &x, const QVector3D &y, const QVector3D &z);

private slots:
	void on_comboBoxAxis_currentIndexChanged(int index);

	void on_toolButton_toggled(bool checked);

	void on_toolButtonOrientation_toggled(bool checked);

private:
	Ui::SVLocalCoordinateView *m_ui;

	QVector3D				m_xAxis;
	QVector3D				m_yAxis;
	QVector3D				m_zAxis;
};

#endif // SVLocalCoordinateViewH
