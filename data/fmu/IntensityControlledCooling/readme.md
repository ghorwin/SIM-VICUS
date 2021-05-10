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
zu den NANDRAD-Dateien in der .bashrc setzen:

```
export NANDRAD_PATH=/home/ghorwin/git/SIM-VICUS/bin/release
```

1. Generieren der NANDRAD-FMU

Im Testverzeichnis ausführen:

```bash
> $NANDRAD_PATH/NandradFMUGenerator --generate="CooledZone" CooledZone.nandrad
```

Dies erstellt die Datei `CooledZone.fmu`.

2. Generieren der Controller-FMU

Im Verzeichnis `Controller/CoolingController/build/` ausführen:

```bash
> ./build.sh
> ./deploy.sh
```

3. Durchlaufen der Testsimulation

fmu-Dateien ins Verzeichnis `release_mode` kopieren. Dort Simulation starten:

```bash
> MasterSimulator IntensityControlledCooling.msim
```



