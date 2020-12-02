#ifndef SVImportIDFDialogH
#define SVImportIDFDialogH

#include <QDialog>

namespace Ui {
	class SVImportIDFDialog;
}

/*! Import dialog for IDF files. */
class SVImportIDFDialog : public QDialog {
	Q_OBJECT
public:
	explicit SVImportIDFDialog(QWidget *parent = nullptr);
	~SVImportIDFDialog();

	enum ImportResults {
		ReplaceProject,
		MergeProjects
	};

	ImportResults import(const QString & fname);

private:
	Ui::SVImportIDFDialog *m_ui;
};

#endif // SVImportIDFDialogH
