# Konvergenzverlauf, Rohr-Pumpen-System

## Variante 1

Pumpe:

	return p_outlet - p_inlet - deltaP;

Rohr:

	return p_inlet - p_outlet - deltaP;



*** Iter 1
Nodal pressures [Pa]
  0   0.7
  1   1000
Mass fluxes [kg/s]
  0   0.7
  1   0.7
Jacobian:
[         1         0        -1         1  ]  [      -0.7 ]
[         0         0         1        -1  ]  [        -0 ]
[        -1         1   10937.5        -0  ]  [   172.575 ]
[        -1         1         0     -2800  ]  [     -19.3 ]

deltaY
  0   -0.7
  1   49.8407
  2   0.0111574
  3   -0.0111574

*** Iter 2
Nodal pressures [Pa]
  0   2.0129e-11
  1   1049.84
Mass fluxes [kg/s]
  0   0.711157
  1   0.688843
Jacobian:
[         1         0        -1         1  ]  [ 0.0223148 ]
[         0         0         1        -1  ]  [ -0.0223148 ]
[        -1         1   11111.8        -0  ]  [ -0.972474 ]
[        -1         1         0  -2755.37  ]  [  -100.833 ]

deltaY
  0   5.05366e-09
  1   129.873
  2   -0.0117753
  3   -0.0105395

*** Iter 3
Nodal pressures [Pa]
  0   5.07379e-09
  1   1179.71
Mass fluxes [kg/s]
  0   0.699382
  1   0.678303
Jacobian:
[         1         0        -1         1  ]  [  0.021079 ]
[         0         0         1        -1  ]  [ -0.021079 ]
[        -1         1   10927.9        -0  ]  [  -1.08335 ]
[        -1         1         0  -2713.21  ]  [  -259.523 ]

deltaY
  0   -2.80977e-10
  1   253.505
  2   -0.0232972
  3   0.00221816

Masseströme driften auseinander






-------------------

## Variante 2

Pumpe:

	return p_inlet - p_outlet + deltaP; // Mind: pumps add pressure

Rohr:

	return p_inlet - p_outlet - deltaP;



*** Iter 1
Nodal pressures [Pa]
  0   0.7
  1   1000
Mass fluxes [kg/s]
  0   0.7
  1   0.7
Jacobian:
[         1         0        -1         1  ]  [      -0.7 ]
[         0         0         1        -1  ]  [        -0 ]
[         1        -1  -10937.5        -0  ]  [  -172.575 ]
[        -1         1         0     -2800  ]  [     -19.3 ]
deltaY
  0   -0.7
  1   -20.5082
  2   0.0139672
  3   0.0139672
  
  
*** Iter 2
Nodal pressures [Pa]
  0   2.0129e-11
  1   979.492
Mass fluxes [kg/s]
  0   0.713967
  1   0.713967
Jacobian:
[         1         0        -1         1  ]  [ -2.0129e-11 ]
[         0         0         1        -1  ]  [        -0 ]
[         1        -1  -11155.7        -0  ]  [  -38.0925 ]
[        -1         1         0  -2855.87  ]  [   40.0067 ]
deltaY
  0   -2.01297e-11
  1   -39.6165
  2   -0.000136614
  3   -0.000136614
  
  
*** Iter 3
Nodal pressures [Pa]
  0   -6.66133e-16
  1   939.875
Mass fluxes [kg/s]
  0   0.713831
  1   0.713831
Jacobian:
[         1 5.64325e-17        -1         1  ]  [ 6.66133e-16 ]
[         0         0         1        -1  ]  [        -0 ]
[         1        -1  -11153.6        -0  ]  [  -79.2329 ]
[        -1         1         0  -2855.32  ]  [   79.2331 ]
deltaY
  0   -0
  1   -79.233
  2   -1.31293e-08
  3   -1.31293e-08
  
  
*** Iter 4
Nodal pressures [Pa]
  0   -6.66133e-16
  1   860.642
Mass fluxes [kg/s]
  0   0.713831
  1   0.713831
Jacobian:
[         1         0        -1         1  ]  [ 6.66133e-16 ]
[         0         0         1        -1  ]  [        -0 ]
[         1        -1  -11153.6        -0  ]  [  -158.466 ]
[        -1         1         0  -2855.32  ]  [   158.466 ]
deltaY
  0   -0
  1   -158.466
  2   -5.59956e-15
  3   -5.59956e-15
  
  
*** Iter 5
Nodal pressures [Pa]
  0   -6.66133e-16
  1   702.176
Mass fluxes [kg/s]
  0   0.713831
  1   0.713831
Jacobian:
[         1         0        -1         1  ]  [ 6.66133e-16 ]
[         0         0         1        -1  ]  [        -0 ]
[         1        -1  -11153.6        -0  ]  [  -316.932 ]
[        -1         1         0  -2855.32  ]  [   316.932 ]
deltaY
  0   -0
  1   -316.932
  2   -8.11531e-18
  3   -8.11531e-18


Druck p2 driftet ab? Warum?


Korrekte analytische Lösung:  
-7/10          = -0.7
3000/157       = 19.108
307/21980      = 0.013967
307/21980      = 0.013967


--> numerische Lösung des DenseMatrix Solvers (LU-Faktorisierung mit Zeilen-Pivotisierung) ist falsch 
(schlechte Konditionierung)!




Konditionierung der Matrix ungünstig (selbst mit Pivotisierung). Verbesserung durch "Nachlösen":

deltaY (first solve)
  0   -0.7
  1   -20.5082
  2   0.0139672
  3   0.0139672
deltaY (improved)
  0   -0.7
  1   19.1082
  2   0.0139672
  3   0.0139672
converged
Nodal pressures [Pa]
  0   2.0129e-11
  1   1019.11
Mass fluxes [kg/s]
  0   0.713967
  1   0.713967




## Skalierung

--> Masseströme skalieren ok

## Mehr Rohre? Verzweigung?



