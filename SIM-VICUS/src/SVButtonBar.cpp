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
	/// \todo move styling to special style class
	btn->setIcon(QIcon(iconFile));
	btn->setIconSize(QSize(32,32));
	btn->setAutoRaise(true);
	btn->setToolTip(hint);
}

void setupToolButton(QToolButton * btn) {
	/// \todo move styling to special style class
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
	toolButtonViewPostProc = new QToolButton(this); lay->addWidget(toolButtonViewPostProc);
	lay->addStretch(1);
	toolButtonQuit = new QToolButton(this); lay->addWidget(toolButtonQuit);

	setupToolButton(toolButtonAbout);
	setupToolButton(toolButtonNew);
	setupToolButton(toolButtonLoad);
	setupToolButton(toolButtonSave);

	setupToolButton(toolButtonUndo);
	setupToolButton(toolButtonRedo);
	setupToolButton(toolButtonViewPostProc);

	setupToolButton(toolButtonQuit);

	// geometry view is the default
	m_currentView = GeometryView;

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


