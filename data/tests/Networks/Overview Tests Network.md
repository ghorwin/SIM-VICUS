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


### HydraulicNetworkSplitter.nandrad

- Zwei Kreise (parallele Rohre), siehe `NetworkWithSplitter.pdf`
- Dadurch, dass das Rohr 103 einen deutlich höheren Strömungswiderstand hat,
  fließt dort weniger Fluid durch.
- Masseströme durch Rohre 103 und 105 summieren sich zum Massestrom durch Pumpe und
  Rohre 107, 101 und 102  

```
  103 (Rohr)         : 0.0174 kg/s
  105 (Rohr)         : 0.0694 kg/s (wie auch 104 und 106)
  107 (Rohr)         : 0.0868 kg/s (wie auch 101, 102, und Pumpe 201)
```


### HydraulicNetworkStaticBackFlow.nandrad

- Zwei Kreise, zwei Pumpen (eine dezentral), siehe `NetworkWithBackFlow.pdf`
- kleine Pumpe (202) erzeugt lokal einen Druckanstieg, der die Strömungsrichtung 
  in Rohr 103 umkehrt.
  
```
  103 (Rohr)         : -0.0354 kg/s
  105 (Rohr)         :  0.0968 kg/s (wie auch 104 und 106)
  107 (Rohr)         :  0.0614 kg/s (wie auch 101, 102, und Pumpe 201)
```

- In diesem Beispiel zirkuliert ein Teil des Fluids immer nur im oberen Kreis.


## Thermo-Hydraulic Tests

### ThermoHydraulicNetworkStaticFlowConstantHeatloss_Dense.nandrad

Ein Kreis, 3 Elemente:

- Pumpe 201 (`ConstantPressurePump`), Wirkungsgrad 100%
- Wärmetauscher 301, Wärmeaustauschmodell `HeatFluxConstant`, 500 W konstante Energiequelle
- Rohr 101 (`SimplePipe`), Wärmeaustauschmodell `TemperatureConstant`, 
  Erdreichtemperatur 0 C

Abkühlung des Systems von einer einheitlichen Fluidtemperatur von 20 C
- Anfänglich erwärmt der Wärmetauscher das Fluid in der Pumpe etwas, bis 
  sich das im Rohr abgekühlte Fluid auswirkt
- nach 


### Test1 Mixer

....

### Test2 Splitter

....











