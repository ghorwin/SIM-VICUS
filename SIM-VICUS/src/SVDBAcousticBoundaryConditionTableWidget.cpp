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
	if(columnCount() >= SVDBAcousticBoundaryConditionEditWidget::NumCol){
		if(columnWidth(SVDBAcousticBoundaryConditionEditWidget::ColName) < 50)
			setColumnWidth(SVDBAcousticBoundaryConditionEditWidget::ColName, 50);
		for(int i = 0; i < rowCount(); ++i){
			QWidget * widget = cellWidget(i, SVDBAcousticBoundaryConditionEditWidget::ColName);
			if(widget){
				widget->setFixedWidth(columnWidth(SVDBAcousticBoundaryConditionEditWidget::ColName));
			}
		}
	}
}

void SVDBAcousticBoundaryConditionTableWidget::setButtonAndText(int row, int column, QString text)
{
	QWidget *w = new QWidget();
	QHBoxLayout *l = new QHBoxLayout();
	l->setMargin(0);
	l->setSpacing(0);
	w->setLayout(l);
	QPushButton *button = new QPushButton(QString("..."), this);
	button->setFixedSize(24, 24);
	QString name = QString(" %1 ").arg(text);
	QLabel *nameLabel = new QLabel(name);
	nameLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	nameLabel->setMinimumWidth(100);
	l->addWidget(nameLabel);
	l->addWidget(button);
	setCellWidget(row, column, w);
	SVDBAcousticBoundaryConditionEditWidget* parent = static_cast<SVDBAcousticBoundaryConditionEditWidget*>(this->parent());
	connect(button, &QPushButton::clicked, parent, [parent, row](){
		parent->onLayerChosen(row, SVDBAcousticBoundaryConditionEditWidget::ColName);
		}
	);
}
