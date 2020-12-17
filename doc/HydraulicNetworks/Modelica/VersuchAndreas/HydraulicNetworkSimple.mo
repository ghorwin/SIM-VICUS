model HydraulicNetwork
  Real mdot1(start=0.2);
  Real mdot2(start=0.2);
  Real p1;
  Real p2;
equation
  // fix pressure at one node
  p1 = 0;

  // flow element equations  
  0 = p1 - p2 + 0.1*mdot1*mdot1;
  0 = p1 - p2 + 100;
  
  // nodal equations
  0 = mdot1 + mdot2;
end HydraulicNetwork;

