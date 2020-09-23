# Spezifische Anpassungen

## Layout hints

## Render hints


# Performance-Optimierungen

- Punkte filtern (monotone x-Achsen)
- Antialiasing

# Overlays

...

# Spline-Interpolation von Kurven
ab `6.3.x_trunk`
...



# Zeichenreihenfolge

- wie kontrolliert man die Zeichenreihenfolge (zValues)
- Standardreihenfolge beim Zeichnen

# Tricks und Tipps

## Besitzübernahme von Objekten des Plots

Um einmal hinzugefügte Elemente zu entnehmen, _ohne dass diese gelöscht werden_, muss man zuerst deren parent-Attribut ändern. Dazu benötigt man Schreibzugriff auf das jeweilige Element, welchen man eventuell über einen `const_cast` erhalten kann. Um z.B. ein Legendenobjekt zu entnehmen, um es später modifiziert wieder hinzuzufügen, kann man folgendes tun:
```c++
QwtLegend * legend = const_cast<QwtLegend *>(plot->legend());
legend->setParent(anotherObject);
plot->insertLegend(NULL); // remove legend from plot
```
