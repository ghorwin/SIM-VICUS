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


