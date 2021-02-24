# Network Test Cases



## Hydraulic Tests

### HydraulicNetworkStatic.nandrad

- Einzelner Kreis, 4 Komponenten, siehe `Network.pdf`
- Referenzelement ist 201 (die Pumpe), d.h. Referenzdruck von 0 Pa
  liegt am Einlauf der Pumpe an
- Druckabfall in Strömungsrichtung, jeweils nach dem Strömungselement:

```
  201 (Pumpe)         : 1000.0 Pa  (node 2)
  101 (Rohr 1)        :  515.4 Pa  (node 3)
  301 (Wärmetauscher) :  484.6 Pa  (node 4)
  102 (Rohr 2)        :    0.0 Pa  (node 1)
```
- Konstanter Massenstrom:  0.0570898 kg/s


### HydraulicNetworkStaticInvertedPump.nandrad

- wie `HydraulicNetworkStatic.nandrad`
- Pumpe entgegengesetzt eingebaut
- Massestrom sollte anderes Vorzeichen haben
- Druckverteilung sollte entsprechend anders herum abnehmen
- Druckabfall, ausgegeben am "Outlet" (beachte, Outlet der Pumpe ist an 
  Node 1, und Outlet of 102 ist auch an Node 1)

```
  201 (Pumpe)         : 1000.0 Pa   (node 1)
  102 (Rohr 2)        : 1000.0 Pa   (node 1)
  301 (Wärmetauscher) :  515.4 Pa   (node 4)
  101 (Rohr 1)        :  484.6 Pa   (node 3)
```
- Konstanter Massenstrom:  -0.0570908 kg/s

**BUG**: Ausgabe des Massestroms für Variable Network.FluidMassFluxes zeigt +0.0570908 kg/s !!!


### HydraulicNetworkSplitter.nandrad

- Zwei Kreise (parallele Rohre), siehe `NetworkWithSplitter.pdf`
- Konstante Lösung, Druckverteilungstest entsprechend der 
  hydraulischen Widerstände der Teilkreise
  
### HydraulicNetworkStaticBackFlow.nandrad

- Zwei Kreise, zwei Pumpen (eine dezentral), siehe `NetworkWithBackFlow.pdf`
- kleine Pumpe kann den Massestrom nicht aufhalten und wird in Gegenrichtung
  durchströmt
  




## Thermo-Hydraulic Tests


### Test1 Mixer

....

### Test2 Splitter

....











