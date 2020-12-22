#ifndef SVDBComponentEditDialogH
#define SVDBComponentEditDialogH

#include <QDialog>

namespace Ui {
class SVDBComponentEditDialog;
}

class QSortFilterProxyModel;
class QModelIndex;

class SVDBComponentTableModel;

/*! The edit dialog for component types. */
class SVDBComponentEditDialog : public QDialog {
	Q_OBJECT

public:
	explicit SVDBComponentEditDialog(QWidget *parent = nullptr);
	~SVDBComponentEditDialog();

	/*! Starts the dialog in "edit components" mode. */
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
	Ui::SVDBComponentEditDialog *m_ui;

	/*! The component table model (owned). */
	SVDBComponentTableModel			*m_dbModel		= nullptr;
	/*! The sort filter model (owned). */
	QSortFilterProxyModel			*m_proxyModel	= nullptr;
};

#endif // SVDBComponentEditDialogH
