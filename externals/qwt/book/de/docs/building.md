# Verwendung der Bibliothek in eigenen Programmen

Wie bei anderen externen Bibliotheken, gibt es grundsätzlich zwei Möglichkeiten, die Qwt-Bibliothek in eigenen Projekten zu verwenden:

1. Verwendung einer installierten, kompilierten Bibliothek unter Verwendung der installierten Headerdateien
2. Verwendung der Qwt-Bibliothek im Quelltext innerhalb der eigenen Projektstruktur

Unter Linux-Systemen (bzw. unter MacOSX, wenn Qwt über Homebrew oder MacPorts installiert ist), wird zumeist die Variante 1 sinnvoll sein.

Variante 2 ist stets dann sinnvoll, wenn man Zugriff auf die Implementierung der Qwt-Bibliothek haben möchte und gegebenenfalls eigene Änderungen einbringen möchte, oder wenn man mit einer noch nicht veröffentlichten Version aus einem Quelltextarchiv arbeiten will.

Auch für die Verwendung von Visual Studio unter Windows ist die Variante 2 am sinnvollsten, da so flexibel zwischen Debug und Release-Builds umgeschaltet werden kann (siehe [Visual Studio Projekte](#erstellen-mit-visual-studio)).

# Verwenden der vorkompilierten Bibliothek

- include Pfade
- lib-Dateinamen, statische/dynamische libs

## QMake und Qt Creator
...

## CMake
...

## Visual Studio

...

# Erstellung aus dem Quelltext

Die Qwt-Bibliothek kann auch als Quelltext-Archiv heruntergeladen werden. In diesem Fall muss die Bibliothek zuerst erstellt und gegebenenfalls installiert werden. Letzteres kopiert Bibliothek und benötigte Headerdateien ins Installationsverzeichnis, sodass andere Bibliotheken/Programme darauf zugreifen können.

Je nach Entwicklungsumgebung/Build-System kann man die Bibliothek auch ohne Installation verwenden (siehe [Verwendung der vorkompilierten Bibliothek](#verwenden-der-vorkompilierten-bibliothek)).

## Quelltext-Verzeichnisstruktur

Das Quelltextarchiv enthält folgende Verzeichnisstruktur:

```bash
admin         # Deployment-Scripte, in trunk
classincludes # Include-Dateien im Still von Qt5 Includes
designer/     # Quelltext der Designer-Plugins
doc/          # Doxygen Konfigurationsdateien
examples/     # Beispiele
playground/   # Zusätzliche Tests/Beispiele, ab Qwt 6.2
src/          # Der eigentliche Quelltext
tests/        # spez. Komponententests, ab Qwt 6.2
textengines/  # Zusatzkomponenten für Texte (MathML), ab Qwt 6.3

Qwtbuild.pri      # Grundlegende Kompilierungseinstellungen
Qwtconfig.pri     # Auswahl der zu kompilierenden Teile
Qwt.pro           # Master QMake-Datei
```

## Erstellen mit qmake

### Konfiguration

- TODO: QMake Build System und einflussreiche Dateien
- TODO: wo passe ich was an
- TODO: wie erstelle ich Beispiele (-> Examples + Playground)

### Erstellen und (optionale) Installation

Im Basisverzeichnes des Qwt Quelltextarchivs (bezeichnet mit `<qwt-root>`) ist `qmake` aufzurufen und dann `make`, bzw. unter Windows entsprechend `jom` oder `nmake`.

```bash
qmake
make
```

Die Beispiele (sofern konfiguriert) werden in `<qwt-root>/examples/bin` erstellt.

... TODO

## Erstellen mit cmake

## Visual Studio Projektdateien

- mit Erstellen der Bibliothek (Qwt-vcproj)


# Qwt Designer Plugins

...
