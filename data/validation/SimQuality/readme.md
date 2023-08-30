The *SimQuality* test suite checks for correct calculation of individual
model aspects and combined effects.

The test suite is organized in test cases (german "TestFall" hence "TF" prefix of directories),
with usually several simulation projects.

The subdirectory `climate` contains climate data files in `c6b` format (created as
described in the test suite).

## Validation Results

NOTE: The *NANDRAD 2* solver is the calculation kernel of _SIM-VICUS_.

### TestCase 01 - Sonnenstandsberechnung

<img src="TF01/results/TF01.5_Azimuth.png" width="600px" height="400px"/><img src="TF01/results/TF01.5_Elevation.png" width="600" height="400"/>

### TestCase 02 - Solare Lasten auf opaque Bauteile

<img src="TF02/results/TF02.W90Diff.png" width="400px"/><img src="TF02/results/TF02.W90Dir.png" width="400px"/>

### TestCase 03 - Wärmeleitung

<img src="TF03/results/TF03.1.png" width="400px"/><img src="TF03/results/TF03.1.png" width="400px"/><br>
<img src="TF03/results/TF03.3.png" width="400px"/><img src="TF03/results/TF03.4.png" width="400px"/>

### TestCase 04 - Wärmeleitung- und Speicherung (1- und 2-Raummodelle)

<img src="TF04/results/TF04.1.png" width="400px"/><img src="TF04/results/TF04.2.png" width="400px"/><br>
<img src="TF04/results/TF04.3A.png" width="400px"/><img src="TF04/results/TF04.3B.png" width="400px"/><br>
<img src="TF04/results/TF04.4A.png" width="400px"/><img src="TF04/results/TF04.4B.png" width="400px"/>

### TestCase 05 - Natürliche Lüftung

<img src="TF05/results/TF05.1.png" width="400px"/><img src="TF05/results/TF05.2.png" width="400px"/><br>
<img src="TF05/results/TF05.3.png" width="400px"/>

### TestCase 06 - Solarstrahlung und Wärmeleitung (nur opaque Bauteile)

#### Variante 1

<img src="TF06/results/TF06.1.png" width="800px" height="550px"/><img src="TF06/results/TF06.1_radloads.png" width="800px" height="550px"/>

#### Variante 2

<img src="TF06/results/TF06.2.png" width="800px" height="550px"/><img src="TF06/results/TF06.2_radloads.png" width="800px" height="550px"/>


### TestCase 07 - Solare Gewinne durch Fenster

#### Variante 1 - Nur Wärmeleitung durch das Fenster

<img src="TF07/results/TF07.1.png" width="400px"/><img src="TF07/results/TF07.1_heatFlux.png" width="400px"/><br>

#### Variante 2 - Konstanter g-Wert (SHGC)

<img src="TF07/results/TF07.2.png" width="400px"/><img src="TF07/results/TF07.2_radLoad.png" width="400px"/><br>
<img src="TF07/results/TF07.2_heatCondWall.png" width="400px"/>

#### Variante 3 - Winkelabhängiger g-Wert (SHGC)

<img src="TF07/results/TF07.3.png" width="400px"/><img src="TF07/results/TF07.3_radLoad.png" width="400px"/><br>
<img src="TF07/results/TF07.3_heatCondWall.png" width="400px"/>


### TestCase 08 - Innere Lasten (konvektiv/radiativ; konstant und mit Zeitplan)

<img src="TF08/results/TF08.1.png" width="600px" height="400px"/><br>
<img src="TF08/results/TF08.2.png" width="600px" height="400px"/><br>
<img src="TF08/results/TF08.3.png" width="600px" height="400px"/><br>
<img src="TF08/results/TF08.4.png" width="600px" height="400px"/>

### TestCase 09 - Externe & dynamische Verschattung

<img src="TF09/results/TF09.1.png" width="600px" height="400px"/><br>
<img src="TF09/results/TF09.2.png" width="600px" height="400px"/><br>
<img src="TF09/results/TF09.3.png" width="600px" height="400px"/><br>
<img src="TF09/results/TF09.4.png" width="600px" height="400px"/><br>
<img src="TF09/results/TF09.5.png" width="600px" height="400px"/><br>
<img src="TF09/results/TF09.6.png" width="600px" height="400px"/><br>
