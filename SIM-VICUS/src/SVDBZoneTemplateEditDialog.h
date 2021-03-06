#ifndef SVDBZoneTemplateEditDialogH
#define SVDBZoneTemplateEditDialogH

#include <QDialog>

namespace Ui {
	class SVDBZoneTemplateEditDialog;
}

class QModelIndex;
class QGroupBox;
class QSortFilterProxyModel;
class SVDBZoneTemplateTreeModel;
class SVDBZoneTemplateEditWidget;

#include <VICUS_Constants.h>

/*! The edit dialog for materials types. */
class SVDBZoneTemplateEditDialog : public QDialog {
	Q_OBJECT

public:
	/*! Constructor of the dialog.
		The dialog is being configured with the arguments.

		\note Passing an empty editWidgetTitle will not generate a groupbox, but a plain widget instead.
	*/
	explicit SVDBZoneTemplateEditDialog(QWidget *parent);
	~SVDBZoneTemplateEditDialog() override;

	/*! Starts the dialog in "edit" mode. */
	void edit(unsigned int initialId = VICUS::INVALID_ID);

	/*! Starts the dialog in "select mode".
		\param initialId The item indicated by this ID is initially selected.
		\return If an item was selected and double-clicked/or the "Select" button was
				pressed, the function returns the ID of the selected item. Otherwise, if the
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
	/*! If table contains an element with matching ID, this row is made current.
		Signals are blocked in this function.
	*/
	void selectItemById(unsigned int id);

	Ui::SVDBZoneTemplateEditDialog *m_ui;

	/*! The sort filter model (owned). */
	QSortFilterProxyModel			*m_proxyModel	= nullptr;
	/*! The table model (owned). */
	SVDBZoneTemplateTreeModel		*m_dbModel		= nullptr;
	/*! The edit widget (owned). */
	SVDBZoneTemplateEditWidget		*m_editWidget	= nullptr;

	QGroupBox						*m_groupBox = nullptr;

	friend class SVMainWindow;

};

#endif // SVDBZoneTemplateEditDialogH
