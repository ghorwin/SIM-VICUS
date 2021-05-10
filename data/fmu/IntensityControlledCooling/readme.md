Die FMU regelt eine ideale Heizung. D.h. die FMU liefert
ein `CoolingControlValue` und überschreibt damit das Signal
des Thermostatmodells.

Für die Berechnung verwendet die FMU die Strahlungsintensität
auf einen Strahlungssensor und schaltet die Kühlung an, sobald die
Strahlungsintensität und Außenlufttemperatur über Schwellwerten liegen.
Die Kühlung läuft dann ungeregelt mit konstanter Leistung.


## NANDRAD FMU

Exportiert:

- Solare Strahlungsintensität auf das Dach: `ConstructionInstance(id=5).SolarRadiationFluxB` in W/m2
- Außenlufttemperatur: `Location.Temperature` in C
- Zonenlufttemperatur: `Zone(id=100).AirTemperature` in C

Importiert:

- Kontrollsignal für Kühlung: `Zone(id=100).CoolingControlValue` in ---


## Automatisierte Tests

0. Vorbereitung

`MasterSimulator` muss global ausführbar sein. Dazu am 
Besten das `~/bin` Verzeichnis in den Suchpfad eintragen und dort einen
Symlink auf die ausführbare Datei anlegen.

```bash
ln -s ~/svn/mastersim-code/bin/release/MasterSimulator MasterSimulator
```

Das Verzeichnis von `NandradFMUGenerator` und `NandradSolver` (müssen
im gleichen Verzeichnis liegen), muss bekannt sein. Dazu den Pfad
zu den ausführbaren Dateien in den Suchpfad in der .bashrc setzen:

```
export PATH=$PATH:/home/ghorwin/git/SIM-VICUS/bin/release

```

1. Generieren der FMU

Im Testverzeichnis ausführen:

```bash
> $NANDRAD_PATH/NandradFMUGenerator --generate SingleZoneOnlyOpaqueWalls.nandrad
```

Dies erstellt die Datei `SingleZoneOnlyOpaqueWalls.fmu`, oder gibt einen Fehler aus.

2. Durchlaufen der Testsimulation

```bash
> MasterSimulator IntensityControlledCooling.msim
```

3. Vergleich der Testergebnisse

...



