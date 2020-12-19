#ifndef SVDBCONSTRUCTIONEDITDIALOG_H
#define SVDBCONSTRUCTIONEDITDIALOG_H

#include <QDialog>

namespace Ui {
	class SVDBConstructionEditDialog;
}

class QItemSelection;

class SVDBConstructionTreeModel;

/*! The edit dialog for construction types. */
class SVDBConstructionEditDialog : public QDialog {
	Q_OBJECT

public:
	explicit SVDBConstructionEditDialog(QWidget *parent = nullptr);
	~SVDBConstructionEditDialog();

	/*! Starts the dialog in "edit constructions" mode. */
	void edit();

	/*! Starts the dialog in "select construction mode".
		\return If a construction was selected and double-clicked/or the "Select" button was
				pressed, the function returns the ID of the selected construction. Otherwise, if the
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
//	void onCurrentIndexChanged(QModelIndex,QModelIndex);

private:

	Ui::SVDBConstructionEditDialog	*m_ui;

	/*! Cached pointer to construction tree model (provided and owned by SVSettings). */
	SVDBConstructionTreeModel		*m_constructionTreeModel = nullptr;
};

#endif // SVDBCONSTRUCTIONEDITDIALOG_H
