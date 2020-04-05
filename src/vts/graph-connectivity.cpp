#include<vts.h>
#include<z3-util.h>

// z3::expr vts::k_connected_graph_constraint ( unsigned K, bool eQuantVar ) {

//   /***** Building [[1]] ****/
//   VecExpr ee_set = flattern_3d ( edges );
//   //edgeQuant = ee_set;

//   z3::expr_vector setR1 = flattern2d ( d_reach1, N, N, true );
//   z3::expr_vector setR2 = flattern2d ( d_reach2, N, N, true );
  
//   // Create:  Exists (setR1, reach_d1 && d1_all-conn )
//   z3::expr is_reach1 = exists( setR1, reachability_under_drop_def( d_reach1 , drop1, 0 ) && remains_connected( d_reach1 )  ); 
//   z3::expr is_reach2 = exists( setR2, reachability_under_drop_def( d_reach2 , drop2, 1 ) && gets_disconnected( d_reach2 )  );
  
//   z3::expr_vector setD1 = flattern3d ( drop1, N, N, E_arity, true );
//   z3::expr_vector setD2 = flattern3d ( drop2, N, N, E_arity, true );
  
//   z3::expr k_min_1_connected = forall ( setD1, implies 
// 		  (  (exactly_k_drops ( K-1, drop1 ) && only_present_edges_can_be_dropped ( drop1 )), is_reach1 ) );
 
//   z3::expr k_not_connected = exists ( setD2,  
// 		    (exactly_k_drops ( K, drop2 ) && only_present_edges_can_be_dropped ( drop2 )) && is_reach2 ) ;
 
//   z3::expr at_least_k_edges = at_least_three( ee_set );
  
//   // [[1]] : K Connected Graph 
//   // EXISTS [ setE, kedges && k-1Conn && knotConn ]
//   z3::expr_vector setE = flattern3d ( edges, N, N, E_arity, true );

//   if ( eQuantVar ) { 
//     z3::expr cons = exists ( setE, at_least_k_edges && k_min_1_connected && k_not_connected );
//     return cons;
//   }else {
//     z3::expr cons = at_least_k_edges && k_min_1_connected && k_not_connected;
//     return cons;
//   }

// }

z3::expr vts::k_connected_graph_constraint ( unsigned K, bool eQuantVar ) {

  /***** Building [[1]] ****/
  VecExpr ee_set = flattern_3d ( edges );
  //edgeQuant = ee_set;

  z3::expr_vector setR1 = flattern3d ( d_reach1, N, N, N, true );
  z3::expr_vector setR2 = flattern2d ( d_reach2, N, N, true );
  
  // Create:  Exists (setR1, reach_d1 && d1_all-conn )
  z3::expr is_reach1 = exists( setR1, reachability_under_drop_def( d_reach1 , drop1) && remains_connected( d_reach1 )  );
  z3::expr is_reach2 = exists( setR2, reachability_under_drop_def( d_reach2 , drop2, 1 ) && gets_disconnected( d_reach2 )  );
  
  z3::expr_vector setD1 = flattern3d ( drop1, N, N, E_arity, true );
  z3::expr_vector setD2 = flattern3d ( drop2, N, N, E_arity, true );
  
  z3::expr k_min_1_connected = forall ( setD1, implies 
		  (  (exactly_k_drops ( K-1, drop1 ) && only_present_edges_can_be_dropped ( drop1 )), is_reach1 ) );
    // std::cout << k_min_1_connected;

  z3::expr k_not_connected = exists ( setD2,  
		    (exactly_k_drops ( K, drop2 ) && only_present_edges_can_be_dropped ( drop2 )) && is_reach2 ) ;
 
	// TODO: Implement a linear encoding
  z3::expr at_least_k_edges = at_least_three( ee_set );
  
  // [[1]] : K Connected Graph 
  // EXISTS [ setE, kedges && k-1Conn && knotConn ]
  z3::expr_vector setE = flattern3d ( edges, N, N, E_arity, true );

  if ( eQuantVar ) { 
    z3::expr cons = exists ( setE, at_least_k_edges && k_min_1_connected && k_not_connected );
    return cons;
  }else {
    z3::expr cons = at_least_k_edges && k_min_1_connected && k_not_connected;
    return cons;
  }

}
