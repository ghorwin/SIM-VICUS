#ifndef SVDBDUPLICATESDIALOG_H
#define SVDBDUPLICATESDIALOG_H

#include <QDialog>

namespace Ui {
	class SVDBDuplicatesDialog;
}


#include "SVDatabase.h"

/*! This dialog handles merging of duplicate database elements. */
class SVDBDuplicatesDialog : public QDialog {
	Q_OBJECT

public:
	explicit SVDBDuplicatesDialog(QWidget *parent = nullptr);
	~SVDBDuplicatesDialog();

	/*! Shows and starts the database dialog.
		\param dbType Filters out only a single DB element type (SVDatabase::NUM_DT selects all).
	*/
	void removeDuplicates(SVDatabase::DatabaseTypes dbType = SVDatabase::NUM_DT);

private:
	Ui::SVDBDuplicatesDialog *m_ui;
};

#endif // SVDBDUPLICATESDIALOG_H
