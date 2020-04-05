#include<vts.h>
#include<z3-util.h>

/*********   N [here = 3] CNF  Encoding *************************************
 *
 *   lit_list ::= a1 C1 || a2 C2 || ... || an Cn 
 *   cl_list  ::= a1 + a2 + a3 + ... + an == N (here 3)
 *   il_list  ::= !Coeff (C1) || !Coeff (C2) 
 *   
 *   outer_list ::=  Sum_i { [lit_list] && [cl_list] && [il_list] } 
 * 
 * * *************************************************************************/

// Bool e :: edge or not (node) : [arg_list_2d, N, M, 0/1, N, E_arity]
z3::expr vts::literal_cnf ( Vec2Expr s, z3::expr_vector& sideCons, unsigned i, unsigned k, bool e, unsigned j = 0, unsigned q = 0 ) {

  z3::expr_vector outer_list(ctx);
  z3::expr_vector lit_listC(ctx);
  z3::expr_vector lit_listI(ctx);
  
  // d: no of clauses
  for ( unsigned d = 0; d < D; d++ ) {
    z3::expr_vector il_list(ctx);
    z3::expr_vector cl_list(ctx);
    z3::expr_vector inner_list(ctx);
    
    //std::cout << "value of k is :" << k << "\n";
    for ( unsigned k1 = 0; k1 < M; k1++ ) {
      if ( k1 == k )  continue;
      
      if ( e == true ) { 
        inner_list.push_back( ( s[d][k1] && presence_edge[i][j][q][k1] ) 
                          || ( s[d][k1+M] && !presence_edge[i][j][q][k1] ) );
      } else { 
        inner_list.push_back( ( s[d][k1] && nodes[i][k1] ) 
                           || ( s[d][k1+M] && !nodes[i][k1] ) );
      //std::cout << "value of k1 is: " << k1 << "\n" ; 
      //std::cout << "Interested var is: " << s[d][k1] << " and " << s[d][k1+M] << "\n";
      }
      
      cl_list.push_back( s[d][k1] );
      cl_list.push_back( s[d][k1+M] );
      il_list.push_back( !s[d][k1] || !s[d][k1+M] ) ;
    }

    // Exactly three : 3 CNF 
    // To make compuational challenge easy lets fix at_most_one`
    // auto cConst =  at_least_three ( cl_list) && ! at_least_four( cl_list );
    //auto cConst = !at_least_two( cl_list ); 
    auto cConst = !at_least_two( cl_list) && at_least_one( cl_list ); 
    lit_listC.push_back( cConst );
    
    auto iConst = mk_and ( il_list );
    lit_listI.push_back( iConst  );
    
    auto cons = mk_or ( inner_list ); 
    outer_list.push_back( cons );
  }

  auto c1 = mk_and( lit_listI );
  auto c2 = mk_and( lit_listC );
  auto c3 = mk_and( outer_list );
  sideCons.push_back( c1 && c2 );
  
  return c3;
}

/*** Node activity constraint: a[i][k]  ***/
z3::expr vts::node_cnf ( Vec3Expr& node_parameter_var ) {

  z3::expr_vector main_list(ctx);
  z3::expr_vector sideCons( ctx );
  for ( unsigned i = 0; i < N; i++ ) {
    for ( unsigned k = 0; k < M; k++ ) {
      auto cnf = (active_node[i][k] == literal_cnf( node_parameter_var[k], sideCons, i, k, false ) );
      main_list.push_back ( cnf ); 
    }
  }

  auto additionalCons = mk_and ( sideCons );
  auto cons = mk_and ( main_list );
  return (cons && additionalCons );
}

/*** Edge activity constraint: b[i][j][q][k]  ***/
z3::expr vts::edge_cnf ( Vec3Expr& edge_parameter_var ) {

  z3::expr_vector main_list(ctx);
  z3::expr_vector sideCons( ctx );
  for( unsigned i = 0 ; i < N; i++ ) {
    for( unsigned j = 0 ; j < N; j++ ) {
      if ( i == j )  continue;
      for ( unsigned q = 0; q < E_arity; q++ ) {
        for ( unsigned k = 0; k < M; k++ ) { 
          auto cnf =  ( active_edge[i][j][q][k] == literal_cnf( edge_parameter_var[k], sideCons, i, k, true, j, q ) ) ;  
          main_list.push_back ( cnf ); 
        }
      }
    }
  }
  
  auto additionalCons = mk_and ( sideCons );
  auto cons = mk_and ( main_list );
  return (cons && additionalCons );
}

/** Function has a restricted form with Three CNF  **/ 
z3::expr vts::cnf_function ( Vec3Expr& node_parameter_var, Vec3Expr& edge_parameter_var ) { 

  z3::expr nodeCnf = node_cnf ( node_parameter_var );
  //std::cout << nodeCnf << "\n";
  
  z3::expr edgeCnf = edge_cnf ( edge_parameter_var ); 
  //std::cout << edgeCnf << "\n";
  
  auto cons = nodeCnf && edgeCnf;
  //std::cout << cons;
  return cons;
}
