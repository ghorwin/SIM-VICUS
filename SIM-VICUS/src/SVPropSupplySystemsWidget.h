#ifndef SVPropSupplySystemsWidgetH
#define SVPropSupplySystemsWidgetH

#include <QWidget>
#include "ui_SVPropSupplySystemsWidget.h"

#include <map>
#include <set>

//namespace Ui {
//	class SVPropSupplySystemsWidget;
//}

namespace VICUS {
	class SupplySystem;
	class Surface;
}

/*! A table showing the used boundary condition models.

	All visible surfaces are inspected and grouped according to their usage in ComponentInstances.
	A map is created of Components and surfaces, that use these components.
	Then, the boundary conditions in these components are inspected and a map is created
	for BoundaryConditions vs. surfaces. This is then shown in the table.
*/
class SVPropSupplySystemsWidget : public QWidget {
	Q_OBJECT

public:
	explicit SVPropSupplySystemsWidget(QWidget *parent = nullptr);
	~SVPropSupplySystemsWidget();

	/*! Updates user interface. */
	void updateUi();

private slots:
	void on_pushButtonEditSupplySystem_clicked();
	void on_tableWidgetSupplySystems_itemSelectionChanged();

	void on_pushButtonSelectSupplySystem_clicked();

private:
	Ui::SVPropSupplySystemsWidget *m_ui;

	/*! Map that holds the list of used supply systems vs. visible surfaces. */
	std::map<const VICUS::SupplySystem *, std::set<const VICUS::Surface *> >	m_supplySysSurfacesMap;
};

#endif // SVPropSupplySystemsWidgetH
