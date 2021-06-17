#ifndef SVPropAddWindowWidgetH
#define SVPropAddWindowWidgetH

#include <QWidget>

namespace Ui {
class SVPropAddWindowWidget;
}

class QSpinBox;

class ModificationInfo;

/*! Property widget for adding sub-surfaces. */
class SVPropAddWindowWidget : public QWidget {
	Q_OBJECT

public:
	explicit SVPropAddWindowWidget(QWidget *parent = nullptr);
	~SVPropAddWindowWidget();

	/*! Call this function before switching to this property widget.
		This will setup the "new subsurface preview" object and fix any invalid
		inputs with defaults.
	*/
	void setup();

public slots:

	void onModified(int modificationType, ModificationInfo * /*data*/);

private slots:
	void onSpinBoxValueChanged(int arg1);

	void on_lineEditWindowWidth_editingFinishedSuccessfully();

private:
	struct WindowComputationData {
		int m_priorities[4];
		double m_width;
		double m_height;
		double m_windowSillHeight;
		double m_distance;
	};

	/*! Updates widget to current project state. */
	void updateUi();

	/*! Collects input data from widget and updates data in window geometry object. */
	void updateGeometryObject();

	Ui::SVPropAddWindowWidget *m_ui;

	WindowComputationData		m_windowInputData;

	QSpinBox					*m_prioritySpinBoxes[4];
};

#endif // SVPropAddWindowWidgetH
