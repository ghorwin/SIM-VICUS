#ifndef SVPropBuildingSurfaceConnectionWidgetH
#define SVPropBuildingSurfaceConnectionWidgetH

#include <QWidget>
#include <set>

#include "SVSmartIntersectionDialog.h"
#include "Vic3DSceneView.h"

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
	void updateUi(bool onlySelectionModified);

private slots:
    void on_pushButtonRemoveComponentInstance_clicked();
	void on_tableWidgetInterlinkedSurfaces_itemSelectionChanged();

    void on_pushButtonSmartClipping_clicked();

    void on_comboBoxHighlightingMode_currentIndexChanged(int);

signals:
    void updatedHighlightingMode(Vic3D::Scene::HighlightingMode mode);

private:
	Ui::SVPropBuildingSurfaceConnectionWidget			*m_ui;



	/*! Caches currently selected surfaces. Updated in updateUi().
		We use a set since we frequently need to search for surfaces in this container.
	*/
	std::map<unsigned int, const VICUS::Surface*>		m_selectedSurfaces;

    SVSmartIntersectionDialog                           *m_smartClippingDialog = nullptr;

};

#endif // SVPropBuildingSurfaceConnectionWidgetH
