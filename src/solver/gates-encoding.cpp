#include<vts.h>
#include<z3-util.h>

/** Boolean Gates Implementation ***
 * L := a G L | a          a : Bool_var [args]   G : gate
 * G == AND | OR 
 */

// The expression can take any gates. gth gate.
z3::expr vts::gates( VecExpr& u, z3::expr x, z3::expr y ) {
  assert( u.size() == 1 );
  auto g1 = x && y;
  auto g2 = x || y;

  return ( ( u[0] && g1 )  ||  ( !u[0] && g2 ) );

  /** Implement in case of more than two gates **
  auto g3 = ctx.bool_val(true);
  auto g4 = x;
  auto g5 = y;
   z3::expr_vector g_list(ctx);
   z3::expr_vector c_list (ctx);

   c_list.push_back( u[0] );
   c_list.push_back( u[1] );

   // Exactly One
   auto cSum = at_least_one( c_list ) && !at_least_two( c_list );

   auto lsCons =  ( u[k][g][0] && g1 )  ||  ( u[k][g][1] && g2 );

   auto cons = cSum && lsCons;
   return cons;
   */
}

z3::expr vts::var_fml ( VecExpr& chooseVar, 
                        z3::expr_vector& sideConstraintFormula, 
                        z3::expr_vector& sideConstraintLiteral,
                        unsigned i, unsigned m, bool e,
                        unsigned j = 0, unsigned q = 0 ) {
  z3::expr_vector ls(ctx);
  z3::expr_vector cl_list(ctx);
  z3::expr_vector il_list(ctx);

  for ( unsigned m1 = 0; m1 < M; m1++ ) {
    if ( m1 == m )  continue;
    if ( e == true ) {
       ls.push_back( ( chooseVar[m1] && presence_edge [i][j][q][m1] ) 
                  || ( chooseVar[m1+M] && !presence_edge[i][j][q][m1] ) );
    } else{
       ls.push_back( ( chooseVar[m1] && nodes[i][m1] )  
                  || ( chooseVar[m1+M] && !nodes[i][m1] ) ); 
    }

    cl_list.push_back( chooseVar[m1] );
    cl_list.push_back( chooseVar[m1+M] );
    il_list.push_back( !chooseVar[m1] || !chooseVar[m1+M] ) ;
  }

  //ls.push_back( chooseVar[2*M] && ctx.bool_val(true) );
  //ls.push_back( chooseVar[(2*M) + 1] && ctx.bool_val(false) );
  ls.push_back( chooseVar[2*M] && true );
  ls.push_back( chooseVar[(2*M) + 1] && false );
  
  cl_list.push_back( chooseVar[2*M] );
  cl_list.push_back( chooseVar[(2*M) + 1] );

  auto varList = mk_or( ls );
  
  auto coeffSum = at_least_one( cl_list ) && at_most_one ( cl_list ); 
  sideConstraintFormula.push_back( coeffSum ); 
  auto litList = mk_and ( il_list );
  sideConstraintLiteral.push_back( litList );

  return varList;
}

// chooseVars : params for selecting variables at the leaf of functions
// chooseGates : params for selecting gates at the internal nodes of functions
// i : node number
// m1 : molecule
// isEdge : edge Or Node
// j : target node
// q : edge idx
z3::expr vts::build_rhs_fml ( Vec2Expr& chooseVars, Vec2Expr& chooseGates, 
                              z3::expr_vector& sideCons,
                              unsigned i, unsigned m,
                              bool isEdge,
                              unsigned j = 0, unsigned q = 0 ) { 
  z3::expr gfml(ctx);
  z3::expr_vector ls(ctx);
  z3::expr_vector n_list( ctx );
  z3::expr_vector main_list ( ctx );
  z3::expr_vector sideConstraintFormula( ctx );
  z3::expr_vector sideConstraintLiteral( ctx );
  unsigned gateVar = 0;    
  unsigned leaf_number = 4;

  for( unsigned l = 0; l < leaf_number; l++ ) {
    if ( isEdge == true )
      n_list.push_back( var_fml( chooseVars[l], sideConstraintFormula, sideConstraintLiteral, i, m, isEdge, j, q ) );
    else
      n_list.push_back( var_fml( chooseVars[l], sideConstraintFormula, sideConstraintLiteral, i, m, isEdge ) );
  }

  unsigned local_leaf_num = leaf_number;
  unsigned gate_counter = 0;
 
  while( local_leaf_num > 1 ) {
    for( unsigned l = 0; l < local_leaf_num; l = l + 2 ) {
      if( l == local_leaf_num - 1 ) {
        n_list[l>>1] = n_list[l];
      }else{
        n_list[l>>1] = gates( chooseGates[gateVar],
                              n_list[l], n_list[l+1] );
        gate_counter++;
      }
    }
    local_leaf_num = (local_leaf_num+1) >> 1;
  }
  assert( local_leaf_num == 1);
  sideCons.push_back ( z3::mk_and( sideConstraintFormula ) && z3::mk_and( sideConstraintLiteral ) );
  return n_list[0];
}


z3::expr vts::node_gate_fml ( Vec3Expr& chooseVars, 
                              Vec3Expr& chooseGates ) {
  z3::expr_vector sideCons( ctx );
  z3::expr_vector n_list(ctx);
  for( unsigned i = 0; i < N; i++ ) {
    for( unsigned m = 0; m < M; m++ ) {
      auto nfml = build_rhs_fml( chooseVars[m], chooseGates[m], sideCons,  i, m, false );
      auto fml = ( active_node[i][m] == nfml );
      n_list.push_back ( fml );
    }
  }
  auto additionCons = mk_and ( sideCons );
  auto cons = mk_and( n_list );
  return cons && additionCons;
}

z3::expr vts::edge_gate_fml ( Vec3Expr& chooseVars,
                              Vec3Expr& chooseGates ) {
  z3::expr_vector sideCons( ctx );
  z3::expr_vector e_list(ctx); 
  for( unsigned i = 0 ; i < N; i++ ) {
    for( unsigned j = 0 ; j < N; j++ ) {
      if ( i == j )  continue;
      for ( unsigned q = 0; q < E_arity; q++ ) {
        for ( unsigned m = 0; m < M; m++ ) {
          auto efml = build_rhs_fml ( chooseVars[m], chooseGates[m], sideCons, i, m, true, j, q );
          auto fml = ( active_edge[i][j][q][m] == efml ); 
          e_list.push_back ( fml );
        }
      }
    }
  }
  auto additionalCons = mk_and ( sideCons );
  auto cons = mk_and ( e_list );
  return cons && additionalCons;
}

z3::expr vts::logic_gates ( Vec3Expr& s_var, Vec3Expr& t_var, 
                            Vec3Expr& u_var, Vec3Expr& v_var ) {

  z3::expr nodeGate = node_gate_fml ( s_var, u_var );
  z3::expr edgeGate = edge_gate_fml ( t_var, v_var );
  
  auto cons = nodeGate && edgeGate;
  
  return cons;
}

