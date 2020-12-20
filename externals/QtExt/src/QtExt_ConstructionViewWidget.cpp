#include "QtExt_ConstructionViewWidget.h"
#include "ui_QtExt_ConstructionViewWidget.h"

#include <QDebug>
#include <QScreen>
#include <QApplication>

namespace QtExt {

ConstructionViewWidget::ConstructionViewWidget(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::ConstructionViewWidget),
	m_toolBar(new QToolBar),
	m_fixed(true)
{
	ui->setupUi(this);

	setupUI();
	enableToolBar(false);

	// send signal from graphicsview to outside
	connect(ui->graphicsView, SIGNAL(layerSelected(int)), this, SLOT(onLayerSelected(int)));
	connect(ui->graphicsView, SIGNAL(layerDoubleClicked(int)), this, SLOT(onLayerDoubleClicked(int)));
}

ConstructionViewWidget::~ConstructionViewWidget() {
	delete ui;
}

void ConstructionViewWidget::setData(const QVector<ConstructionLayer>& layers, bool fixed) {
	m_fixed = fixed;
	enableToolBar(false);
	ui->graphicsView->setData(this, layers, 1.0);
	ui->graphicsView->update();
}

void ConstructionViewWidget::setToolbarVisible(bool visible) {
	m_toolBar->setVisible(visible);
}

void ConstructionViewWidget::clear() {
	enableToolBar(false);
	ui->graphicsView->clear();
}

void ConstructionViewWidget::selectLayer(int index) {
	ui->graphicsView->selectLayer(index);
}

void ConstructionViewWidget::setupUI() {
//	ui->verticalLayout->removeWidget(ui->widgetToolBar);
	ui->verticalLayout->insertWidget(0, m_toolBar);

	m_toolBar->addAction(ui->actionAssign_material);
	m_toolBar->addAction(ui->actionInsert_layer_left);
	m_toolBar->addAction(ui->actionInsert_layer_right);
	m_toolBar->addAction(ui->actionMove_layer_left);
	m_toolBar->addAction(ui->actionMove_layer_right);
	m_toolBar->addAction(ui->actionRemove_layer);

#if QT_VERSION >= 0x050a00
	QScreen* screen = QApplication::screenAt(QPoint(0,0));
#else
	QList<QScreen *> screens = QApplication::screens();
	QScreen* screen = screens.empty() ? nullptr : screens.front();
#endif
	Q_ASSERT(screen != nullptr);
	qreal dpi = screen->logicalDotsPerInch();
	if(dpi <= 100) {
		m_toolBar->setFixedHeight(34);
		m_toolBar->setIconSize(QSize(32,32));
	}
	else {
		m_toolBar->setFixedHeight(50);
		m_toolBar->setIconSize(QSize(48,48));
	}
}

void ConstructionViewWidget::enableToolBar(bool enable) {
	if(m_fixed)
		enable = false;

	ui->actionAssign_material->setEnabled(enable);
	ui->actionInsert_layer_left->setEnabled(enable);
	ui->actionInsert_layer_right->setEnabled(enable);
	ui->actionMove_layer_left->setEnabled(enable);
	ui->actionMove_layer_right->setEnabled(enable);
	ui->actionRemove_layer->setEnabled(enable);
	m_toolBar->setEnabled(enable);
}

void ConstructionViewWidget::on_actionAssign_material_triggered() {
	emit assignMaterial(ui->graphicsView->selectedLayer());
}

void ConstructionViewWidget::on_actionInsert_layer_left_triggered() {
	emit insertLayer(ui->graphicsView->selectedLayer(), true);
}

void ConstructionViewWidget::on_actionInsert_layer_right_triggered() {
	emit insertLayer(ui->graphicsView->selectedLayer(), false);
}

void ConstructionViewWidget::on_actionRemove_layer_triggered() {
	emit removelayer(ui->graphicsView->selectedLayer());
}

void ConstructionViewWidget::on_actionMove_layer_left_triggered() {
	emit moveLayer(ui->graphicsView->selectedLayer(), true);
}

void ConstructionViewWidget::on_actionMove_layer_right_triggered() {
	emit moveLayer(ui->graphicsView->selectedLayer(), false);
}

void ConstructionViewWidget::onLayerSelected(int index) {
	bool validlayer = index != -1;
	enableToolBar(validlayer);
	emit layerSelected(index);
}

void ConstructionViewWidget::onLayerDoubleClicked(int index) {
//	bool validlayer = index != -1;
//	enableToolBar(validlayer);
	if(!m_fixed)
		emit assignMaterial(index);
}

} // namespace QtExt

