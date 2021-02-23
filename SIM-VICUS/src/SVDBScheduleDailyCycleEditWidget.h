#ifndef SVDBSCHEDULEDAILYCYCLEEDITWIDGET_H
#define SVDBSCHEDULEDAILYCYCLEEDITWIDGET_H

#include <QWidget>

namespace Ui {
class SVDBScheduleDailyCycleEditWidget;
}

namespace VICUS {
class DailyCycle;
}

class SVDatabase;

class QItemDelegate;

class SVDBScheduleDailyCycleEditWidget : public QWidget
{
	Q_OBJECT

public:
	explicit SVDBScheduleDailyCycleEditWidget(QWidget *parent = nullptr);
	~SVDBScheduleDailyCycleEditWidget();

	void updateInput(VICUS::DailyCycle *dc, SVDatabase *db);

private slots:
	void on_tableWidgetDayCycle_currentCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn);

	void on_tableWidgetDayCycle_cellChanged(int row, int column);

private:
	Ui::SVDBScheduleDailyCycleEditWidget *m_ui;

	/*! Cached pointer to actual daily cycle. */
	VICUS::DailyCycle					*m_dailyCycle;

	/*! Cached pointer to database object. */
	SVDatabase							*m_db;

	/*! Cached pointer to item delegate. */
	QItemDelegate						*m_delegate;
};

#endif // SVDBSCHEDULEDAILYCYCLEEDITWIDGET_H
