model HydraulicNetwork3
  Real mdot1;
  Real mdot2;
  Real mdot3;
  Real p1;
  Real p2;
  Real p3;
equation
  // fix pressure at one node
  p1 = 1000;

  // flow element equations  
  0 = p1 - p2 - 0.1*mdot1*mdot1;
  0 = p2 - p3 - 0.7*mdot2*mdot2;
  0 = p3 - p1 + 100; // try 0; // pump, adds pressure
  
  // nodal equations
  0 = mdot1 - mdot2;
  0 = mdot2 - mdot3;
  
  // Note: modelica does not allow formulation of closing equation, even though - after symbolic analysis
  //       one equation would disappear
//  0 = mdot3 + mdot1;
end HydraulicNetwork3;

