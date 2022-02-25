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

void ConstructionViewWidget::setData(const QVector<ConstructionLayer>& layers, bool fixed,
									 QString	leftSideLabel,
									 QString	rightSideLabel,
									 int visibleItems)
{
	m_fixed = fixed;
	enableToolBar(false);
	ui->graphicsView->m_leftSideLabel = leftSideLabel;
	ui->graphicsView->m_rightSideLabel = rightSideLabel;
	ui->graphicsView->setData(this, layers, 1.0, visibleItems);
}


void ConstructionViewWidget::updateView() {
	ui->graphicsView->updateView();
}

void ConstructionViewWidget::setToolbarVisible(bool visible) {
	m_toolBar->setVisible(visible);
}

void ConstructionViewWidget::setBackground(const QColor& bkgColor) {
	ui->graphicsView->setBackground(bkgColor);
}


void ConstructionViewWidget::markLayer(int layerIndex) {
	ui->graphicsView->markLayer(layerIndex);
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
	if (dpi <= 100) {
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
	int selectedLayerIndex = ui->graphicsView->selectedLayer();
	emit moveLayer(selectedLayerIndex, true);
	if(selectedLayerIndex > 0)
		--selectedLayerIndex;
	ui->graphicsView->selectLayer(selectedLayerIndex);
}

void ConstructionViewWidget::on_actionMove_layer_right_triggered() {
	int selectedLayerIndex = ui->graphicsView->selectedLayer();
	emit moveLayer(ui->graphicsView->selectedLayer(), false);
	if(selectedLayerIndex < ui->graphicsView->layers().size())
		++selectedLayerIndex;
	ui->graphicsView->selectLayer(selectedLayerIndex);
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

