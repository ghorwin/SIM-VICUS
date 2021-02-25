#ifndef SVDBWindowEditDialogH
#define SVDBWindowEditDialogH

#include <QDialog>

namespace Ui {
	class SVDBWindowEditDialog;
}

class QSortFilterProxyModel;
class QModelIndex;

class SVDBWindowTableModel;

/*! The edit dialog for construction types. */
class SVDBWindowEditDialog : public QDialog {
	Q_OBJECT

public:
	explicit SVDBWindowEditDialog(QWidget *parent = nullptr);
	~SVDBWindowEditDialog();

	/*! Starts the dialog in "edit" mode. */
	void edit();

	/*! Starts the dialog in "selectmode".
		\param initialId The window indicated by this ID is initially selected.
		\return If a window was selected and double-clicked/or the "Select" button was
				pressed, the function returns the ID of the selected window. Otherwise, if the
				dialog was aborted, the function returns -1.
	*/
	int select(unsigned int);

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

	Ui::SVDBWindowEditDialog	*m_ui;

	/*! The construction table model (owned). */
	SVDBWindowTableModel		*m_dbModel		= nullptr;
	/*! The sort filter model (owned). */
	QSortFilterProxyModel		*m_proxyModel	= nullptr;
};

#endif // SVDBWindowEditDialogH
