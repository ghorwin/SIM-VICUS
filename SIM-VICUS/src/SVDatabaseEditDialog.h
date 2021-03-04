#ifndef SVDatabaseEditDialogH
#define SVDatabaseEditDialogH

#include <QDialog>

namespace Ui {
	class SVDatabaseEditDialog;
}

class SVAbstractDatabaseTableModel;
class SVAbstractDatabaseEditWidget;
class QModelIndex;
class QGroupBox;
class QSortFilterProxyModel;

/*! The edit dialog for materials types. */
class SVDatabaseEditDialog : public QDialog {
	Q_OBJECT

public:
	explicit SVDatabaseEditDialog(QWidget *parent, SVAbstractDatabaseTableModel * tableModel,
								  SVAbstractDatabaseEditWidget * editWidget,
								  const QString & title, const QString & editWidgetTitle,
								  bool horizontalLayout);
	~SVDatabaseEditDialog() override;

	/*! Starts the dialog in "edit" mode. */
	void edit();

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
	// Factory functions to create all the individual dialogs
	static SVDatabaseEditDialog * createMaterialEditDialog(QWidget * parent);


	Ui::SVDatabaseEditDialog *m_ui;

	/*! The sort filter model (owned). */
	QSortFilterProxyModel			*m_proxyModel	= nullptr;
	/*! The table model (owned). */
	SVAbstractDatabaseTableModel	*m_dbModel		= nullptr;
	/*! The edit widget (owned). */
	SVAbstractDatabaseEditWidget	*m_editWidget	= nullptr;
	QGroupBox						*m_editWidgetGroupBox = nullptr;

	friend class SVMainWindow;
};

#endif // SVDatabaseEditDialogH
