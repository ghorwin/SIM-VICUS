#include "SVDBAcousticBoundaryConditionTableWidget.h"

#include "SVDBAcousticBoundaryConditionEditWidget.h"
#include <QHBoxLayout>
#include <QHeaderView>
#include <QDebug>
#include <QPushButton>
#include <QLabel>

SVDBAcousticBoundaryConditionTableWidget::SVDBAcousticBoundaryConditionTableWidget(QWidget *parent)
{
	setParent(parent);
	QHeaderView *horizontalHeader = this->horizontalHeader();
	connect(horizontalHeader, &QHeaderView::sectionResized, this, &SVDBAcousticBoundaryConditionTableWidget::onSectionResized);
}

void SVDBAcousticBoundaryConditionTableWidget::onSectionResized()
{
	if(columnCount() >= SVDBAcousticBoundaryConditionEditWidget::ColNameButton){
		for(int i = 0; i < rowCount(); ++i){
			QWidget * widgetButton = cellWidget(i, SVDBAcousticBoundaryConditionEditWidget::ColNameButton);
			if(widgetButton){
				int newX = columnViewportPosition(SVDBAcousticBoundaryConditionEditWidget::ColNameButton);
				int width = columnWidth(SVDBAcousticBoundaryConditionEditWidget::ColNameButton);
				widgetButton->setGeometry(newX, widgetButton->y(), width, rowHeight(i));
			}
		}

	}
}


void SVDBAcousticBoundaryConditionTableWidget::setButton(int row, int column)
{
	QPushButton *button = new QPushButton(QString("..."), this);
	button->setFixedSize(24, 21);
	button->setStyleSheet("QPushButton{padding-top: 2px; padding-bottom: 2px; padding-left: 3px; padding-right: 3px;}");
	setCellWidget(row, column, button);

	SVDBAcousticBoundaryConditionEditWidget* parent = static_cast<SVDBAcousticBoundaryConditionEditWidget*>(this->parent());
	connect(button, &QPushButton::clicked, parent, [parent, row](){
		parent->onLayerChosen(row, SVDBAcousticBoundaryConditionEditWidget::ColName);
		}
	);
}
