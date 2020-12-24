#ifndef SVDBBoundaryConditionEditDialogH
#define SVDBBoundaryConditionEditDialogH

#include <QDialog>

namespace Ui {
class SVDBBoundaryConditionEditDialog;
}

class QSortFilterProxyModel;
class QModelIndex;

class SVDBBoundaryConditionTableModel;

/*! The edit dialog for component types. */
class SVDBBoundaryConditionEditDialog : public QDialog{
	Q_OBJECT

public:
	explicit SVDBBoundaryConditionEditDialog(QWidget *parent = nullptr);
	~SVDBBoundaryConditionEditDialog();

	/*! Starts the dialog in "edit boundary condition" mode. */
	void edit();

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



private:
	Ui::SVDBBoundaryConditionEditDialog		*m_ui;

	/*! The component table model (owned). */
	SVDBBoundaryConditionTableModel			*m_dbModel		= nullptr;
	/*! The sort filter model (owned). */
	QSortFilterProxyModel					*m_proxyModel	= nullptr;

};

#endif // SVDBBoundaryConditionEditDialogH
