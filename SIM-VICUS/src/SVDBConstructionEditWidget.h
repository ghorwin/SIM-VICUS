#ifndef SVDBConstructionEditWidgetH
#define SVDBConstructionEditWidgetH

#include <QWidget>

namespace Ui {
class SVDBConstructionEditWidget;
}

#include <VICUS_Construction.h>

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


	/*! Updates widget with content of given construction data. */
	void setConstruction(const VICUS::Construction & con);

	/*! Returns current construction data. */
	const VICUS::Construction & construction() const { return m_construction; }

signals:

	/*! Emitted, whenever m_construction changes due to user interaction. */
	void constructionChanged();

private:
	Ui::SVDBConstructionEditWidget	*m_ui;

	/*! Stores data currently shown in widget.
		Any change commited in widget are stored first here, then the change signal is emitted.
	*/
	VICUS::Construction				m_construction;
};

#endif // SVDBConstructionEditWidgetH
