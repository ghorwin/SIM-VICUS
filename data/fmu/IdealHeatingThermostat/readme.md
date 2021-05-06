Die FMU regelt eine ideale Heizung. D.h. die FMU liefert
ein "CoolingControlValue" und überschreibt damit das Signal
des Thermostatmodells.

Für die Berechnung verwendet die FMU die Strahlungsintensität
auf einen Strahlungssensor und schaltet die Kühlung an, sobald die
Strahlungsintensität und Außenlufttemperatur über Schwellwerten liegen.
Die Kühlung läuft dann ungeregelt mit konstanter Leistung.

