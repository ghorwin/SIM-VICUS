#ifndef SVPropBuildingSurfaceConnectionWidgetH
#define SVPropBuildingSurfaceConnectionWidgetH

#include <QWidget>
#include <set>

namespace Ui {
	class SVPropBuildingSurfaceConnectionWidget;
}

namespace VICUS {
	class Surface;
}

/*! This page shows a table with all ComponentInstances of interconnected surfaces. */
class SVPropBuildingSurfaceConnectionWidget : public QWidget {
	Q_OBJECT

public:
	explicit SVPropBuildingSurfaceConnectionWidget(QWidget *parent = nullptr);
	~SVPropBuildingSurfaceConnectionWidget();

	/*! Updates user interface. */
	void updateUi();

private slots:
	void on_pushButtonRemoveComponentInstance_clicked();
	void on_pushButtonConnectSurfaces_clicked();
	void on_tableWidgetInterlinkedSurfaces_itemSelectionChanged();

private:
	Ui::SVPropBuildingSurfaceConnectionWidget			*m_ui;

	/*! Caches currently selected surfaces. Updated in updateUi().
		We use a set since we frequently need to search for surfaces in this container.
	*/
	std::set<const VICUS::Surface*>						m_selectedSurfaces;

};

#endif // SVPropBuildingSurfaceConnectionWidgetH