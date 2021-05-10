Test für den FMU-Export von Schedulegrößen. Die FMU liefert
ein `PersonHeatLoadPerAreaSchedule`, `LightingHeatLoadPerAreaSchedule`, 
`EquipmentHeatLoadPerAreaSchedule`.

Der Berechnungsablauf bleibt vom FMU-Export unberührt, die CoSimulation
benötigt also keine `Partner`-FMU.


## NANDRAD FMU

Exportiert:

- Intensitätsdichte für Wärmelasten pro Person und Grundflächeneinheit: `Zone(id=1).PersonHeatLoadPerAreaSchedule` in W/m2
- Intensitätsdichte für Wärmelasten durch elektrisches Licht pro Grundflächeneinheit: `Zone(id=1).LightingHeatLoadPerAreaSchedule` in W/m2
- Intensitätsdichte für Wärmelasten durch elektrische Geräte pro Grundflächeneinheit: `Zone(id=1).EquipmentHeatLoadPerAreaSchedule` in W/m2




