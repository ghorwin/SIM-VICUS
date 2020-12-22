#ifndef SVDBMaterialEditDialogH
#define SVDBMaterialEditDialogH

#include <QDialog>

namespace Ui {
class SVDBMaterialEditDialog;
}

class SVDBMaterialTableModel;
class QModelIndex;
class QSortFilterProxyModel;

/*! The edit dialog for materials types. */
class SVDBMaterialEditDialog : public QDialog {
	Q_OBJECT

public:
	explicit SVDBMaterialEditDialog(QWidget *parent = nullptr);
	~SVDBMaterialEditDialog();

	/*! Starts the dialog in "edit materials" mode. */
	void edit();

	/*! Starts the dialog in "select materials mode".
		\param initialMadId The material indicated by this ID is initially selected.
		\return If a material was selected and double-clicked/or the "Select" button was
				pressed, the function returns the ID of the selected material. Otherwise, if the
				dialog was aborted, the function returns -1.
	*/
	int select(unsigned int initialMatId);

protected:
	void showEvent(QShowEvent * event) override;

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
	Ui::SVDBMaterialEditDialog *m_ui;

	/*! The sort filter model (owned). */
	QSortFilterProxyModel		*m_proxyModel	= nullptr;
	/*! The material table model (owned). */
	SVDBMaterialTableModel		*m_dbModel		= nullptr;

};

#endif // SVDBMaterialEditDialogH
