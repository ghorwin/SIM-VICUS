#ifndef SVNavigationTreeWidgetH
#define SVNavigationTreeWidgetH

#include <QWidget>

namespace Ui {
class SVNavigationTreeWidget;
}

class ModificationInfo;

/*! The tree widget with all building/rooms/etc.. */
class SVNavigationTreeWidget : public QWidget {
	Q_OBJECT

public:
	explicit SVNavigationTreeWidget(QWidget *parent);
	~SVNavigationTreeWidget();

public slots:

	/*! Connected to SVProjectHandler::modified() */
	void onModified( int modificationType, ModificationInfo * data );

private:

	/*! Ui pointer. */
	Ui::SVNavigationTreeWidget			*m_ui;

};

#endif // SVNavigationTreeWidgetH

