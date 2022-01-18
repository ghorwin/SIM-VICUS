# Testbeschreibung

Ein einfacher Raum hat eine ideale Heizung. Die Heizlast wird in einer FMU berechnet und verhält sich nichtlinear
in Abhängigkeit von der Raumtemperatur. Dadurch muss ein iterativer Masteralgorithmus mehrfach iterieren, um
ein Ergebnis zu erhalten.

Mit diesem Test wird das korrekte Rücksetzverhalten der NANDRAD-FMU getestet, einschließlich der Ausgabenbehandlung.

## FMUs

`BuildingModel` - das Gebäude _"IdealHeating"_
`HeatingModel` - die Heizung

## Schnittstellen

- `IdealHeating.RoomTemperature` -> `HeatingModel.RoomTemperature`
- `HeatingModel.HeatingPower` -> `IdealHeating.HeatingLoad`

## Varianten

- `BuildingModel/IdealHeating_heatingpower0.nandrad` - alleinstehende NANDRAD Simulation, maximale Heizlast ist auf 0 W/m2 gesetzt, damit ist die Heizung aus. Das Gebäude schwingt frei.


- `StandAlone.msim` - Nur die Building FMU, keine angekoppelten FMUs, d.h. Heizleistung des FMU-Inputs = 0 -> frei schwingend wie beim alleinstehenden Testfall, Ergebnisse müssen (nahezu) identisch sein
- `NoIteration.msim` - Heizungs-FMU ist angekoppelt, Heizleistung wird berechnet, aber es wird nicht iteriert (Kommunikationsschrittlänge = 12 min)
- `GaussSeidel.msim` - Heizungs-FMU ist angekoppelt, Heizleistung wird berechnet, es wird bis zu 6 mal iteriert (Kommunikationsschrittlänge = 12 min)

Erwartung: bei der iterativen Variante sind die Ausgaben des NANDRAD-Slaves zunächst mal chronologisch korrekt (d.h. alle 3 Minuten). Außerdem sollten inhaltlich keine riesig großen Unterschiede auftreten.


