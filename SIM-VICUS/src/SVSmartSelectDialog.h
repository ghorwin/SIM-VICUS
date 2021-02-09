#ifndef SVSMARTSELECTDIALOG_H
#define SVSMARTSELECTDIALOG_H

#include <QDialog>

namespace Ui {
class SVSmartSelectDialog;
}

class SVSmartSelectDialog : public QDialog
{
	Q_OBJECT

public:
	explicit SVSmartSelectDialog(QWidget *parent = nullptr);
	~SVSmartSelectDialog();

private:
	Ui::SVSmartSelectDialog *m_ui;
};

#endif // SVSMARTSELECTDIALOG_H
