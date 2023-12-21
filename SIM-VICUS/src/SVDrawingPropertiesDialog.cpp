#include "SVDrawingPropertiesDialog.h"
#include "ui_SVDrawingPropertiesDialog.h"

SVDrawingPropertiesDialog::SVDrawingPropertiesDialog(QWidget *parent) :
	QDialog(parent),
	m_ui(new Ui::SVDrawingPropertiesDialog)
{
	m_ui->setupUi(this);
}

SVDrawingPropertiesDialog::~SVDrawingPropertiesDialog()
{
	delete m_ui;
}

bool SVDrawingPropertiesDialog::showDrawingProperties(QWidget *parent, VICUS::Drawing *drawing) {

	SVDrawingPropertiesDialog dlg(parent);

	double oldScalingFactor = drawing->m_scalingFactor;

	double xValueOld = drawing->m_origin.m_x;
	double yValueOld = drawing->m_origin.m_y;
	double zValueOld = drawing->m_origin.m_z;

	dlg.m_ui->lineEditScalingFactor->setText(QString("%1").arg(oldScalingFactor, 0, 'f', 3));

	dlg.m_ui->lineEditX->setText(QString("%1").arg(xValueOld, 0, 'f', 3));
	dlg.m_ui->lineEditY->setText(QString("%1").arg(yValueOld, 0, 'f', 3));
	dlg.m_ui->lineEditZ->setText(QString("%1").arg(zValueOld, 0, 'f', 3));

	int res = dlg.exec();

	bool ok;
	double newScalingFactor = dlg.m_ui->lineEditScalingFactor->text().toDouble(&ok);
	if (ok && oldScalingFactor != newScalingFactor)
		drawing->m_scalingFactor = newScalingFactor;

	double newXValue = dlg.m_ui->lineEditX->text().toDouble(&ok);
	if (ok && xValueOld != newXValue)
		drawing->m_origin.m_x = newXValue;

	double newYValue = dlg.m_ui->lineEditY->text().toDouble(&ok);
	if (ok && yValueOld != newYValue)
		drawing->m_origin.m_y = newYValue;

	double newZValue = dlg.m_ui->lineEditZ->text().toDouble(&ok);
	if (ok && zValueOld != newZValue)
		drawing->m_origin.m_z = newZValue;

	return res == QDialog::Accepted;
}
