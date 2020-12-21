#ifndef SVDBMaterialsEditDialogH
#define SVDBMaterialsEditDialogH

#include <QDialog>

namespace Ui {
class SVDBMaterialsEditDialog;
}

class SVDBMaterialTableModel;
class QModelIndex;

/*! The edit dialog for materials types. */
class SVDBMaterialsEditDialog : public QDialog {
	Q_OBJECT

public:
	explicit SVDBMaterialsEditDialog(QWidget *parent = nullptr);
	~SVDBMaterialsEditDialog();

	/*! Starts the dialog in "edit materials" mode. */
	void edit();

	/*! Starts the dialog in "select materials mode".
		\return If a material was selected and double-clicked/or the "Select" button was
				pressed, the function returns the ID of the selected material. Otherwise, if the
				dialog was aborted, the function returns 0.
	*/
	unsigned int select();

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
	Ui::SVDBMaterialsEditDialog *m_ui;
};

#endif // SVDBMaterialsEditDialogH
