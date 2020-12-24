#ifndef SVDBBOUNDARYCONDITIONEDITDIALOG_H
#define SVDBBOUNDARYCONDITIONEDITDIALOG_H

#include <QWidget>

namespace Ui {
class SVDBBoundaryConditionEditDialog;
}

class SVDBBoundaryConditionEditDialog : public QWidget
{
	Q_OBJECT

public:
	explicit SVDBBoundaryConditionEditDialog(QWidget *parent = nullptr);
	~SVDBBoundaryConditionEditDialog();

private:
	Ui::SVDBBoundaryConditionEditDialog *ui;
};

#endif // SVDBBOUNDARYCONDITIONEDITDIALOG_H
