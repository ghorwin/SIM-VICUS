#ifndef SVNOTESDIALOG_H
#define SVNOTESDIALOG_H

#include <QDialog>

class ModificationInfo;

namespace Ui {
class SVNotesDialog;
}

namespace VICUS {
class Project;
}

/*! Dialog to add/edit project specific notes/comments. */
class SVNotesDialog : public QDialog
{
	Q_OBJECT

public:
	explicit SVNotesDialog(QWidget *parent = nullptr);
	~SVNotesDialog();

public slots:
	/*! Connected to SVProjectHandler::modified().
		Updates Project Notes.
	*/
	void onModified(int modificationType, ModificationInfo * );

private slots:
	void on_pushButtonSave_clicked();
	void on_pushButtonCancel_clicked();


private:
	/*! Updates note in UI from VICUS prj. */
	void updateNotesFromPrj();


	Ui::SVNotesDialog			*m_ui;
};

#endif // SVNOTESDIALOG_H
