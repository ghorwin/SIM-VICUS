model NetworkReferenceSimVicus
  replaceable package MediumWater = Modelica.Media.Water.ConstantPropertyLiquidWater;
  Modelica.Fluid.Pipes.StaticPipe pipe6(redeclare package Medium = MediumWater, diameter = 0.0256, length = 100, roughness = 7e-6) annotation(
    Placement(visible = true, transformation(origin = {34, -14}, extent = {{10, -10}, {-10, 10}}, rotation = 0)));
  Modelica.Fluid.Pipes.StaticPipe pipe4(redeclare package Medium = MediumWater, diameter = 0.0256, length = 100, roughness = 7e-6) annotation(
    Placement(visible = true, transformation(origin = {64, -52}, extent = {{-10, -10}, {10, 10}}, rotation = 90)));
  Modelica.Fluid.Sources.Boundary_pT inlet(redeclare package Medium = MediumWater, nPorts = 2, p = 10000) annotation(
    Placement(visible = true, transformation(origin = {14, -82}, extent = {{-10, -10}, {10, 10}}, rotation = 0)));
  Modelica.Fluid.Sources.FixedBoundary outlet(redeclare package Medium = MediumWater, nPorts = 1, p = 0) annotation(
    Placement(visible = true, transformation(origin = {-28, -80}, extent = {{10, -10}, {-10, 10}}, rotation = 0)));
  Modelica.Fluid.Fittings.SimpleGenericOrifice orifice(redeclare package Medium = MediumWater, diameter = 0.0256, zeta = 1000)  annotation(
    Placement(visible = true, transformation(origin = {-4, -14}, extent = {{10, -10}, {-10, 10}}, rotation = 0)));
  Modelica.Fluid.Pipes.StaticPipe pipe7(redeclare package Medium = MediumWater, diameter = 0.0256, length = 100, roughness = 7e-6) annotation(
    Placement(visible = true, transformation(origin = {-48, -14}, extent = {{10, -10}, {-10, 10}}, rotation = 0)));
  Modelica.Fluid.Pipes.StaticPipe pipe5(redeclare package Medium = MediumWater, diameter = 0.0256, length = 100, roughness = 7e-6)  annotation(
    Placement(visible = true, transformation(origin = {-82, -50}, extent = {{10, -10}, {-10, 10}}, rotation = 90)));
  Modelica.Fluid.Pipes.StaticPipe pipe8(redeclare package Medium = MediumWater, diameter = 0.0256, length = 100, roughness = 7e-6) annotation(
    Placement(visible = true, transformation(origin = {64, 44}, extent = {{10, -10}, {-10, 10}}, rotation = -90)));
  Modelica.Fluid.Pipes.StaticPipe pipe10(redeclare package Medium = MediumWater, diameter = 0.0256, length = 100, roughness = 7e-6) annotation(
    Placement(visible = true, transformation(origin = {26, 30}, extent = {{10, -10}, {-10, 10}}, rotation = 0)));
  Modelica.Fluid.Pipes.StaticPipe pipe12(redeclare package Medium = MediumWater, diameter = 0.0256, length = 100, roughness = 7e-6) annotation(
    Placement(visible = true, transformation(origin = {24, 74}, extent = {{10, -10}, {-10, 10}}, rotation = 0)));
  Modelica.Fluid.Fittings.SimpleGenericOrifice orifice3(redeclare package Medium = MediumWater, diameter = 0.0256, zeta = 1000) annotation(
    Placement(visible = true, transformation(origin = {-10, 30}, extent = {{10, -10}, {-10, 10}}, rotation = 0)));
  Modelica.Fluid.Fittings.SimpleGenericOrifice orifice2(redeclare package Medium = MediumWater, diameter = 0.0256, zeta = 1000) annotation(
    Placement(visible = true, transformation(origin = {-10, 74}, extent = {{10, -10}, {-10, 10}}, rotation = 0)));
  Modelica.Fluid.Pipes.StaticPipe pipe11(redeclare package Medium = MediumWater, diameter = 0.0256, length = 100, roughness = 7e-6) annotation(
    Placement(visible = true, transformation(origin = {-46, 30}, extent = {{10, -10}, {-10, 10}}, rotation = 0)));
  Modelica.Fluid.Pipes.StaticPipe pipe13(redeclare package Medium = MediumWater, diameter = 0.0256, length = 100, roughness = 7e-6) annotation(
    Placement(visible = true, transformation(origin = {-44, 74}, extent = {{10, -10}, {-10, 10}}, rotation = 0)));
  Modelica.Fluid.Pipes.StaticPipe pipe9(redeclare package Medium = MediumWater, diameter = 0.0256, length = 100, roughness = 7e-6) annotation(
    Placement(visible = true, transformation(origin = {-82, 24}, extent = {{-10, -10}, {10, 10}}, rotation = -90)));
equation
  connect(pipe7.port_b, pipe5.port_a) annotation(
    Line(points = {{-58, -14}, {-82, -14}, {-82, -40}}, color = {0, 127, 255}));
  connect(pipe5.port_b, outlet.ports[1]) annotation(
    Line(points = {{-82, -60}, {-82, -80}, {-38, -80}}, color = {0, 127, 255}));
  connect(pipe9.port_b, pipe5.port_a) annotation(
    Line(points = {{-82, 14}, {-82, -40}}, color = {0, 127, 255}));
  connect(pipe11.port_b, pipe9.port_a) annotation(
    Line(points = {{-56, 30}, {-68, 30}, {-68, 58}, {-82, 58}, {-82, 34}}, color = {0, 127, 255}));
  connect(pipe13.port_b, pipe9.port_a) annotation(
    Line(points = {{-54, 74}, {-68, 74}, {-68, 58}, {-82, 58}, {-82, 34}}, color = {0, 127, 255}));
  connect(orifice2.port_b, pipe13.port_a) annotation(
    Line(points = {{-20, 74}, {-34, 74}}, color = {0, 127, 255}));
  connect(pipe12.port_b, orifice2.port_a) annotation(
    Line(points = {{14, 74}, {0, 74}, {0, 74}, {0, 74}}, color = {0, 127, 255}));
  connect(orifice3.port_b, pipe11.port_a) annotation(
    Line(points = {{-20, 30}, {-36, 30}}, color = {0, 127, 255}));
  connect(pipe10.port_b, orifice3.port_a) annotation(
    Line(points = {{16, 30}, {0, 30}, {0, 30}, {0, 30}}, color = {0, 127, 255}));
  connect(pipe8.port_b, pipe10.port_a) annotation(
    Line(points = {{64, 54}, {46, 54}, {46, 30}, {36, 30}, {36, 30}}, color = {0, 127, 255}));
  connect(pipe8.port_b, pipe12.port_a) annotation(
    Line(points = {{64, 54}, {46, 54}, {46, 74}, {34, 74}, {34, 74}}, color = {0, 127, 255}));
  connect(pipe4.port_b, pipe8.port_a) annotation(
    Line(points = {{64, -42}, {64, 34}}, color = {0, 127, 255}));
  connect(orifice.port_b, pipe7.port_a) annotation(
    Line(points = {{-14, -14}, {-38, -14}, {-38, -14}, {-38, -14}}, color = {0, 127, 255}));
  connect(pipe6.port_b, orifice.port_a) annotation(
    Line(points = {{24, -14}, {6, -14}, {6, -14}, {6, -14}}, color = {0, 127, 255}));
  connect(pipe4.port_b, pipe6.port_a) annotation(
    Line(points = {{64, -42}, {64, -42}, {64, -14}, {44, -14}, {44, -14}}, color = {0, 127, 255}));
  connect(inlet.ports[1], pipe4.port_a) annotation(
    Line(points = {{24, -82}, {64, -82}, {64, -62}, {64, -62}}, color = {0, 127, 255}));
  annotation(
    uses(Modelica(version = "3.2.3")),
    experiment(StartTime = 0, StopTime = 100, Tolerance = 1e-06, Interval = 0.2),
    __OpenModelica_simulationFlags(emit_protected = "()", lv = "LOG_STATS", outputFormat = "mat", s = "dassl"));
end NetworkReferenceSimVicus;
