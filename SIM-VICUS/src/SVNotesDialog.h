#ifndef SVNOTESDIALOG_H
#define SVNOTESDIALOG_H

#include <QDialog>

namespace Ui {
class SVNotesDialog;
}

class SVNotesDialog : public QDialog
{
	Q_OBJECT

public:
	explicit SVNotesDialog(QWidget *parent = nullptr);
	~SVNotesDialog();

private:
	Ui::SVNotesDialog *m_ui;
};

#endif // SVNOTESDIALOG_H
