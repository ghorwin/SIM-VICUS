#ifndef SVPropSiteWidgetH
#define SVPropSiteWidgetH

#include <QWidget>

namespace Ui {
	class SVPropSiteWidget;
}

class ModificationInfo;

/*! A widget to edit site (and grid) related properties. */
class SVPropSiteWidget : public QWidget {
	Q_OBJECT

public:
	explicit SVPropSiteWidget(QWidget *parent = nullptr);
	~SVPropSiteWidget();

public slots:

	/*! Connected to SVProjectHandler::modified() */
	void onModified( int modificationType, ModificationInfo * data );

private slots:
	void on_lineEditMaxDimensions_editingFinished();

	void on_lineEditGridLineSpacing_editingFinished();

	void on_lineEditViewDepth_editingFinished();

private:
	Ui::SVPropSiteWidget *m_ui;
};


#endif // SVPropSiteWidgetH
