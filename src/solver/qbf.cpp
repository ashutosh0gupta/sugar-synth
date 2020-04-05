#include<vts.h>
#include<z3-util.h>

//#include <vector>
//#include <iterator>

z3::expr vts::create_qbf_formula ( int funcType ) {

  /** Build Constrint Of the Form := [[1]] && [[2]] && [[3]]
   * [[1]] :: Connectivity Constraint : kConnected Graph
   * [[2]] :: V5 Constraint : Self edge not allowed
   * [[3]] :: Function Requirements :
   *          - No Boolean func exists.
   *          - Function structure: 3 CNF. 
   */

  /***** Building [[1]] ****/
  VecExpr ee_list = flattern_3d ( edges );
  z3::expr_vector listE = edge_list();

  // Arg true -> u need edge quantified outside.
  z3::expr kconnectedConstraint = k_connected_graph_constraint ( C, false );

  /***** Building [[2]] ****/
  // [[2]] : V5  
  z3::expr V5 = no_self_edges();                              
  
  /***** Building [[3]] ****/
  z3::expr_vector listN = node_list();
  z3::expr_vector listActiveN = active_node_list(); 
  z3::expr_vector listPresentE = presence_edge_list();
  z3::expr_vector listActiveE = active_edge_list();
  z3::expr_vector listPairingM = pairing_m_list();
  z3::expr_vector listReach = reach_list();

  /* Avoid writing forall( x, forall( y, .... ))
  z3::expr_vector qvarQbf( ctx ); 

  qvarQbf.reserve( listN.size() + listActiveN.size() + listPresentE.size() + listActiveE.size() + listPairingM.size() + listReach.size() ); // preallocate memory
  
  qvarQbf.insert( qvarQbf.end(), listN.begin(), listN.end() );
  qvarQbf.insert( qvarQbf.end(), listActiveN.begin(), listActiveN.end() );
  qvarQbf.insert( qvarQbf.end(), listPresentE.begin(), listPresentE.end() );
  qvarQbf.insert( qvarQbf.end(), listActiveE.begin(), listActiveE.end() );
  qvarQbf.insert( qvarQbf.end(), listPairingM.begin(), listPairingM.end() );
  qvarQbf.insert( qvarQbf.end(), listReach.begin(), listReach.end() );
  
  */
  //for (const auto&& vect : listN ) {
  //  std::cout << vect << "\n";
  //}
  
  // Basic constraints with stability excluding V5 :: self edge
  z3::expr vtsBasicStability = vts_basic_constraints() && vts_stability_constraint();
  z3::expr vtsFusion = ! ( vts_fusion_constraint() ); 

  // [3]: Not a function constraint.
  z3::expr notaFunction = not_a_function( nodes, active_node );
  
  /* Final Qbf Constraint: [[1]] && [[2]] && [[3]] */
  if ( funcType == 1 ) {
    
    // Populate xtra var node_parameter_var : var for node function
    popl3 ( node_parameter_var, M, 2 * M, D, "s" );
    
    // Populate xtra var edge_parameter_var : var for node function 
    popl3 ( edge_parameter_var, M, 2 * M, D, "t" );
  
    // [3]: N-CNF function 
    auto listSvar = flattern3d ( node_parameter_var, M, 2*M, D, false );
    auto listTvar = flattern3d ( edge_parameter_var, M, 2*M, D, false );
  
    z3::expr cnfCons = cnf_function( node_parameter_var, edge_parameter_var );

    z3::expr func3cnf  = forall( listN, 
                         forall( listActiveN, 
                         forall( listPresentE, 
                         forall( listActiveE, 
                         forall( listPairingM, 
                         forall( listReach, 
                         forall( listSvar, 
                         forall( listTvar,  
                                 implies( vtsBasicStability && cnfCons, vtsFusion )))))))));

    z3::expr qbfCons = exists( listE, 
                               kconnectedConstraint && V5 && func3cnf );  

   // z3::expr qbfCons = exists ( listTvar, exists (listSvar, (exists (listE, ( V5 && func3cnf )))) );  
   
    return qbfCons;

  }
  else if ( funcType == 2 ) {
    unsigned noOfGates = 3;
    
    // Populate xtra var node_parameter_var : var for node function
    popl3 ( node_parameter_var, M, M, (2 * M) + 2, "s" );
   
    // Populate xtra var edge_parameter_var : var for node function 
    popl3 ( edge_parameter_var, M, M, (2 * M) + 2, "t" );
    
    // Populate parameter var
    popl3 ( gate_selector_var_node, M, noOfGates, 2, "u" );
    
    // Populate parameter var
    popl3 ( gate_selector_var_edge, M, noOfGates, 2, "v" );
    
    // [3]: Boolean gates  function 
    /*
     * auto listSvar = flattern3d ( node_parameter_var, M, M - 1, 2*M + 2, false );
    auto listTvar = flattern3d ( edge_parameter_var, M, M - 1, 2*M + 2, false );
    auto listUvar = flattern2d ( gate_selector_var_node, M, M, false );
    auto listVvar = flattern2d ( gate_selector_var_edge, M, M, false );
     */
    auto listSvar = flattern3d ( node_parameter_var, M, M, 2*M + 2, false );
    auto listTvar = flattern3d ( edge_parameter_var, M, M, 2*M + 2, false );
    auto listUvar = flattern3d ( gate_selector_var_node, M, noOfGates, 2, false );
    auto listVvar = flattern3d ( gate_selector_var_edge, M, noOfGates, 2, false );
  
    z3::expr gateCons = logic_gates ( node_parameter_var, edge_parameter_var, gate_selector_var_node, gate_selector_var_edge );
    
    z3::expr funcGate  = forall( listN, 
                         forall( listActiveN, 
                         forall( listPresentE, 
                         forall( listActiveE, 
                         forall( listPairingM, 
                         forall( listReach, 
                         forall( listSvar, 
                         forall( listTvar,  
                         forall( listUvar, 
                         forall( listVvar, 
                                 implies ( vtsBasicStability && gateCons, vtsFusion )))))))))));

    z3::expr cons = exists( listE, 
                            kconnectedConstraint && V5 && funcGate );  

    return cons;

  }else {
    
    // No function possible constraint [[3]]
    // FORALL [ qvars, basicConst => notafunc ]
    z3::expr noFunctionPossible = forall( listN, 
                                  forall( listActiveN, 
                                  forall( listPresentE, 
                                  forall( listActiveE, 
                                  forall( listPairingM, 
                                  forall( listReach , 
                                          implies( vtsBasicStability && vtsFusion, notaFunction )))))));

    z3::expr qbfCons = exists( listE, 
                               kconnectedConstraint && V5 && noFunctionPossible );   
    return qbfCons;
  } 
  
  /*** Print the list of built edges and the formula ***/
  // std::cout << "Expected first level quant: " << set_edges << "\n";
  //std::cout << cons << "\n";
  //return kconnectedConstraint;
  
  //return qbfCons;

}

// Z3 home made QBf solver and return model
void vts::use_z3_qbf_solver ( z3::expr cons ) {

  // Print the input formula.
  // std::cout << cons << "\n";
  
  z3::solver s(ctx);

  // making sure model based quantifier instantiation is enabled.
  z3::params p(ctx);
  p.set("mbqi", true);
  s.set(p);

  s.add( cons );
  
 if( s.check() == z3::sat ) {
   std::cout << "The formula is SAT \n";
   z3::model m = s.get_model();
   //std::cout << m << "\n";
   std::cout << m << "\n";
   //return m;
  }else{
    std::cout << "model is not feasible!";
    assert(false);
 }
  //return s.get_model();
}
