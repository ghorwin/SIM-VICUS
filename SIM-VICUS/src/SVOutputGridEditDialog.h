#ifndef SVOutputGridEditDialogH
#define SVOutputGridEditDialogH

#include <QDialog>

namespace Ui {
	class SVOutputGridEditDialog;
}

namespace NANDRAD {
	class OutputGrid;
}

/*! Dialog for editing output grids. */
class SVOutputGridEditDialog : public QDialog {
	Q_OBJECT

public:
	explicit SVOutputGridEditDialog(QWidget *parent = nullptr);
	~SVOutputGridEditDialog();

	bool edit(NANDRAD::OutputGrid & grid, int index);

private:
	Ui::SVOutputGridEditDialog *m_ui;
};

#endif // SVOutputGridEditDialogH
