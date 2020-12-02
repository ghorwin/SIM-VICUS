#ifndef SVImportIDFDialogH
#define SVImportIDFDialogH

#include <QDialog>

#include <VICUS_Project.h>

namespace Ui {
	class SVImportIDFDialog;
}

namespace EP {
	class Project;
}

/*! Import dialog for IDF files. */
class SVImportIDFDialog : public QDialog {
	Q_OBJECT
public:
	explicit SVImportIDFDialog(QWidget *parent = nullptr);
	~SVImportIDFDialog();

	enum ImportResults {
		ReplaceProject,
		MergeProjects,
		ImportCancelled
	};

	ImportResults import(const QString & fname);

	/*! Project data structure, populated when importing the IDF data. */
	VICUS::Project			m_importedProject;

private slots:
	void on_pushButtonReplace_clicked();

	void on_pushButtonMerge_clicked();

private:
	/*! Transfers data from read project to VICUS::Project. */
	void transferData(const EP::Project & prj);

	ImportResults			m_returnCode;

	Ui::SVImportIDFDialog	*m_ui;

};

#endif // SVImportIDFDialogH
