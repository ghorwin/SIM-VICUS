#ifndef SVSimulationLocationOptionsH
#define SVSimulationLocationOptionsH

#include <QWidget>

namespace Ui {
	class SVSimulationLocationOptions;
}

namespace NANDRAD {
	class Location;
}

class SVClimateDataTableModel;
class SVClimateDataSortFilterProxyModel;

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

private slots:
	void on_radioButtonFromDB_toggled(bool checked);

	void on_lineEditTextFilter_editingFinished();

	void on_lineEditTextFilter_textChanged(const QString &arg1);

private:
	Ui::SVSimulationLocationOptions		*m_ui;
	SVClimateDataTableModel				*m_climateDataModel = nullptr;
	SVClimateDataSortFilterProxyModel	*m_filterModel		= nullptr;
	NANDRAD::Location					*m_location;
};

#endif // SVSimulationLocationOptionsH
