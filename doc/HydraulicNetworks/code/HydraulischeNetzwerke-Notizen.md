# Systemformulierungsvarianten

a. Knotendrücke + Masseströme
b. Knotendrücke


## Implementierungstests, Variante a

### Rohr-Pumpen-System

y = [p1, p2, mdot1, mdot2]

Systemgleichungen:

Pumpe:

	deltaP = MAX_DELTAP - mdot*mdot*scale;
	return p_inlet - p_outlet + deltaP; // Mind: pumps add pressure

Rohr:

	double deltaP = mdot*mdot*m_res;
	return p_inlet - p_outlet - deltaP;


#### Anfangsbedingungen

dicht an Lösung:

y0 = [0, 1000, 0.7, 0.7]

............
res = 86.5174

*** Iter 1
Nodal pressures [Pa]
  0   0
  1   1000
Mass fluxes [kg/s]
  0   0.7
  1   0.7
Jacobian:
[         1         0        -1         1  ]  [        -0 ]
[         0         0         1        -1  ]  [        -0 ]
[         1        -1  -10937.5        -0  ]  [  -171.875 ]
[        -1         1         0     -2800  ]  [       -20 ]
deltaY
  0   1.73472e-18
  1   19.1082
  2   0.0139672
  3   0.0139672

res = 0.786571

*** converged
Nodal pressures [Pa]
  0   1.73472e-18
  1   1019.11
Mass fluxes [kg/s]
  0   0.713967
  1   0.713967
............


weit weg von der Lösung:

y0 = [5000, 0, 0.4, 0.1]

res = 5629.38

*** Iter 1
Nodal pressures [Pa]
  0   5000
  1   0
Mass fluxes [kg/s]
  0   0.4
  1   0.1
Jacobian:
[         1         0 -0.999999  0.999999  ]  [   -4999.7 ]
[         0         0         1        -1  ]  [      -0.3 ]
[         1        -1  -6250.01        -0  ]  [     -8750 ]
[        -1         1         0  -399.998  ]  [      5020 ]
deltaY
  0   -5000
  1   357.141
  2   0.542857
  3   0.842857

res = 732.508

*** Iter 2
Nodal pressures [Pa]
  0   0.00169285
  1   357.141
Mass fluxes [kg/s]
  0   0.942857
  1   0.942857
Jacobian:
[         1         0        -1         1  ]  [ -0.00169285 ]
[         0         0         1        -1  ]  [ -2.2108e-11 ]
[         1        -1         0        -0  ]  [   357.139 ]
[        -1         1         0  -3771.43  ]  [   1420.82 ]
deltaY
  0   -0.00169285
  1   -357.141
  2   -0.471429
  3   -0.471429

res = 1646.92

*** Iter 3
Nodal pressures [Pa]
  0   -1.42247e-16
  1   -9.0169e-07
Mass fluxes [kg/s]
  0   0.471428
  1   0.471428
Jacobian:
[         1         0        -1         1  ]  [ 1.42247e-16 ]
[         0         0         1        -1  ]  [        -0 ]
[         1        -1  -7366.07        -0  ]  [  -3263.72 ]
[        -1         1         0  -1885.71  ]  [   444.489 ]
deltaY
  0   1.66533e-16
  1   1019.11
  2   0.304722
  3   0.304722

res = 374.414

*** Iter 4
Nodal pressures [Pa]
  0   2.42861e-17
  1   1019.11
Mass fluxes [kg/s]
  0   0.776151
  1   0.776151
Jacobian:
[         1         0        -1         1  ]  [ -2.42861e-17 ]
[         0         0         1        -1  ]  [        -0 ]
[         1        -1  -12127.4        -0  ]  [   725.434 ]
[        -1         1         0   -3104.6  ]  [   185.712 ]
deltaY
  0   -2.08167e-17
  1   0.00113481
  2   -0.059818
  3   -0.059818

res = 14.4283

*** Iter 5
Nodal pressures [Pa]
  0   3.46945e-18
  1   1019.11
Mass fluxes [kg/s]
  0   0.716333
  1   0.716333
Jacobian:
[         1         0        -1         1  ]  [ -3.46945e-18 ]
[         0         0         1        -1  ]  [        -0 ]
[         1        -1  -11192.7        -0  ]  [   27.9551 ]
[        -1         1         0  -2865.33  ]  [   7.15627 ]
deltaY
  0   -3.46945e-18
  1   -0.000182562
  2   -0.0024976
  3   -0.0024976

res = 0.0251619

*** converged
Nodal pressures [Pa]
  0   0
  1   1019.11
Mass fluxes [kg/s]
  0   0.713835
  1   0.713835
............


Sinnvolle allgemeingültige Startlösung?

y0 = [0, 0, 0.1, 0.1]


res = 2460.96

*** Iter 1
Nodal pressures [Pa]
  0   0
  1   0
Mass fluxes [kg/s]
  0   0.1
  1   0.1
Jacobian:
[         1         0        -1         1  ]  [        -0 ]
[         0         0         1        -1  ]  [        -0 ]
[         1        -1  -1562.51        -0  ]  [  -4921.88 ]
[        -1         1         0  -399.998  ]  [        20 ]
deltaY
  0   -7.95808e-13
  1   1019.1
  2   2.49776
  3   2.49776

res = 6259.6

*** Iter 2
Nodal pressures [Pa]
  0   -7.95808e-13
  1   1019.1
Mass fluxes [kg/s]
  0   2.59776
  1   2.59776
Jacobian:
[         1 -5.28484e-17        -1         1  ]  [ 7.95808e-13 ]
[         0         0         1        -1  ]  [        -0 ]
[         1        -1         0        -0  ]  [    1019.1 ]
[        -1         1         0  -10391.1  ]  [   12477.6 ]
deltaY
  0   1.47793e-12
  1   -1019.1
  2   -1.29888
  3   -1.29888

res = 1687.09

*** Iter 3
Nodal pressures [Pa]
  0   6.82121e-13
  1   -2.57298e-06
Mass fluxes [kg/s]
  0   1.29888
  1   1.29888
Jacobian:
[         1 -5.3109e-17        -1         1  ]  [ -6.82121e-13 ]
[         0         0         1        -1  ]  [        -0 ]
[         1        -1 -4.23516e-16 4.23516e-16  ]  [ -2.57298e-06 ]
[        -1         1         0  -5195.52  ]  [   3374.19 ]
deltaY
  0   -6.82121e-13
  1   2.57298e-06
  2   -0.649441
  3   -0.649441

res = 951.085

*** Iter 4
Nodal pressures [Pa]
  0   -2.30816e-19
  1   -4.23516e-22
Mass fluxes [kg/s]
  0   0.64944
  1   0.64944
Jacobian:
[         1 7.51098e-17        -1         1  ]  [ 2.30892e-19 ]
[         0         0         1        -1  ]  [        -0 ]
[         1        -1  -10147.5        -0  ]  [   -1704.9 ]
[        -1         1         0  -2597.76  ]  [   843.546 ]
deltaY
  0   -1.13687e-13
  1   1019.11
  2   0.0675822
  3   0.0675822

res = 18.4163

*** Iter 5
Nodal pressures [Pa]
  0   -1.13687e-13
  1   1019.11
Mass fluxes [kg/s]
  0   0.717023
  1   0.717023
Jacobian:
[         1 6.78302e-17        -1         1  ]  [ 1.13687e-13 ]
[         0         0         1        -1  ]  [        -0 ]
[         1        -1  -11203.5        -0  ]  [   35.6818 ]
[        -1         1         0  -2868.09  ]  [   9.13485 ]
deltaY
  0   1.13688e-13
  1   0.000246406
  2   -0.00318491
  3   -0.00318491

res = 0.0409127

*** converged
Nodal pressures [Pa]
  0   6.32537e-19
  1   1019.11
Mass fluxes [kg/s]
  0   0.713838
  1   0.713838



Auch 5 Iterationen...


#### Reihenfolge der Gleichungen und Variablen??

y = [mdot1, mdot2, p1, p2]

res = 2460.96

*** Iter 1
Mass fluxes [kg/s]
  0   0.1
  1   0.1
Nodal pressures [Pa]
  0   0
  1   0
Jacobian:
[  -1562.51         0         1        -1  ]  [  -4921.88 ]
[         0  -400.002        -1         1  ]  [        20 ]
[        -1         1         1         0  ]  [        -0 ]
[         1        -1         0         0  ]  [        -0 ]
deltaY
  0   2.49776
  1   2.49776
  2   -4.45488e-16
  3   1019.11

res = 6259.57

*** Iter 2
Mass fluxes [kg/s]
  0   2.59776
  1   2.59776
Nodal pressures [Pa]
  0   -4.45488e-16
  1   1019.11
Jacobian:
[         0         0         1        -1  ]  [   1019.11 ]
[         0    -10391        -1         1  ]  [   12477.6 ]
[        -1         1         1 4.93777e-17  ]  [ -4.4269e-16 ]
[         1        -1         0        -0  ]  [ 8.88178e-16 ]
deltaY
  0   -1.29888
  1   -1.29888
  2   5.07667e-14
  3   -1019.11

res = 1687.09

*** Iter 3
Mass fluxes [kg/s]
  0   1.29888
  1   1.29888
Nodal pressures [Pa]
  0   5.03212e-14
  1   -2.573e-06
Jacobian:
[         0         0         1        -1  ]  [ -2.573e-06 ]
[         0  -5195.52        -1         1  ]  [   3374.17 ]
[        -1         1         1 9.5782e-18  ]  [ -5.03212e-14 ]
[         1        -1         0         0  ]  [        -0 ]
deltaY
  0   -0.649439
  1   -0.649439
  2   -5.03212e-14
  3   2.573e-06

res = 951.087

*** Iter 4
Mass fluxes [kg/s]
  0   0.64944
  1   0.64944
Nodal pressures [Pa]
  0   0
  1   0
Jacobian:
[  -10147.5         0         1        -1  ]  [  -1704.91 ]
[         0  -2597.76        -1         1  ]  [   843.544 ]
[        -1         1         1 -1.26218e-23  ]  [ -1.26218e-29 ]
[         1        -1         0         0  ]  [        -0 ]
deltaY
  0   0.0675828
  1   0.0675828
  2   0
  3   1019.11

res = 18.4166

*** Iter 5
Mass fluxes [kg/s]
  0   0.717023
  1   0.717023
Nodal pressures [Pa]
  0   0
  1   1019.11
Jacobian:
[  -11203.5         0         1        -1  ]  [   35.6824 ]
[         0  -2868.09        -1         1  ]  [   9.13473 ]
[        -1         1         1        -0  ]  [        -0 ]
[         1        -1         0        -0  ]  [        -0 ]
deltaY
  0   -0.00318494
  1   -0.00318494
  2   1.65509e-24
  3   2.10466e-05

res = 0.0409151

*** converged
Mass fluxes [kg/s]
  0   0.713838
  1   0.713838
Nodal pressures [Pa]
  0   1.65509e-24
  1   1019.11


Auch 5 Iterationen... spielt keine Rolle!


### Skalierung der Gleichungen

Masseströme << Drücke -> Massstromgleichungen skalieren?


Option: Beide skalieren:

Faktor 10:     8 Iterationen
Faktor 100:   13 Iterationen
Faktor 1000:  15 Iterationen

Größerer Faktor, größere Genauigkeit der Jacobimatrix -> flachere Gradienten bei der Pumpenlinie


Option: nur Masseströme skalieren?

-> kein Unterschied

Option: nur Gleichungen skalieren?

res = 2460.96

*** Iter 1
Mass fluxes [kg/s]
  0   0.1
  1   0.1
Nodal pressures [Pa]
  0   0
  1   0
Jacobian:
[  -1562.51         0         1        -1  ]  [  -4921.88 ]
[         0  -400.002        -1         1  ]  [        20 ]
[     -1000      1000         1         0  ]  [        -0 ]
[      1000     -1000         0         0  ]  [        -0 ]
deltaY
  0   2.49776
  1   2.49776
  2   0
  3   1019.11

res = 6259.57

*** Iter 2
Mass fluxes [kg/s]
  0   2.59776
  1   2.59776
Nodal pressures [Pa]
  0   0
  1   1019.11
Jacobian:
[         0         0         1        -1  ]  [   1019.11 ]
[         0    -10391        -1         1  ]  [   12477.6 ]
[     -1000      1000         1        -0  ]  [        -0 ]
[      1000     -1000         0        -0  ]  [        -0 ]
deltaY
  0   -1.29888
  1   -1.29888
  2   0
  3   -1019.11

res = 1687.09

*** Iter 3
Mass fluxes [kg/s]
  0   1.29888
  1   1.29888
Nodal pressures [Pa]
  0   0
  1   -2.573e-06
Jacobian:
[         0         0         1        -1  ]  [ -2.573e-06 ]
[         0  -5195.52        -1         1  ]  [   3374.17 ]
[     -1000      1000         1         0  ]  [        -0 ]
[      1000     -1000         0         0  ]  [        -0 ]
deltaY
  0   -0.649439
  1   -0.649439
  2   0
  3   2.573e-06

res = 951.087

*** Iter 4
Mass fluxes [kg/s]
  0   0.64944
  1   0.64944
Nodal pressures [Pa]
  0   0
  1   0
Jacobian:
[  -10147.5         0         1        -1  ]  [  -1704.91 ]
[         0  -2597.76        -1         1  ]  [   843.544 ]
[     -1000      1000         1         0  ]  [        -0 ]
[      1000     -1000         0         0  ]  [        -0 ]
deltaY
  0   0.0675828
  1   0.0675828
  2   -1.10054e-13
  3   1019.11

res = 18.4166

*** Iter 5
Mass fluxes [kg/s]
  0   0.717023
  1   0.717023
Nodal pressures [Pa]
  0   -1.10054e-13
  1   1019.11
Jacobian:
[  -11203.5         0         1        -1  ]  [   35.6824 ]
[         0  -2868.09        -1         1  ]  [   9.13473 ]
[     -1000      1000         1 9.89133e-17  ]  [ -9.68728e-16 ]
[      1000     -1000         0        -0  ]  [ 1.11022e-13 ]
deltaY
  0   -0.00318494
  1   -0.00318494
  2   1.10134e-13
  3   2.10459e-05

res = 0.0409151

*** converged
Mass fluxes [kg/s]
  0   0.713838
  1   0.713838
Nodal pressures [Pa]
  0   8.0547e-17
  1   1019.11



Macht keinen Unterschied. Skalierung bringt offensichtlich keinen Mehrgewinn.



### Pumpe + 3 Rohre (davon 2 parallel)

res = 703.762

*** Iter 1
Mass fluxes [kg/s]
  0   0.7
  1   0.7
  2   0.7
  3   0.7
Nodal pressures [Pa]
  0   0.7
  1   0.7
  2   0.7
Jacobian:
[  -10937.5         0         0         0         1        -1        -0  ]  [  -1171.88 ]
[         0     -1960         0         0        -0         1        -1  ]  [       686 ]
[         0         0     -3360         0        -0         1        -1  ]  [      1176 ]
[         0         0         0     -1400        -1        -0         1  ]  [       490 ]
[        -1         0         0         1         1        -0        -0  ]  [      -0.7 ]
[         1        -1        -1         0        -0        -0        -0  ]  [       0.7 ]
[         0         1         1        -1        -0        -0        -0  ]  [      -0.7 ]
deltaY
  0   0.0502287
  1   -0.318277
  2   -0.331495
  3   0.0502287
  4   -0.7
  5   621.798
  6   559.62

res = 113.429

*** Iter 2
Mass fluxes [kg/s]
  0   0.750229
  1   0.381723
  2   0.368505
  3   0.750229
Nodal pressures [Pa]
  0   9.22681e-11
  1   622.498
  2   560.32
Jacobian:
[  -11722.3         0         0         0         1        -1        -0  ]  [   19.7099 ]
[         0  -1068.83         0         0         0         1        -1  ]  [    141.82 ]
[         0         0  -1768.83         0         0         1        -1  ]  [   263.734 ]
[         0         0         0  -1500.46        -1        -0         1  ]  [   2.52288 ]
[        -1         0         0         1         1        -0        -0  ]  [ -2.01289e-11 ]
[         1        -1        -1         0         0        -0        -0  ]  [ 2.01288e-11 ]
[         0         1         1        -1         0        -0        -0  ]  [ -9.22681e-11 ]
deltaY
  0   -0.0151179
  1   0.033539
  2   -0.0486569
  3   -0.0151179
  4   -9.31074e-11
  5   157.507
  6   -20.1609

res = 2.33015

*** Iter 3
Mass fluxes [kg/s]
  0   0.735111
  1   0.415262
  2   0.319848
  3   0.735111
Nodal pressures [Pa]
  0   -8.39286e-13
  1   780.005
  2   540.159
Jacobian:
[  -11486.1         0         0         0         1        -1        -0  ]  [   1.78567 ]
[         0  -1162.74         0         0         0         1        -1  ]  [   1.57477 ]
[         0         0  -1535.28         0         0         1        -1  ]  [   5.68211 ]
[         0         0         0  -1470.22        -1        -0         1  ]  [  0.228566 ]
[        -1         0         0         1         1 -3.55041e-17 -3.55041e-17  ]  [ -4.23798e-17 ]
[         1        -1        -1         0         0        -0        -0  ]  [ -8.39218e-13 ]
[         0         1         1        -1         0        -0        -0  ]  [ 1.67855e-12 ]
deltaY
  0   -0.000393531
  1   0.00129842
  2   -0.00169195
  3   -0.000393531
  4   8.17525e-13
  5   2.73448
  6   -0.350013

res = 0.00278561

*** converged
Mass fluxes [kg/s]
  0   0.734717
  1   0.416561
  2   0.318157
  3   0.734717
Nodal pressures [Pa]
  0   -2.17612e-14
  1   782.739
  2   539.809



