#ifndef SVPropBuildingEditWidgetH
#define SVPropBuildingEditWidgetH

#include <QWidget>

namespace Ui {
	class SVPropBuildingEditWidget;
}

class ModificationInfo;

/*! A widget to edit site (and grid) related properties. */
class SVPropBuildingEditWidget : public QWidget {
	Q_OBJECT

public:
	explicit SVPropBuildingEditWidget(QWidget *parent = nullptr);
	~SVPropBuildingEditWidget();

	/*! Switches property widget into specific mode. */
	void setPropertyMode(int mode);

public slots:

	/*! Connected to SVProjectHandler::modified() */
	void onModified( int modificationType, ModificationInfo * data );

private:
	Ui::SVPropBuildingEditWidget *m_ui;
};


#endif // SVPropBuildingEditWidgetH
