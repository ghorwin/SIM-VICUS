#ifndef SVDBConstructionEditWidgetH
#define SVDBConstructionEditWidgetH

#include <QWidget>

namespace Ui {
class SVDBConstructionEditWidget;
}

/*! Edit widget for construction (types). */
class SVDBConstructionEditWidget : public QWidget {
	Q_OBJECT

	/*! TODO SIM VICUS TEAM
		Die Materialschichttabelle besteht aus folgenden Punkten
		Nr | Aktiv | Material | Dicke | Wärmeleitfähigkeit | Dichte | spec. Wärmekapazität | Widerstand
		   |       |          |  cm   | W/m2K              | kg/m3  |         J/kgK        |   m2K/W

		Bei Aktiv sind checkboxen drin
		Nur Aktiv und Dicke kann editiert werden
		Rest sind nur Infos
		Die Zeilen werden immer in unterschiedlichen Hintergrundfarben dargestellt
		grau
		hellgrau
		grau
		etc...

		Beispiel
		1 | o | Bitumen | 0.1 | 1 | 1500 | 840 | ...
		1 | x | Beton	| 20  | 2 | 2300 | 840 | ...


		TODO Dirk
		Die weiteren Eigenschaften müssen noch gesetzt werden
		Hersteller, etc...
	*/
public:
	explicit SVDBConstructionEditWidget(QWidget *parent = nullptr);
	~SVDBConstructionEditWidget();

private:
	Ui::SVDBConstructionEditWidget *m_ui;
};

#endif // SVDBConstructionEditWidgetH
