#ifndef SVFloorManagerWidgetH
#define SVFloorManagerWidgetH

#include <QWidget>

namespace Ui {
class SVFloorManagerWidget;
}

class ModificationInfo;

/*! A widget to edit buildings/building levels and associate rooms with building levels. */
class SVFloorManagerWidget : public QWidget {
	Q_OBJECT

public:
	explicit SVFloorManagerWidget(QWidget *parent = nullptr);
	~SVFloorManagerWidget();


public slots:

	/*! Connected to SVProjectHandler::modified() */
	void onModified( int modificationType, ModificationInfo * data );

private slots:
	void on_treeWidget_itemSelectionChanged();

private:
	Ui::SVFloorManagerWidget *m_ui;
};

#endif // SVFloorManagerWidgetH
