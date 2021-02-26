#include "SVDBScheduleAddDialog.h"
#include "ui_SVDBScheduleAddDialog.h"

#include "VICUS_Schedule.h"

#include "QtExt_LanguageHandler.h"
#include <QtExt_Conversions.h>

SVDBScheduleAddDialog::SVDBScheduleAddDialog(QWidget *parent) :
	QDialog(parent),
	m_ui(new Ui::SVDBScheduleAddDialog)
{
	m_ui->setupUi(this);

	m_ui->comboBoxInterpolationMethod->addItem(tr("linear") );
	m_ui->comboBoxInterpolationMethod->addItem(tr("constant") );

	m_ui->comboBoxScheduleType->addItem(tr("periodic (daily cycle based schemes)") );
	m_ui->comboBoxScheduleType->addItem(tr("continuous (e.g. measured data)") );
}

SVDBScheduleAddDialog::~SVDBScheduleAddDialog() {
	delete m_ui;
}

bool SVDBScheduleAddDialog::requestScheduleData(const QString &title, VICUS::Schedule &sched) {
	SVDBScheduleAddDialog dlg(nullptr); // top level

	if (title.isEmpty())
		dlg.setWindowTitle(qApp->applicationName());
	else
		dlg.setWindowTitle(title);

	dlg.m_ui->lineEditName->setString(sched.m_displayName);

	int res = dlg.exec();

	if (res != QDialog::Accepted)
		return false;
	else {
		sched.m_displayName = dlg.m_ui->lineEditName->string();
		if ( dlg.m_ui->comboBoxScheduleType->currentText() == "periodic (daily cycle based schemes)" )
			sched.m_periods.push_back(VICUS::ScheduleInterval() );
		sched.m_useLinearInterpolation = (dlg.m_ui->comboBoxInterpolationMethod->currentText() == "linear");

		return true;
	}
}
