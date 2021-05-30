#ifndef SVPropAddWindowWidgetH
#define SVPropAddWindowWidgetH

#include <QWidget>

namespace Ui {
class SVPropAddWindowWidget;
}

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

private:
	Ui::SVPropAddWindowWidget *m_ui;
};

#endif // SVPropAddWindowWidgetH
