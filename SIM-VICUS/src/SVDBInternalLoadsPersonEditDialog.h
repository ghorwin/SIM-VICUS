#ifndef SVDBInternalLoadsPersonEditDialogH
#define SVDBInternalLoadsPersonEditDialogH

#include <QDialog>

namespace Ui {
class SVDBInternalLoadsPersonEditDialog;
}

class SVDBInternalLoadsTableModel;
class QModelIndex;
class QSortFilterProxyModel;

/*! The edit dialog for internal loads person model types. */
class SVDBInternalLoadsPersonEditDialog : public QDialog {
	Q_OBJECT

public:
	explicit SVDBInternalLoadsPersonEditDialog(QWidget *parent = nullptr);
	~SVDBInternalLoadsPersonEditDialog() override;

	/*! Starts the dialog in "edit internal loads person" mode. */
	void edit();

	/*! Starts the dialog in "internal loads person mode".
		\param initial personId The person load model indicated by this ID is initially selected.
		\return If a person load model was selected and double-clicked/or the "Select" button was
				pressed, the function returns the ID of the selected person load model. Otherwise, if the
				dialog was aborted, the function returns -1.
	*/
	int select(unsigned int initialIntLoadId);

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
	Ui::SVDBInternalLoadsPersonEditDialog	*m_ui;

	/*! The sort filter model (owned). */
	QSortFilterProxyModel					*m_proxyModel	= nullptr;
	/*! The internal load table model (owned). */
	SVDBInternalLoadsTableModel				*m_dbModel		= nullptr;

};

#endif // SVDBInternalLoadsPersonEditDialogH
