/*	QtExt - Qt-based utility classes and functions (extends Qt library)

	Copyright (c) 2014-today, Institut für Bauklimatik, TU Dresden, Germany

	Primary authors:
	  Heiko Fechner    <heiko.fechner -[at]- tu-dresden.de>
	  Andreas Nicolai

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.

	Dieses Programm ist Freie Software: Sie können es unter den Bedingungen
	der GNU General Public License, wie von der Free Software Foundation,
	Version 3 der Lizenz oder (nach Ihrer Wahl) jeder neueren
	veröffentlichten Version, weiter verteilen und/oder modifizieren.

	Dieses Programm wird in der Hoffnung bereitgestellt, dass es nützlich sein wird, jedoch
	OHNE JEDE GEWÄHR,; sogar ohne die implizite
	Gewähr der MARKTFÄHIGKEIT oder EIGNUNG FÜR EINEN BESTIMMTEN ZWECK.
	Siehe die GNU General Public License für weitere Einzelheiten.

	Sie sollten eine Kopie der GNU General Public License zusammen mit diesem
	Programm erhalten haben. Wenn nicht, siehe <https://www.gnu.org/licenses/>.
*/

#include "QtExt_FeatureWidget.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QSettings>
#include <QList>
#include <QSplitter>
#include <QDebug>

#include "QtExt_ActiveLabel.h"
#include "QtExt_IconButton.h"


namespace QtExt {


FeatureWidget::FeatureWidget(QWidget * parent, QWidget * content, int visibleButtons) :
	QWidget(parent),
	m_contentWidget(content),
	m_exclusive(false),
	m_previousShadeState(false)
{
	m_shadeButton = new IconButton(this);
	m_shadeButton->setNormalIcons(
		QPixmap(QString::fromUtf8(":/gfx/featureWidget/arrow_16x16/right/background/arrow_right_middle.png")),
		QPixmap(QString::fromUtf8(":/gfx/featureWidget/arrow_16x16/right/background/arrow_right_light.png")),
		QPixmap(QString::fromUtf8(":/gfx/featureWidget/arrow_16x16/right/background/arrow_right_light.png"))
	);
	m_shadeButton->setCheckedIcons(
		QPixmap(QString::fromUtf8(":/gfx/featureWidget/arrow_16x16/down/background/arrow_down_middle_background.png")),
		QPixmap(QString::fromUtf8(":/gfx/featureWidget/arrow_16x16/down/background/arrow_down_light_background.png")),
		QPixmap(QString::fromUtf8(":/gfx/featureWidget/arrow_16x16/down/background/arrow_down_light_background.png"))
	);
	m_shadeButton->setCheckable(true);
	connect(m_shadeButton, SIGNAL(clicked(bool)), this, SLOT(on_shadeButton_clicked(bool)));

	m_recycleButton = new IconButton(this);
	m_recycleButton->setNormalIcons(
		QPixmap(QString::fromUtf8(":/gfx/featureWidget/recycle_16x16/03_recycle_enabled_background_16x16.png")),
		QPixmap(QString::fromUtf8(":/gfx/featureWidget/recycle_16x16/04_recycle_hover_background_16x16.png")),
		QPixmap(QString::fromUtf8(":/gfx/featureWidget/recycle_16x16/01_recycle_disabled_background_16x16.png"))
	);

	m_recycleButton->setCheckedIcons(
		QPixmap(),
		QPixmap(QString::fromUtf8(":/gfx/featureWidget/recycle_16x16/05_recycle_enabled_checked_background_16x16.png")),
		QPixmap(QString::fromUtf8(":/gfx/featureWidget/recycle_16x16/06_recycle_hover_checked_background_16x16.png"))
	);

	connect(m_recycleButton, SIGNAL(clicked()), this, SLOT(on_recycleButton_clicked()));

	m_copyButton = new IconButton(this);
	m_copyButton->setNormalIcons(
		QPixmap(QString::fromUtf8(":/gfx/featureWidget/copy_16x16/circular/03_copy_enabled_background_16x16.png")),
		QPixmap(QString::fromUtf8(":/gfx/featureWidget/copy_16x16/circular/04_copy_hover_background_16x16.png")),
		QPixmap(QString::fromUtf8(":/gfx/featureWidget/copy_16x16/circular/01_copy_disabled_background_16x16.png"))
	);
	m_copyButton->setCheckedIcons(
		QPixmap(),
		QPixmap(QString::fromUtf8(":/gfx/featureWidget/copy_16x16/circular/05_copy_enabled_checked_background_16x16.png")),
		QPixmap(QString::fromUtf8(":/gfx/featureWidget/copy_16x16/circular/06_copy_hover_checked_background_16x16.png"))
	);
	connect(m_copyButton, SIGNAL(clicked()), this, SLOT(on_copyButton_clicked()));

	m_plusButton = new IconButton(this);
	m_plusButton->setNormalIcons(
		QPixmap(QString::fromUtf8(":/gfx/featureWidget/plus_16x16/03_plus_enabled_background_16x16.png")),
		QPixmap(QString::fromUtf8(":/gfx/featureWidget/plus_16x16/04_plus_hover_background_16x16.png")),
		QPixmap(QString::fromUtf8(":/gfx/featureWidget/plus_16x16/01_plus_disabled_background_16x16.png"))
	);
	m_plusButton->setCheckedIcons(
		QPixmap(),
		QPixmap(QString::fromUtf8(":/gfx/featureWidget/plus_16x16/05_plus_enabled_checked_background_16x16.png")),
		QPixmap(QString::fromUtf8(":/gfx/featureWidget/plus_16x16/06_plus_hover_checked_background_16x16.png"))
	);
	connect(m_plusButton, SIGNAL(clicked()), this, SLOT(on_plusButton_clicked()));

	m_minusButton = new IconButton(this);
	m_minusButton->setShortCut(QKeySequence(Qt::Key_Delete));
	m_minusButton->setNormalIcons(
		QPixmap(QString::fromUtf8(":/gfx/featureWidget/minus_16x16/03_minus_enabled_background_16x16.png")),
		QPixmap(QString::fromUtf8(":/gfx/featureWidget/minus_16x16/04_minus_hover_background_16x16.png")),
		QPixmap(QString::fromUtf8(":/gfx/featureWidget/minus_16x16/01_minus_disabled_background_16x16.png"))
	);
	m_minusButton->setCheckedIcons(
		QPixmap(),
		QPixmap(QString::fromUtf8(":/gfx/featureWidget/minus_16x16/05_minus_enabled_checked_background_16x16.png")),
		QPixmap(QString::fromUtf8(":/gfx/featureWidget/minus_16x16/06_minus_hover_checked_background_16x16.png"))
	);
	connect(m_minusButton, SIGNAL(clicked()), this, SLOT(on_minusButton_clicked()));

	m_moveUpButton = new IconButton(this);
	m_moveUpButton->setShortCut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_Up));
	m_moveUpButton->setNormalIcons(
		QPixmap(QString::fromUtf8(":/gfx/featureWidget/moveButtons_16x16/up.png")),
		QPixmap(QString::fromUtf8(":/gfx/featureWidget/moveButtons_16x16/up.png")),
		QPixmap(QString::fromUtf8(":/gfx/featureWidget/moveButtons_16x16/up-disabled.png"))
	);
	m_moveUpButton->setCheckedIcons(
		QPixmap(QString::fromUtf8(":/gfx/featureWidget/moveButtons_16x16/up.png")),
		QPixmap(QString::fromUtf8(":/gfx/featureWidget/moveButtons_16x16/up.png")),
		QPixmap(QString::fromUtf8(":/gfx/featureWidget/moveButtons_16x16/up-disabled.png"))
	);
	connect(m_moveUpButton, SIGNAL(clicked()), this, SLOT(on_moveUpButton_clicked()));

	m_moveDownButton = new IconButton(this);
	m_moveDownButton->setShortCut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_Up));
	m_moveDownButton->setNormalIcons(
		QPixmap(QString::fromUtf8(":/gfx/featureWidget/moveButtons_16x16/down.png")),
		QPixmap(QString::fromUtf8(":/gfx/featureWidget/moveButtons_16x16/down.png")),
		QPixmap(QString::fromUtf8(":/gfx/featureWidget/moveButtons_16x16/down-disabled.png"))
	);
	m_moveDownButton->setCheckedIcons(
		QPixmap(QString::fromUtf8(":/gfx/featureWidget/moveButtons_16x16/down.png")),
		QPixmap(QString::fromUtf8(":/gfx/featureWidget/moveButtons_16x16/down.png")),
		QPixmap(QString::fromUtf8(":/gfx/featureWidget/moveButtons_16x16/down-disabled.png"))
	);
	connect(m_moveDownButton, SIGNAL(clicked()), this, SLOT(on_moveDownButton_clicked()));


	m_titleLabel = new ActiveLabel(this);
	m_titleLabel->setMaximumHeight(20);
	connect(m_titleLabel, SIGNAL(clicked(bool)), this, SLOT(on_shadeButton_clicked(bool)));

	// if no widget was given, use a standard widget
	if (m_contentWidget == NULL) {

		m_contentWidget = new QWidget(this);
#ifdef DSIX_ENABLE_STYLES
		QPalette pal;
		pal.setColor(QPalette::Window, QColor(57,57,57,255));// QColor(73,73,73,255)); //Qt::gray); // QColor(71,71,71,255));
		pal.setColor(QPalette::WindowText, QColor(161,161,161,255));
		pal.setColor(QPalette::ButtonText, QColor(161,161,161,255));
		pal.setColor(QPalette::Text, QColor(161,161,161,255));
		m_contentWidget->setPalette(pal);
		m_contentWidget->setAutoFillBackground(true);
#endif // DSIX_ENABLE_STYLES
		QVBoxLayout *h2 = new QVBoxLayout();
		h2->setObjectName("Layout");
		QLabel *myLabel = new QLabel("content\nplaceholder");
		myLabel->setObjectName("Label");
		h2->addWidget(myLabel);
		m_contentWidget->setLayout(h2);
		m_contentWidget->setMinimumSize(16,16);
		m_contentWidget->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

	}
	else {
#ifdef DSIX_ENABLE_STYLES
		QPalette pal;
		pal.setColor(QPalette::Window, QColor(57,57,57,255));// QColor(73,73,73,255)); //Qt::gray); // QColor(71,71,71,255));
		pal.setColor(QPalette::WindowText, QColor(161,161,161,255));
		pal.setColor(QPalette::ButtonText, QColor(161,161,161,255));
		pal.setColor(QPalette::Text, QColor(161,161,161,255));
		m_contentWidget->setPalette(pal);
		m_contentWidget->setAutoFillBackground(true);
#endif // DSIX_ENABLE_STYLES
		m_contentWidget = content;
	}

	// create controll area
	QWidget * sizeFix = new QWidget();
	sizeFix->setSizePolicy( QSizePolicy::MinimumExpanding, QSizePolicy::Fixed );
	sizeFix->setMinimumHeight( 20 );
	sizeFix->setMaximumHeight( 20 );
//	QIcon iconBtn;
#ifdef DSIX_ENABLE_STYLES
	QPalette widgetpal;
	widgetpal.setColor(QPalette::Window,  QColor(73,73,73,255));//QColor(57,57,57,255));// Qt::darkGray); //QColor(41,41,41,255));
	widgetpal.setColor(QPalette::WindowText, Qt::white);
	widgetpal.setColor(QPalette::ButtonText, Qt::white);
	setPalette(widgetpal);
	setAutoFillBackground(true);
#else // DSIX_ENABLE_STYLES
	QPalette pal;
	pal.setColor(QPalette::Window,  QColor(73,73,73,255));
	sizeFix->setPalette(pal);
	sizeFix->setAutoFillBackground(true);
#endif // DSIX_ENABLE_STYLES

	// compose title bar
	QHBoxLayout * hlay = new QHBoxLayout(sizeFix);
	hlay->setSpacing(4);
	hlay->addWidget(m_shadeButton);
	hlay->addSpacing(5);
	hlay->addWidget(m_titleLabel);
	hlay->addStretch();
	hlay->addWidget(m_moveUpButton);
	hlay->addWidget(m_moveDownButton);
	hlay->addWidget(m_plusButton);
	hlay->addWidget(m_minusButton);
	hlay->addWidget(m_copyButton);
	hlay->addWidget(m_recycleButton);
	hlay->setContentsMargins(0,0,2,0);

	// compose everything
	m_vlay = new QVBoxLayout;
	m_vlay->addWidget(sizeFix, 0);
	m_vlay->addWidget(m_contentWidget, 1);
	m_vlay->setSpacing(0);
	m_vlay->setContentsMargins(0,0,0,0);
	setLayout(m_vlay);

	setVisibleButtons(visibleButtons);

	// set feature widget to expanded mode by default
	m_contentWidget->setVisible(false); // so that expand() works
	expand();
	setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Minimum);
//	qDebug() << "SizePolicy=" << sizePolicy();
}


FeatureWidget::~FeatureWidget(){

	delete	m_shadeButton;
	delete	m_plusButton;
	delete	m_minusButton;
	delete	m_copyButton;
	delete	m_recycleButton;
	delete	m_moveUpButton;
	delete	m_moveDownButton;

	m_shadeButton = NULL;
	m_plusButton = NULL;
	m_minusButton = NULL;
	m_copyButton = NULL;
	m_recycleButton = NULL;
	m_moveUpButton = NULL;
	m_moveDownButton = NULL;

}

void FeatureWidget::setEnabled( bool enabled ){

	m_copyButton->setEnabled( enabled );
	m_plusButton->setEnabled( enabled );
	m_minusButton->setEnabled( enabled );
	m_recycleButton->setEnabled( enabled );
	m_moveUpButton->setEnabled( enabled );
	m_moveDownButton->setEnabled( enabled );
	m_contentWidget->setEnabled( enabled );

}

void FeatureWidget::setVisibleButtons(int visibleButtons) {
	m_shadeButton->setVisible(visibleButtons & ShadeButton);
	m_copyButton->setVisible(visibleButtons & CopyButton);
	m_plusButton->setVisible(visibleButtons & PlusButton);
	m_minusButton->setVisible(visibleButtons & MinusButton);
	m_moveUpButton->setVisible(visibleButtons & MoveUpButton);
	m_moveDownButton->setVisible(visibleButtons & MoveDownButton);
	m_recycleButton->setVisible(visibleButtons & RecycleButton);
}

void FeatureWidget::setContentWidget(QWidget * w) {
	Q_ASSERT(w != NULL);

	//qDebug() << "[FeatureWidget::setContentWidget] Number of widgets in layout before delete:" << m_vlay->count();
	// this deletes the widget, all its child widgets and removes it from its parent and parent layout
	// if this causes an access violation somewhere, the problem is likely to be a dangling signal/slot connection
	delete m_contentWidget;
	//qDebug() << "[FeatureWidget::setContentWidget] Number of widgets in layout after delete:" << m_vlay->count();

	// set new widget
	m_contentWidget = w;

	// set appropriate palette
#ifdef DSIX_USE_STYLES
	QPalette pal;
	pal.setColor(QPalette::Window, QColor(57,57,57,255));//Qt::gray);
	pal.setColor(QPalette::WindowText, QColor(161,161,161,255));
	pal.setColor(QPalette::ButtonText, QColor(161,161,161,255));
	pal.setColor(QPalette::Text, QColor(161,161,161,255));
	m_contentWidget->setAutoFillBackground(true);
	m_contentWidget->setPalette(pal);
#endif // DSIX_USE_STYLES
	m_contentWidget->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Expanding);

	//set new widget
	m_vlay->addWidget(m_contentWidget,1);

	// set feature widget content to expanded mode by default
	expand();
}

QWidget * FeatureWidget::contentWidget() const {
	return m_contentWidget;
}

IconButton * FeatureWidget::shadeButton() const {
	return m_shadeButton;
}

IconButton * FeatureWidget::recycleButton() const {
	return m_recycleButton;
}

IconButton * FeatureWidget::moveUpButton() const {
	return m_moveUpButton;
}

IconButton * FeatureWidget::moveDownButton() const {
	return m_moveDownButton;
}

IconButton * FeatureWidget::copyButton() const {
	return m_copyButton;
}

IconButton * FeatureWidget::plusButton() const {
	return m_plusButton;
}

IconButton * FeatureWidget::minusButton() const {
	return m_minusButton;
}

void FeatureWidget::setTitle(const QString & titleText) {
	m_titleLabel->setText(titleText);
}

QString FeatureWidget::title() const {
	return m_titleLabel->text();
}

bool FeatureWidget::isCollapsed(){
	return m_shadeButton->isChecked();
}

void FeatureWidget::setExclusive(bool is_exclusive) {
	m_exclusive = is_exclusive;
}

bool FeatureWidget::isExclusive() const {
	return m_exclusive;
}

void FeatureWidget::writeSettings( QSettings& settings ) {
	settings.setValue( QString("%1checked").arg(title()), m_shadeButton->isChecked() );
}

void FeatureWidget::readSettings( QSettings& settings ) {
	Q_ASSERT(m_contentWidget != NULL);

	bool on = settings.value( QString("%1checked").arg(title()), m_shadeButton->isChecked() ).toBool();
	m_shadeButton->setChecked( on );
	m_contentWidget->setVisible(on);
}

void FeatureWidget::expand() {
	if (m_contentWidget->isVisibleTo(this)) return; // already expanded
	//qDebug() << "[FeatureWidget::expand()]" << size();
	// reset maximum size of widget to something very large
	setMaximumHeight(20000);
	m_contentWidget->setVisible(true);
	if (m_exclusive) {
		QWidget * w = dynamic_cast<QWidget * >(parent());
		// this could be an Q_ASSERT, because FeatureWidgets _must_ have a parent widget!
		if (w != NULL) {
			// loop over all
			QLayout * lay = w->layout();
			for (int i=0; i<lay->count(); ++i) {
				FeatureWidget * fw = dynamic_cast<FeatureWidget * >(lay->itemAt(i)->widget());
				if (fw == NULL) continue;
				// skip current widget
				if (fw == this) continue;
				// hide only exclusive widgets
				if (fw->isExclusive())
					fw->collapse();
			}
		}
	}
	// we need to reset the button if the label emitted the button clicked signal
	if (!m_shadeButton->isChecked()) m_shadeButton->setChecked(true);
	// since expand is public we have to save state here
	m_previousShadeState=true;
	emit expanded( this );
}

void FeatureWidget::collapse() {
	if (!m_contentWidget->isVisibleTo(this)) return; // already collapsed
	//qDebug() << "[FeatureWidget::collapse()]" << size() << sizeHint();
	m_contentWidget->setVisible(false);
	// we need to reset the button if the label emitted the button clicked signal
	if (m_shadeButton->isChecked()) m_shadeButton->setChecked(false);
	// since colapse is public we have to save state here
	m_previousShadeState=false;
	// limit vertical size of feature widget to shaded state
	setMaximumHeight(qMax(20, m_titleLabel->height()));
	emit collapsed( this );
}

void FeatureWidget::on_recycleButton_clicked() {
	emit recycleClicked();
}

void FeatureWidget::on_copyButton_clicked() {
	emit copyClicked();
}

void FeatureWidget::on_plusButton_clicked() {
	emit plusClicked();
}

void FeatureWidget::on_minusButton_clicked() {
	emit minusClicked();
}

void FeatureWidget::on_moveUpButton_clicked() {
	emit moveUpClicked();
}

void FeatureWidget::on_moveDownButton_clicked() {
	emit moveDownClicked();
}

void FeatureWidget::on_shadeButton_clicked(bool checked) {

	Q_UNUSED(checked);

	// if button is not visible, we ignore the event
	if (!m_shadeButton->isVisibleTo(this))
		return;

	// user can not uncheck a featureWidget that is exclusive
	if (m_previousShadeState && m_exclusive)
		return;

	if (m_contentWidget->isVisibleTo(this))
		collapse();
	else
		expand();
}


} // namespace QtExt
