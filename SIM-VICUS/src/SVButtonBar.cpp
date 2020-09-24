#include "SVButtonBar.h"

#include <QVBoxLayout>
#include <QToolButton>
#include <QIcon>
#include <QSpacerItem>
#include <QMenu>
#include <QAction>
#include <QMessageBox>
#include <QTranslator>
#include <QKeySequence>

#include <QtExt_LanguageHandler.h>
#include <VICUS_Constants.h>

void setupToolButton(QToolButton * btn, const QString & iconFile, const QString & hint) {
	btn->setIcon(QIcon(iconFile));
	btn->setIconSize(QSize(32,32));
	btn->setAutoRaise(true);
	btn->setToolTip(hint);
}

void setupToolButton(QToolButton * btn) {
	btn->setIconSize(QSize(32,32));
	btn->setAutoRaise(true);
}


SVButtonBar::SVButtonBar(QWidget * parent) :
	QWidget(parent)
{
	QVBoxLayout * lay  = new QVBoxLayout(this);

	// create tool buttons and assign resource files
	toolButtonAbout = new QToolButton(this); lay->addWidget(toolButtonAbout);

	toolButtonNew = new QToolButton(this); lay->addWidget(toolButtonNew);
	toolButtonLoad = new QToolButton(this); lay->addWidget(toolButtonLoad);
	toolButtonSave = new QToolButton(this); lay->addWidget(toolButtonSave);
	lay->addSpacerItem( new QSpacerItem(20,20, QSizePolicy::Preferred, QSizePolicy::Fixed) );
	toolButtonUndo = new QToolButton(this); lay->addWidget(toolButtonUndo);
	toolButtonRedo = new QToolButton(this); lay->addWidget(toolButtonRedo);
	lay->addSpacerItem( new QSpacerItem(20,20, QSizePolicy::Preferred, QSizePolicy::Fixed) );

	lay->addStretch(1);
	toolButtonSwitchLanguage = new QToolButton(this); lay->addWidget(toolButtonSwitchLanguage);
	toolButtonQuit = new QToolButton(this); lay->addWidget(toolButtonQuit);

	setupToolButton(toolButtonAbout);
	setupToolButton(toolButtonNew);
	setupToolButton(toolButtonLoad);
	setupToolButton(toolButtonSave);

	setupToolButton(toolButtonUndo);
	setupToolButton(toolButtonRedo);

	setupToolButton(toolButtonSwitchLanguage, ":/gfx/actions/32x32/switch_language_32x32.png", tr("Change currently used language."));
	setupToolButton(toolButtonQuit);

	// open context menu with languages
	m_languageMenu = new QMenu(this);
	// language menu will be populated in SVMainWindow::addLanguageAction()

	// geometry view is the default
	m_currentView = GeometryView;

	connect(toolButtonSwitchLanguage, SIGNAL(clicked()),
			this, SLOT(onToolButtonSwitchLanguageClicked()));

	lay->setMargin(4);
	setLayout(lay);
}


SVButtonBar::~SVButtonBar() {
}


void SVButtonBar::setCurrentView(Views view) {
	m_currentView = view;

	switch (view) {
		case ProjectView :
//			toolButtonViewGeometry->defaultAction()->setChecked(false);
//			toolButtonViewSimulate->defaultAction()->setChecked(false);
		break;

		case GeometryView :
//			toolButtonViewProjectInformation->defaultAction()->setChecked(false);
//			toolButtonViewSimulate->defaultAction()->setChecked(false);
		break;

		case SimulationView :
//			toolButtonViewGeometry->defaultAction()->setChecked(false);
//			toolButtonViewProjectInformation->defaultAction()->setChecked(false);
		break;

		default : Q_ASSERT(false);
	}

	emit currentViewChanged(view);
}


void SVButtonBar::onToolButtonSwitchLanguageClicked() {
	m_languageMenu->popup(mapToGlobal(toolButtonSwitchLanguage->pos()));
}

