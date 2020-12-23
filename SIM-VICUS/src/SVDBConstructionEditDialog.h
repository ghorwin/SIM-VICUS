#ifndef SVDBConstructionEditDialogH
#define SVDBConstructionEditDialogH

#include <QDialog>

namespace Ui {
	class SVDBConstructionEditDialog;
}

class QSortFilterProxyModel;
class QModelIndex;

class SVDBConstructionTableModel;

/*! The edit dialog for construction types. */
class SVDBConstructionEditDialog : public QDialog {
	Q_OBJECT

public:
	explicit SVDBConstructionEditDialog(QWidget *parent = nullptr);
	~SVDBConstructionEditDialog();

	/*! Starts the dialog in "edit constructions" mode. */
	void edit();

	/*! Starts the dialog in "select construction mode".
		\param initialId The construction indicated by this ID is initially selected.
		\return If a construction was selected and double-clicked/or the "Select" button was
				pressed, the function returns the ID of the selected construction. Otherwise, if the
				dialog was aborted, the function returns -1.
	*/
	int select(unsigned int initialId);

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

	Ui::SVDBConstructionEditDialog	*m_ui;

	/*! The construction table model (owned). */
	SVDBConstructionTableModel		*m_dbModel		= nullptr;
	/*! The sort filter model (owned). */
	QSortFilterProxyModel			*m_proxyModel	= nullptr;
};

#endif // SVDBConstructionEditDialogH
