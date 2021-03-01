#ifndef SVDBBoundaryConditionEditDialogH
#define SVDBBoundaryConditionEditDialogH

#include <QDialog>

namespace Ui {
class SVDBBoundaryConditionEditDialog;
}

class QSortFilterProxyModel;
class QModelIndex;

class SVDBBoundaryConditionTableModel;

/*! The edit dialog for boundary types. */
class SVDBBoundaryConditionEditDialog : public QDialog{
	Q_OBJECT

public:
	explicit SVDBBoundaryConditionEditDialog(QWidget *parent = nullptr);
	~SVDBBoundaryConditionEditDialog();

	/*! Starts the dialog in "edit boundary condition" mode. */
	void edit();

	/*! Starts the dialog in "select boundary condition mode".
		\param initialId The boundary condition indicated by this ID is initially selected.
		\return If a boundary condition was selected and double-clicked/or the "Select" button was
				pressed, the function returns the ID of the selected boundary condition. Otherwise, if the
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
	Ui::SVDBBoundaryConditionEditDialog		*m_ui;

	/*! The component table model (owned). */
	SVDBBoundaryConditionTableModel			*m_dbModel		= nullptr;
	/*! The sort filter model (owned). */
	QSortFilterProxyModel					*m_proxyModel	= nullptr;

};

#endif // SVDBBoundaryConditionEditDialogH
