#ifndef SVDBWindowEditWidgetH
#define SVDBWindowEditWidgetH

#include <QWidget>

class QSortFilterProxyModel;

namespace Ui {
class SVDBWindowEditWidget;
}

class SVDBWindowEditWidget : public QWidget {
	Q_OBJECT

public:
	explicit SVDBWindowEditWidget(QWidget *parent = nullptr);
	~SVDBWindowEditWidget();

	void edit();

private slots:
	void on_toolButtonAdd_clicked();
	void on_toolButtonCopy_clicked();
	void on_toolButtonRemove_clicked();

	void on_tableWidget_itemSelectionChanged();

private:
	/*! Transfers data from database to table widget. */
	void updateUi();

	/*! Transfers data from database element with given id to
		edit widget.
	*/
	void updateEditWidget(unsigned int id);

	Ui::SVDBWindowEditWidget	*m_ui;

	QSortFilterProxyModel		*m_sortFilterModel;
};

#endif // SVDBWindowEditWidgetH
