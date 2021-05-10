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
- Außenlufttemperatur: `Location.AirTemperature` in C

Importiert:

- Kontrollsignal für Kühlung: `Zone(id=100).CoolingControlValue` in ---


## Automatisierte Tests

0. Vorbereitung

`NandradFMUGenerator` und `MasterSimulator` müssen global ausführbar sein. Dazu am 
Besten das `~/bin` Verzeichnis in den Suchpfad eintragen und dort symlinks auf die 
ausführbaren Dateien anlegen.

```bash
ln -s ~/git/SIM-VICUS/bin/release/NandradFMUGenerator NandradFMUGenerator
ln -s ~/svn/mastersim-code/bin/release/MasterSimulator MasterSimulator
```


1. Generieren der FMU

Im Testverzeichnis ausführen:

```bash
> NandradFMUGenerator --generate SingleZoneOnlyOpaqueWalls.nandrad
```

Dies erstellt die Datei `SingleZoneOnlyOpaqueWalls.fmu`, oder gibt einen Fehler aus.

2. Durchlaufen der Testsimulation

```bash
> MasterSimulator IntensityControlledCooling.msim
```

3. Vergleich der Testergebnisse

...



