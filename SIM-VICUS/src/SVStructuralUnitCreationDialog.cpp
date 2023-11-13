#include "SVStructuralUnitCreationDialog.h"
#include "ui_SVStructuralUnitCreationDialog.h"
#include "VICUS_Project.h"
#include "SVProjectHandler.h"
#include "SVUndoAddStructuralUnit.h"
#include "SVUndoModifyStructuralUnit.h"

#include <VICUS_utilities.h>
#include "SVStyle.h"



SVStructuralUnitCreationDialog::SVStructuralUnitCreationDialog(QWidget *parent) :
    QDialog(parent),
	m_ui(new Ui::SVStructuralUnitCreationDialog)
{
	m_ui->setupUi(this);

}

SVStructuralUnitCreationDialog::~SVStructuralUnitCreationDialog() {
	delete m_ui;
}

bool SVStructuralUnitCreationDialog::edit(const VICUS::StructuralUnit * unit) {
	setWindowTitle(tr("Edit Structural Unit"));
	m_ui->lineEditUnitName->setText(unit->m_displayName);
	m_ui->pushButtonColor->setColor(unit->m_color);
	m_currentStructuralUnit = unit;
	return exec();
}


bool SVStructuralUnitCreationDialog::create() {
	setWindowTitle(tr("Create Structural Unit"));
	std::set<QString> existingNames;
	for (const VICUS::StructuralUnit& unit : project().m_structuralUnits)
		existingNames.insert(unit.m_displayName);
	QString defaultName = VICUS::uniqueName(tr("Unit"), existingNames);
	m_ui->lineEditUnitName->setText(defaultName);
	m_ui->pushButtonColor->setColor(SVStyle::randomColor());
	// reset structural unit
	m_currentStructuralUnit = nullptr;
	return exec();
}

void SVStructuralUnitCreationDialog::on_buttonBoxCreateUnit_accepted() {

	QString name = m_ui->lineEditUnitName->text();
	QColor color = m_ui->pushButtonColor->color();

	if(m_currentStructuralUnit != nullptr){
		// is in editing mode
		// create UNDO Action for modifications
		VICUS::StructuralUnit newUnit(*m_currentStructuralUnit);
		newUnit.m_color = color;
		newUnit.m_displayName = name;
		SVUndoModifyStructuralUnit * undo = new SVUndoModifyStructuralUnit(tr("Modifying structural unit #%1 '%2")
																		   .arg(newUnit.m_id)
																		   .arg(newUnit.m_displayName), newUnit);
		undo->push(); // this will update our tree widget
	} else {
		VICUS::StructuralUnit newUnit;
		newUnit.m_id = project().nextUnusedID();
		newUnit.m_displayName = name;
		newUnit.m_color = color;
		SVUndoAddStructuralUnit * undo = new SVUndoAddStructuralUnit(tr("Adding structural unit '%1'").arg(newUnit.m_displayName), newUnit);
		undo->push(); // this will update our tree widget
	}

}

