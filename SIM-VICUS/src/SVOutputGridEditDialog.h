#ifndef SVOutputGridEditDialogH
#define SVOutputGridEditDialogH

#include <QDialog>

#include <NANDRAD_Interval.h>

namespace Ui {
	class SVOutputGridEditDialog;
}

namespace NANDRAD {
	class OutputGrid;
}

namespace VICUS {
	class Outputs;
}


/*! Dialog for editing output grids. */
class SVOutputGridEditDialog : public QDialog {
	Q_OBJECT

public:
	explicit SVOutputGridEditDialog(QWidget *parent = nullptr);
	~SVOutputGridEditDialog();

	/*! Opens the editor; ownIndex is the index of the currently being edited output grid in
		the list of output grids (-1 for newly added output grids).
	*/
	bool edit(NANDRAD::OutputGrid & def, const VICUS::Outputs & outputs, int ownIndex);

protected:
	/*! QDialog::accept() re-implemented for input data checking (called indirectly from buttonBox). */
	void accept();

private slots:
	void on_spinBoxIntervalCount_valueChanged(int arg1);

	/*! Triggered on any cell value change. */
	void on_tableWidget_cellChanged(int row, int column);

private:
	/*! Populates interval table with content of interval vector in either m_schedule or m_grid, depending
		on type of this dialog.
	*/
	void updateIntervalTable();

	/*! Helper function, fills in a single column in the interval edit table. */
	void fillColumn(int columnIdx, const NANDRAD::Interval & ival);

	/*! Checks for correct input in interval table. */
	bool checkIntervals();

	/*! Transfers data from interval table to vector with intervals.
		Table input must have been checked for validity beforehand.
	*/
	void storeIntervals(std::vector<NANDRAD::Interval> & intervals) const;

	/*! Parses the interval table and stops on error.
		\param intervals The interval list to be populated.
		\param showMessageOnError If an error occurs and this flag is true, an QMessageBox is shown indicating the problem.
		\return Returns true, if table was successfully parsed and a valid interval list was entered. False if not.
	*/
	bool parseTable(std::vector<NANDRAD::Interval> &intervals, bool showMessageOnError) const;

	/*! Shows a message box and highlights and selects item in table widget. */
	void showError(int row, int col, const QString & text) const;

	Ui::SVOutputGridEditDialog *m_ui;

	/*! Pointer to outputs data structure - needed to check for uniqueness of output grid name. */
	const VICUS::Outputs		*m_outputs;
	/*! Pointer to currently edited data structure (output grid). */
	NANDRAD::OutputGrid			*m_grid;
	/*! Own definition index (to skip name clash check), -1 if not yet in list. */
	int							m_ownIdx;
};

#endif // SVOutputGridEditDialogH
