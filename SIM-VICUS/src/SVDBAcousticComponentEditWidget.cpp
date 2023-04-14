#include "SVDBAcousticComponentEditWidget.h"
#include "ui_SVDBAcousticComponentEditWidget.h"


#include <QtExt_LanguageHandler.h>

#include "SVSettings.h"
#include "SVDBAcousticComponentTableModel.h"
#include "SVConstants.h"
#include "SVMainWindow.h"

#include <IBK_math.h>
#include <IBK_StringUtils.h>



SVDBAcousticComponentEditWidget::SVDBAcousticComponentEditWidget(QWidget *parent) :
	SVAbstractDatabaseEditWidget(parent),
	m_ui(new Ui::SVDBAcousticComponentEditWidget)
{
	m_ui->setupUi(this);
	m_ui->masterLayout->setMargin(4);

	m_ui->lineEditName->initLanguages(QtExt::LanguageHandler::instance().langId().toStdString(), THIRD_LANGUAGE, true);
	m_ui->lineEditName->setDialog3Caption(tr("Acoustic component identification name"));

	m_ui->pushButtonColor->setDontUseNativeDialog(SVSettings::instance().m_dontUseNativeDialogs);

	updateInput(-1);
}

void SVDBAcousticComponentEditWidget::setup(SVDatabase * db, SVAbstractDatabaseTableModel * dbModel) {
	m_db = db;
	m_dbModel = dynamic_cast<SVDBAcousticComponentTableModel*>(dbModel);
}

void SVDBAcousticComponentEditWidget::updateInput(int id) {
	m_current = nullptr; // disable edit triggers

	if (id == -1) {
		// disable all controls
		setEnabled(false);

		// clear input controls
		m_ui->lineEditName->setString(IBK::MultiLanguageString());

		// construction property info fields
		m_ui->lineEditISV->setText("");
		m_ui->lineEditASRV->setText("");

		m_ui->pushButtonColor->setColor(Qt::black);

		return;
	}
	// re-enable all controls
	setEnabled(true);

	VICUS::AcousticComponent * acComp = const_cast<VICUS::AcousticComponent*>(m_db->m_acousticComponents[(unsigned int)id]);
	m_current = acComp;

	// now update the GUI controls
	m_ui->lineEditName->setString(acComp->m_displayName);
	m_ui->pushButtonColor->blockSignals(true);
	m_ui->pushButtonColor->setColor(m_current->m_color);
	m_ui->pushButtonColor->blockSignals(false);

	m_ui->lineEditISV->setText(QString("%L1").arg(m_current->m_impactSoundValue, 0, 'f', 4));
	m_ui->lineEditASRV->setText(QString("%L1").arg(m_current->m_airSoundResistenceValue, 0, 'f', 4));


	// for built-ins, disable editing/make read-only
	bool isEditable = !acComp->m_builtIn;
	m_ui->lineEditName->setReadOnly(!isEditable);
	m_ui->pushButtonColor->setReadOnly(!isEditable);
	m_ui->lineEditISV->setReadOnly(!isEditable);
	m_ui->lineEditASRV->setReadOnly(!isEditable);

}


void SVDBAcousticComponentEditWidget::on_pushButtonColor_colorChanged() {
	Q_ASSERT(m_current != nullptr);

	if (m_current->m_color != m_ui->pushButtonColor->color()) {
		m_current->m_color = m_ui->pushButtonColor->color();
		modelModify();
	}

}

void SVDBAcousticComponentEditWidget::modelModify(){
	m_db->m_acousticComponents.m_modified = true;
	m_dbModel->setItemModified(m_current->m_id); // tell model that we changed the data
	SVProjectHandler::instance().setModified(SVProjectHandler::ComponentInstancesModified);
}



SVDBAcousticComponentEditWidget::~SVDBAcousticComponentEditWidget() {
	delete m_ui;
}

void SVDBAcousticComponentEditWidget::on_lineEditName_editingFinished() {
	Q_ASSERT(m_current != nullptr);

	qDebug() << "Editing finished.";

	if (m_current->m_displayName != m_ui->lineEditName->string()) {
		m_current->m_displayName = m_ui->lineEditName->string();
		modelModify();
	}
}


void SVDBAcousticComponentEditWidget::on_lineEditISV_editingFinishedSuccessfully() {
	Q_ASSERT(m_current != nullptr);

	qDebug() << "Editing finished.";


	if (!IBK::nearly_equal<4>(m_current->m_impactSoundValue, IBK::string2val<double>(m_ui->lineEditISV->text().toStdString()))) {
		m_current->m_impactSoundValue = IBK::string2val<double>(m_ui->lineEditISV->text().toStdString());
		modelModify();
	}
}


void SVDBAcousticComponentEditWidget::on_lineEditASRV_editingFinishedSuccessfully() {
	Q_ASSERT(m_current != nullptr);

	qDebug() << "Editing finished.";


	if (!IBK::nearly_equal<4>(m_current->m_airSoundResistenceValue, IBK::string2val<double>(m_ui->lineEditASRV->text().toStdString()))) {
		m_current->m_airSoundResistenceValue = IBK::string2val<double>(m_ui->lineEditASRV->text().toStdString());
		modelModify();
	}
}

