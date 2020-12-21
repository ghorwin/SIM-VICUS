#ifndef SVDBMATERIALSEDITDIALOG_H
#define SVDBMATERIALSEDITDIALOG_H

#include <QDialog>

namespace Ui {
class SVDBMaterialsEditDialog;
}

class SVDBMaterialsEditDialog : public QDialog
{
	Q_OBJECT

public:
	explicit SVDBMaterialsEditDialog(QWidget *parent = nullptr);
	~SVDBMaterialsEditDialog();

private:
	Ui::SVDBMaterialsEditDialog *ui;
};

#endif // SVDBMATERIALSEDITDIALOG_H
