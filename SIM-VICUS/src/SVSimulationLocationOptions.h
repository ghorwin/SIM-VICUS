#ifndef SVSimulationLocationOptionsH
#define SVSimulationLocationOptionsH

#include <QWidget>

namespace Ui {
	class SVSimulationLocationOptions;
}

namespace NANDRAD {
	class Location;
}

/*! Widget with settings related to location. */
class SVSimulationLocationOptions : public QWidget {
	Q_OBJECT

public:
	SVSimulationLocationOptions(QWidget *parent, NANDRAD::Location & location);
	~SVSimulationLocationOptions();

	/*! Updates user interface with properties from the project data structure.
		This function is called whenever the dialog is first shown.
	*/
	void updateUi();

private:
	Ui::SVSimulationLocationOptions		*m_ui;
	NANDRAD::Location					*m_location;
};

#endif // SVSimulationLocationOptionsH
