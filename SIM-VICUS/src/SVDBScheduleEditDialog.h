#ifndef SVDBScheduleEditDialogH
#define SVDBScheduleEditDialogH

#include <QDialog>

namespace Ui {
class SVDBScheduleEditDialog;
}

class QSortFilterProxyModel;
class QModelIndex;

class SVDBScheduleTableModel;

/*! The edit dialog for schedule types. */
class SVDBScheduleEditDialog : public QDialog{
	Q_OBJECT

public:
	explicit SVDBScheduleEditDialog(QWidget *parent = nullptr);
	~SVDBScheduleEditDialog();

	/*! Starts the dialog in "edit schedule" mode. */
	void edit();

	/*! Starts the dialog in "select schedule mode".
		\param initialId The schedule indicated by this ID is initially selected.
		\return If a schedule was selected and double-clicked/or the "Select" button was
				pressed, the function returns the ID of the selected schedule. Otherwise, if the
				dialog was aborted, the function returns VICUS::INVALID_ID.
	*/
	unsigned int select(unsigned int initialId);

private slots:
	void on_pushButtonSelect_clicked();
	void on_pushButtonCancel_clicked();
	void on_pushButtonClose_clicked();

	void on_toolButtonAdd_clicked();
	void on_toolButtonCopy_clicked();
	void on_toolButtonRemove_clicked();

	/*! Connected to the respective signal in the table view.
		Enables/disables the remove button.
	*/
	void onCurrentIndexChanged(const QModelIndex &current, const QModelIndex &/*previous*/);

	void on_pushButtonReloadUserDB_clicked();

	void on_tableView_doubleClicked(const QModelIndex &index);

private:
	Ui::SVDBScheduleEditDialog				*m_ui;

	/*! The component table model (owned). */
	SVDBScheduleTableModel					*m_dbModel		= nullptr;
	/*! The sort filter model (owned). */
	QSortFilterProxyModel					*m_proxyModel	= nullptr;

};

#endif // SVDBScheduleEditDialogH
