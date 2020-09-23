# Benutzer-Interaktion mit dem Diagramm

Wenn ein Diagramm erstellt wurde, gibt es verschiedene Möglichkeiten für den Nutzer, mit dem Diagramm zu interagieren:

- Werte unter dem Mauskursor anzeigen
- Werte der nächstgelegenen Kurve anzeigen
- Zoomen und Verschieben der Plotansicht
- und weitere...

Um das umzusetzen, könnte man das QwtPlot ableiten und die entsprechenden Maus- und Tastaturevents von Qt in den jeweiligen Event-Funktionen behandelt. Die Qwt-Bibliothek bringt aber ein paar vorbereitete Funktionen mit. Diese registrieren sich über EventFilter beim Diagramm und fangen entsprechende Ereignisse (Mausklick, Mausbewegungen, Tastaturanschläge,... ) ab.

Die Implementierung erfolgt in den _Interaktionskomponenten_ stets ähnlich - das entsprechende Objekt wird erstellt und im Konstruktor übergibt man die Zeichenfläche (canvas) des gewählten `QwtPlot`. Damit kann die Interaktionskomponente den EventFilter installieren.

> **Hinweis:** Für die meisten Komponenten ist die Reihenfolge, in der die Events abgearbeitet werden, unwichtig. In manchen Fällen möchte man jedoch Seiteneffekte vermeiden, d.h. eine Interaktionskomponente akzeptiert und verarbeitet das Ereignis und die nachgeordneten EventFilter erhalten das Ereignis gar nicht mehr. In diesem Fall muss man die Reihenfolge der Erstellung der Interaktionskomponenten beachten.

## Datenanzeige (Tracker)

Die Basisklasse für Interaktionskomponenten ist die Klasse `QwtPicker`. Diese implementiert die Behandlung von Enter, Leave, Mouse und Keyboard-Ereignissen und transformiert diese in ausgewählte Punkte. Dabei gibt es verschiedene Zustandsautomaten, welche man nachnutzen kann.

Für die Anzeige von
QwtPicker *picker = new QwtPicker(widget);

Die Klasse QwtTracker

- wie funktioniert der Tracker
- punkt, vertikal, horizontal

## Verschieben der Plotfläche

Das Verschieben der aktuellen Ansichtsfläche des Diagramms erfolgt mit dem `QwtPlotPanner`.

```c++
// Erstelle Objekt, welches sich gleich bei der Zeichenfläche des
// Plot registriert
d_panner = new QwtPlotPanner(plot->canvas());
d_panner->setMouseButton(Qt::MidButton);
```


## Kurvenverlauf abfahren (CurveTracker)

- wie funktioniert der Tracker

## Zoomen/Plotausschnitt verschieben (Zooming/Panning):

- zoomer einstellungen
- zoom rect

### Basiszoomstufe einstellen
- zoom base einstellen (maximale rauszoomgröße)

### Mehrere Achsen zoomen

- Rücktransformation zeigen


# Legendeneinträge ein/ausschalten

Bei einer außenliegenden Legende sind die standardmäßig erstellen Legendeneinträge einfache Label vom Typ `QwtLegendLabel`. Diesen können an- und ausgeschaltet werden (checked). Ein typischer Anwendungsfall ist das Sichtbar-/Versteckt-Schalten von Diagrammkurven/Balken, um einen Teil der Informationen zu verbergen.

Zunächst muss die entsprechende Legendeneigenschaft eingeschaltet werden:
```c++
// Legendeneinträge "checkbar" machen
d_legend->setDefaultItemMode(QwtLegendData::Checkable);
```
## Auf Checked/Unchecked-Ereignisse reagieren
Damit man auf das Umschalten der Legendeinträge reagieren kann, muss man eine Signal-Slot-Verbindung zwischen Signal `QwtLegendLabel::checked(bool)` und einer eigenen Funktion herstellen. Das zu einer Kurve gehörige LegendLabel findet man über die Funktion `QwtLegend::legendWidget()` (wobei das eigentlich das Widget eines einzelnen Legendeneintrags ist). Das kann man dann in ein Label casten und via mit der entsprechenden Funktion verknüpfen:
```c++
// das zu einer Kurve gehörige LegendLabel findet man über die Funktion QwtLegend::legendWidget()
// 'plotCurve1' ist ein Zeiger zu einer hinzugefügten QwtPlotCurve
QWidget* legendWidget = d_legend->legendWidget(itemToInfo(plotCurve1));
QwtLegendLabel* label = qobject_cast<QwtLegendLabel*>(legendWidget);
// das Signal verknüpfen
connect(label, SIGNAL(checked(bool)),
        this, SLOT(onCurveVisibilityChanged(bool)) );
```

> **Hinweis:** Statt `qobject_cast<QwtLegendLabel*>` könnte man auch `dynamic_cast<QwtLegendLabel*> verwenden, ersteres ist aber schneller und daher besser bei QObject-abgeleiteten Klassentypen.

### Sichtbar/Unsichtbar-Schalten von Linien/Kurven im Diagramm (QwtPlotCurve)

Die Implementierung bei Liniendiagrammen sieht so aus:

```c++
void MyPlot::onCurveVisibilityChanged (bool on) {
	QObject * origin = sender();
	// cast pointer to QwtLegendItem
	QwtLegendLabel * item = qobject_cast<QwtLegendLabel *>(origin);
	// check that is was a valid sender
	if (item == NULL)
		return; // invalid origin

	// now find out which curve belongs to this legend item
	for (unsigned int i=0; i<NumCurves; ++i) {
		if (m_curves[i] == NULL)
			continue; // skip empty curves
		QWidget* legendWidget = m_legend->legendWidget(itemToInfo(m_curves[i]));
		QwtLegendLabel* label = qobject_cast<QwtLegendLabel*>(legendWidget);
		if (label == item) {
			// set visibility of curve based on function argument
			m_curves[i]->setVisible(on);
			replot();
			break;
		}
	}
}
```


## Legendeneinträge alternativ schalten

Implement show curve function such:
```c++
void CpuPlot::showCurve(QwtPlotItem *item, bool on)
{
    // wenn eine Curve angeschaltet werden soll, vorher alle anderen ausschalten
    if (on) {
        const QwtPlotItemList &list = this->itemList();
        for (QwtPlotItemIterator it = list.begin();it!=list.end();++it) {
            QwtPlotItem *item2 = *it;
            if (item2->rtti() == QwtPlotItem::Rtti_PlotCurve)
            item2->setVisible(false);
        }
    }
    item->setVisible(on);
    // Legendenwidget aktualisieren
    QWidget *w = legend()->find(item);
    if ( w && w->inherits("QwtLegendItem") )
        ((QwtLegendItem *)w)->setChecked(on);
    replot();
}
```
