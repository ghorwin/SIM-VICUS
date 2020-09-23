# Das Qwt (Hand-)Buch

_Copyright 2018, Andreas Nicolai <andreas.nicolai-at-gmx.net>_

```
```

Qwt (_Qwt - Qt Widgets for Technical Applications_, akt. Entwickler _Uwe Rathmann_) ist eine open-source Bibliothek für technische Anwendungen und stellt
bestimmte Widgets für Anzeigen und Kontrollkomponenten bereit. Die wohl wichtigste Komponente der Qwt Bibliothek ist das `QwtPlot`,
eine sehr flexible und mächtige Diagrammkomponente.

![QwtPlot Beispieldiagramm](imgs/exBode.png)

Die Qwt Bibliothek steht unter einer OpenSource Lizenz und wird auf
SourceForge.net gehostet:

- [Qwt Webseite (englisch)](http://qwt.sourceforge.net)
- [Qwt SourceForge Projektseite](https://sourceforge.net/projects/qwt)

# Handbuchstruktur

Das Handbuch ist in folgende Kapitel unterteilt:

* [Allgemeine Einführung](#allgemeine-einfuhrung) gibt einen Kurzüberblick über die Bibliothek und deren Grundkonzepte
* [Erstellen und Verwenden](building) beschreibt die Konfiguration und Erstellung der Bibliothek, und die Verwendung in eigenen Programmen
* Das Kapitel [Grundlagen](basics) beginnt mit einem einfachen Einführungsbeispiel, in welchem die einzelnen Schritte bis zum fertigen Diagramm beschrieben sind. Danach werden die einzelnen Anpassungsmöglichkeiten beschrieben.
* [Diagramm-Interaktion](interaction) beschreibt die Funktionen zur Nutzerinterkation mit dem Diagramm, z.B. Zoomen und Verschieben des Diagrammausschnitts, Anzeige von Werten unter dem Cursor, etc.
* Im Kapitel [Fortgeschrittenes](advanced) werden weiterführende Techniken zur Anpassung des Diagrammerscheinungsbildes beschrieben.
* [Anpassungen](customization) beschreibt die Erweiterung der Diagrammfunktionalität durch eigenen Klassen, welche die eingebauten Qwt-Klassen durch Überladung erweitern
* [Drucken/Exportieren](export) behandelt das Thema des Diagrammexports als Bild oder Vektorgrafik, letzteres auch zum Drucken des Diagramms
* Im Anhangskapitel [von Qwt 5.x zu 6.x](porting) werden typische Fragen und Problemstellungen bei der Portierung von Qwt 5 auf Qwt 6 beantwortet.

# Allgemeine Einführung

## Über die Bibliothek

### Entwicklungsgeschichte

- die erste Version der Qwt-Bibliothek stammt noch aus dem Jahr 1997 von Josef Wilgen
- seit 2002 wird die Bibliothek von _Uwe Rathmann_ entwickelt und gepflegt
- Version 5 ist wohl am weitesten verbreitet (erstes Release vom 26.02.2007)
- Version 6 (erstes Release vom 15.04.2011, kein Qt3 Support mehr) enthält wesentliche API-Änderungen
- aktuelle stabile Version 6.1.3, im trunk gibt es bereits wesentlich mehr und fortgeschrittene Funktionen

### Download der Bibliothek

Die Qwt Bibliothek kann von der [Qwt SourceForge Projektseite](https://sourceforge.net/projects/qwt) als Quelltextarchiv
geladen werden. Unter Linux wird Qwt bei vielen Distributionen als Paket gehalten. Genau genommen gibt es mehrere Pakete für die unterschiedlichen Qwt-Bibliotheksversionen.

**Ubuntu 16.04 LTS**:

```bash
# Qwt 5.x unter Verwendung von qt4 (für Qwt 5.2.3-1)
sudo apt install libqwt5-qt4-dev

# Qwt 6.x unter Verwendung von qt5 (für Qwt 6.1.2-5)
sudo apt install libqwt-qt5-dev
```

**Ubuntu 18.04 LTS**:

```bash
# Qwt 5.x unter Verwendung von qt4 (für Qwt 5.2.3-1)
sudo apt install libqwt5-qt4-dev

# Qwt 6.x unter Verwendung von qt4 (für Qwt 6.1.3-1)
sudo apt install libqwt-dev

# Qwt 6.x unter Verwendung von qt5 (für Qwt 6.1.3-1)
sudo apt install libqwt-qt5-dev
```

## Widget-Konzept und Erscheinungsbild

Die Qwt Bibliothek liefert Komponenten, welche analog zu den normalen Qt-Widgets in Deskopanwendungen verwendet werden können. Die Komponenten verwenden die vom jeweiligen Oberflächenstil definierten Paletteneinstellungen, sodass die Qwt-Widgets in die jeweilige Oberfläche passen. Dadurch integrieren sich die Widgets nahtlos in Programmoberflächen. Abrundungseffekte beim Plot-Widget ermöglichen das Immitieren klassischer Anzeigen:

![](imgs/styledDialsAndPlot.png)

Bei den Kontroll-Komponenten wird die Farbgebung über Palettenrollen definiert, z.B. bei den Zeigerkontrollen (`QwtDial`) via `QPalette::WindowText` (der Hintergrund der inneren Fläche) und `QPalette::Base` (die Grundfarbe des äußeren Ringes) und `QPalette::Text` (die Schriftfarbe):
![Anzeigen, eingefärbt](imgs/dialsColored.png)

In ähnlicher Weise können auch Gradienten benutzt werden, um ein plastisches Erscheinungsbild zu erzeugen:
![Anzeigen, mit Gradient](imgs/exDials2.png)

Bei der `QwtPlot`-Komponente betrifft das alle Teile, die außerhalb der eigentlichen Zeichenfläche liegen. Das heißt

- Titel,
- außenliegende Legende,
- Achsen, und
- Diagrammrahmen (einschließlich des möglichen 3D Rahmeneffekts)

werden durch die Paletten-Eigenschaften des Plotwidgets beeinflusst (siehe Beschreibung der relevanten Funktionen in [Rahmen und Zeichenfläche](basics/#rahmen-und-zeichenflache))

## Besitzer/Eigentümer-Konzept des QwtPlot-Widgets

Eine grundlegende Eigenschaft der `QwtPlot`-Klasse ist die Besitzübername hinzugefügter Elemente. Dies gilt allgemein für Zeichenobjekte, die Legende etc. Einmal hinzugefügte Elemente können nicht wieder losgelöst werden (bzw. nur über einen Trick, wie im Kapitel [Fortgeschrittenes](advanced) beschrieben wird). Daher ist es sinnvoll bei veränderlichen Diagrammelementen einen Mechanismus zur jeweiligen Neuerstellung eines Zeichenobjekts  vorzusehen (Factory-Konzept).

Beispiel:
```c++
void Plot::updateLegend() {
    QwtLegend * legend = new Qwtlegend();
    // Legendeneigenschaften setzen
    ...

    // Legende in Diagramm ersetzen (alte Legende wird gelöscht)
    insertLegend(legend);
}
```
## Zeichenobjekte und deren Achsenabhängigkeit

Ein wesentliches Designmerkmal beim `QwtPlot` ist die Möglichkeit, beliebige Zeichenobjekte (Kurven, Marker, Legende, ...) dem Plot zu übergeben. Damit sich diese Zeichenobjekte (engl. _PlotItem_) am Koordinatengitter ausrichten können, wird ihnen eine Achsenabhängigkeit gegeben. Dadurch erhalten diese Zeichenobjekte eine Information, wann immer sich die Achsenskalierung ändert (durch Zoomen, oder Änderung der Wertebereiche etc.).

Diese Funktionalität definiert die zentrale Bedeutung der (bis zu) 4 Achsen im Diagramm. Deswegen sind diese auch fest im `QwtPlot` verankert und werden nicht wie andere Zeichenobjekte beliebig hinzugefügt.

## Vererbungskonzept

Grundsätzlich ist das `QwtPlot` und die beteiligten Klassen auf maximale Anpassungsfähigkeit ausgelegt, d.h. es wird (fast) überall Polymorphie unterstützt. Wenn die eingebaute Funktionalität nicht zureichend ist, kann man einfach immer die entsprechende Klasse ableiten und die jeweils anzupassende Funktion re-implementieren und verändern (siehe Kapitel [Anpassungen](customization)).

## Verwendung der Designer Plugins

Die Qwt Bibliothek bringt Plugins für Qt Designer mit, welche das Einfügen von Qwt-Komponenten in ui-Dateien erleichtert. Es lassen sich jedoch keine QwtPlot-Eigenschaften festlegen oder Kurven hinzufügen. Die eigentliche Anpassung und Ausgestaltung des Plots erfolgt im Quelltext. Deswegen wird die Konfiguration und Anpassung des `QwtPlot` in diesem Handbuch ausschließlich durch normale API-Aufrufe demonstriert.

> **Hinweis:** Soll das QwtPlot auch ohne Designer-Plugins im grafischen QtDesigner-Editor eingefügt werden, kann man einfach ein QWidget einfügen und dieses als Platzhalter für die `QwtPlot`-Klasse definieren.

Eine Beschreibung, wie Designer-plugins erstellt und in Qt Creator/Designer integriert werden ist am Abschnitt [Qwt Designer Plugins(building#qwt-designer-plugins)] beschrieben.

---

_Autoreninfo: Schreibkonventionen_:

- `QwtDial` - Qwt-Klassen im Fließtext immer als Code-Schnipsel
- Quelltextblöcke immer mit c++ Hervorhebung einfügen (Dreifach schräges Hochkomma)
- Bilder ohne Skalierung einfügen (Skalierung erfolgt durch CSS)
