#ifndef SVSTRUCTURALUNITCREATIONDIALOG_H
#define SVSTRUCTURALUNITCREATIONDIALOG_H

#include <QDialog>
#include "VICUS_Project.h"

namespace Ui {
class SVStructuralUnitCreationDialog;
}

class SVStructuralUnitCreationDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SVStructuralUnitCreationDialog(QWidget *parent = nullptr);
    ~SVStructuralUnitCreationDialog();

	bool edit(const VICUS::StructuralUnit * unit);

	bool create();

private slots:
	void on_buttonBoxCreateUnit_accepted();

private:
	Ui::SVStructuralUnitCreationDialog *m_ui;

	/*! Current structural unit. */
	const VICUS::StructuralUnit			*m_currentStructuralUnit = nullptr;
};

#endif // SVSTRUCTURALUNITCREATIONDIALOG_H
