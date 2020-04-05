#include<vts.h>
#include<z3-util.h>
//#include <vector>
#include <iterator>

VecExpr vts::flattern_3d ( Vec3Expr& dump ) {
  VecExpr d1;
  for ( unsigned int i = 0; i < N; i++ ) {
    for( unsigned j = 0 ; j < N; j++ ) {
      if (j == i)
        continue;
      for ( unsigned q = 0; q < E_arity; q++ ) {
        d1.push_back( dump[i][j][q] );
     }
    }
  }
  return d1;
}

z3::expr vts::cardC (z3::expr_vector d1, unsigned drop_count) {
   z3::expr expr = ctx.int_val(0);
        auto tt = ctx.bool_val(true);
        for (unsigned i=0; i < d1.size(); i++ ) {
         expr = expr + z3::ite( d1[i], ctx.int_val(1), ctx.int_val(0) ) ;
       }
     // std::cout << "The total count is : " << expr;  
      return (tt && (expr == ctx.int_val(drop_count)) );
}


// Use PbEq for exactly k.
z3::expr vts::exactly_k_drops( unsigned drop_count, Vec3Expr& dump ) { //
  // D2: Flattening the array. Avoid i == j.
  VecExpr d1 = flattern_3d( dump );
  
  // Print the flattern arrar.
  // for ( auto& i: d1 ) {
  //	  std::cout << i  << "\n";
  // }
  // exit(0);

  //unsigned int L = d1.size();
  
  // Only have support for exactly 2 and 3.
  if ( drop_count == 1 ) {
    z3::expr al = at_least_one ( d1 );
    z3::expr am = at_least_two ( d1 );
    return ( al && !am );
  }

  if (drop_count == 2) {
    z3::expr al = at_least_two( d1 ); 
    z3::expr am = at_least_three( d1 ); 
    return ( al && !am );
  }
  
  else if (drop_count == 3) {
    z3::expr al = at_least_three( d1 ); 
    z3::expr am = at_least_four( d1 ); 
    return ( al && !am );
  } 
  
  else if (drop_count == 4) {
    z3::expr al = at_least_four( d1 ); 
    z3::expr am = at_least_five( d1 ); 
  } 
  
  else { // todo : fill the right code 
    z3::expr al = at_least_two( d1 ); 
    z3::expr am = at_least_three( d1 ); 
    return ( al && !am );
 }
  /* ite exactly 3 Implementation.
  z3::expr expr = ctx.int_val(0);
  auto tt = ctx.bool_val(true);
  for (auto& i: d1) {
    expr = expr + z3::ite( i, ctx.int_val(1), ctx.int_val(0) ) ;
  }
  //std::cout << "The total count is : " << expr;  
  return (tt && (expr == ctx.int_val(drop_count)) );
  */
  
  return ctx.bool_val(true);
}

/* Different type of Flattern */

z3::expr_vector vts::flattern2d ( Vec2Expr& dump, unsigned s1, unsigned s2, bool eq ) {
  z3::expr_vector d1(ctx);
  for ( unsigned i = 0; i < s1; i++ ) {
    for( unsigned j = 0 ; j < s2; j++ ) {
      if ( eq == true && j == i) continue;
      d1.push_back( dump[i][j] );
      //std::cout << dump[i][j] << "\n";
    }	    
  }
  return d1;
}

z3::expr_vector vts::flattern3d ( Vec3Expr& dump, unsigned s1, unsigned s2, unsigned s3, bool eq) {
  z3::expr_vector d1(ctx);
  for ( unsigned i = 0; i < s1; i++ ) {
    for( unsigned j = 0 ; j < s2; j++ ) {
      if ( eq == true && j == i) continue;
      for ( unsigned q = 0; q < s3; q++ ) {
        d1.push_back( dump[i][j][q] );	
      }		       
    }
  }
 return d1;
}

z3::expr_vector vts::flattern4d ( Vec4Expr& dump, unsigned s1, unsigned s2, unsigned s3, unsigned s4, bool eq ) {
  z3::expr_vector  d1(ctx);
  for ( unsigned i = 0; i < s1; i++ ) {
    for( unsigned j = 0 ; j < s2; j++ ) {
      if ( eq == true && j == i) continue;
      for ( unsigned q = 0; q < s3; q++ ) {
        for ( unsigned k = 0; k < s4; k++ ) {
          d1.push_back( dump[i][j][q][k] );	
        }
      }		       
    }
  }
 return d1;
}

// At least 1
z3::expr vts::at_least_one ( VecExpr dump ) {
  unsigned L = dump.size(); 
  if ( L < 1 ) {
    return ctx.bool_val(false);
   // return false;
   }
  z3::expr_vector ls(ctx);
  for ( unsigned i = 0; i < L; i++ ) {
    ls.push_back( dump [i] );
  }
  return z3::mk_or ( ls );
}

// At least 2 
z3::expr vts::at_least_two ( VecExpr dump ) {
  unsigned L = dump.size(); 
  z3::expr_vector ls(ctx);
  if ( L < 2 ) {
    return ctx.bool_val(false);
   // return false;
  }
  for ( unsigned i = 0; i < L-1; i++ ) {
    for ( unsigned j = i+1; j < L; j++) {
      ls.push_back ( dump[i] && dump[j] );  
    }
  }
  return z3::mk_or( ls );
}

// At least 3
z3::expr vts::at_least_three ( VecExpr dump ) {
  unsigned L = dump.size(); 
  if ( L < 3 ) {
    return ctx.bool_val(false);
    //return false;
  }
  z3::expr_vector ls(ctx);
  z3::expr lhs(ctx);
  for ( unsigned i = 0; i < L-2; i++ ) {
    for ( unsigned j = i+1; j < L-1; j++) {
      lhs =  ( dump[i] && dump[j] );  
      for ( unsigned k = j+1; k < L; k++) {
	      ls.push_back ( lhs && dump[k] );
    }
   }
  }
  return z3::mk_or( ls );
}


// At least 4. 
z3::expr vts::at_least_four ( VecExpr dump ) {
  unsigned L = dump.size(); 
  if ( L < 4 ) {
    return ctx.bool_val(false);
  }
  z3::expr_vector ls(ctx);
  z3::expr lhs(ctx);
  z3::expr lhs1(ctx);
  for ( unsigned i = 0; i < L-3; i++ ) {
    for ( unsigned j = i+1; j < L-2; j++ ) {
      lhs =  ( dump[i] && dump[j] );  
      for ( unsigned k = j+1; k < L-1; k++ ) {
        lhs1 = (lhs && dump[k]);
        for ( unsigned x = k+1; x < L; x++ ) {
	        ls.push_back ( lhs1 && dump[x] );
     }
    }
   }
  }
  return z3::mk_or( ls );
}

/** Overloaded z3::expr **/

// At least 5 
z3::expr vts::at_least_five ( VecExpr dump ) {
  unsigned L = dump.size(); 
  if ( L < 5 ) {
    return ctx.bool_val(false);
    //return false;
  }
  z3::expr_vector ls(ctx);
  z3::expr lhs(ctx);
  z3::expr lhs1(ctx);
  z3::expr lhs2(ctx);

  for ( unsigned i = 0; i < L-4; i++ ) {
    for ( unsigned j = i+1; j < L-3; j++ ) {
      lhs =  ( dump[i] && dump[j] );  
      for ( unsigned k = j+1; k < L-2; k++ ) {
        lhs1 = (lhs && dump[k]);
        for ( unsigned x = k+1; x < L-1; x++ ) {
          lhs2 = (lhs1 && dump[x]);
          for ( unsigned z = x + 1; z < L; z++ ) {
            ls.push_back ( lhs2 && dump[z] );
          }
        }
      }
    }
  } 
  return z3::mk_or( ls );
}

// At least 1
z3::expr vts::at_least_one ( z3::expr_vector dump ) {
  unsigned L = dump.size(); 
  z3::expr_vector ls(ctx);
  for ( unsigned i = 0; i < L; i++ ) {
    ls.push_back( dump [i] );
  }
  return z3::mk_or ( ls );
}

z3::expr vts::at_most_one ( z3::expr_vector dump ) {
  unsigned L = dump.size(); 
  z3::expr_vector ls(ctx);
  if ( L < 2 ) {
    return ctx.bool_val(true);
  }
  for ( unsigned i = 0; i < L-1; i++ ) {
    for ( unsigned j = i+1; j < L; j++) {
      ls.push_back ( !dump[i] || !dump[j] );  
    }
 }
  return z3::mk_and ( ls );
}


// At least 2 
z3::expr vts::at_least_two ( z3::expr_vector dump ) {

  unsigned L = dump.size(); 
  if ( L < 2 ) {
    return ctx.bool_val(false);
    //return false;
  }
  z3::expr_vector ls(ctx);

  for ( unsigned i = 0; i < L-1; i++ ) {
    for ( unsigned j = i+1; j < L; j++) {
      ls.push_back ( dump[i] && dump[j] );  
    }
  }

  return z3::mk_or( ls );
}

// At least 3 
z3::expr vts::at_least_three ( z3::expr_vector dump ) { 

  unsigned L = dump.size(); 
  if ( L < 3 ) {
    return ctx.bool_val(false);
    //return true;
  }
  z3::expr_vector ls(ctx);
  z3::expr lhs(ctx);
 
  for ( unsigned i = 0; i < L-2; i++ ) {
    for ( unsigned j = i+1; j < L-1; j++) {
      lhs =  ( dump[i] && dump[j] );  
      for ( unsigned k = j+1; k < L; k++) {
	      ls.push_back ( lhs && dump[k] );
    }
   }
  }

  return z3::mk_or( ls );
}

// At least 4. 
z3::expr vts::at_least_four ( z3::expr_vector dump ) {
  unsigned L = dump.size(); 
  if ( L < 4 ) {
    return ctx.bool_val(false);
    //return false;
  }

  z3::expr_vector ls(ctx);
  z3::expr lhs(ctx);
  z3::expr lhs1(ctx);

  for ( unsigned i = 0; i < L-3; i++ ) {
    for ( unsigned j = i+1; j < L-2; j++) {
      lhs =  ( dump[i] && dump[j] );  
      for ( unsigned k = j+1; k < L-1; k++) {
        lhs1 = (lhs && dump[k]);
        for ( unsigned x = k+1; x < L; x++) {
	        ls.push_back ( lhs1 && dump[x] );
     }
    }
   }
  }

  return z3::mk_or( ls );
}


z3::expr_vector vts::node_list() {
    return flattern2d ( nodes, N, M, false);
}

z3::expr_vector vts::active_node_list() {
    return flattern2d ( active_node, N, M, false );
}

z3::expr_vector vts::presence_edge_list() {
    return flattern4d ( presence_edge, N, N, E_arity, M, true );
}

z3::expr_vector vts::pairing_m_4d_list() {
    return flattern4d ( pairing_m_4d, N, N, E_arity, M, true );
}

z3::expr_vector vts::active_edge_list() {
    return flattern4d ( active_edge, N, N, E_arity, M, true );
}

z3::expr_vector vts::pairing_m_list() {
    return flattern2d ( pairing_m, M, M, true );
}

z3::expr_vector vts::reach_list() {
    return flattern4d ( reach, N, N, M, N-1, true );
}

z3::expr_vector vts::edge_list() {
    return flattern3d ( edges, N, N, E_arity, true );
}


//------------------------------------------
// Adder circuit

std::pair< z3::expr, z3::expr >
mk_full_adder(  z3::expr i1, z3::expr i2, z3::expr cin ) {
  z3::expr out = _xor( _xor( i1, i2 ), cin);
  z3::expr cout = (i1 && i2) || (i2 && cin) || (cin && i1);
  return std::make_pair( out, cout );
}


void
mk_adder( const std::vector<z3::expr>& i1,
          const std::vector<z3::expr>& i2,
          std::vector<z3::expr>& result ) {
  assert( i1.size() > 0);
  assert( i1.size() == i2.size() );
  auto& ctx = i1[0].ctx();
  result.clear();
  z3::expr cin = ctx.bool_val(false);
  for(unsigned i = 0; i < i1.size(); i++) {
    auto out_pair = mk_full_adder(i1[i], i2[i], cin );
    result.push_back( out_pair.first );
    cin = out_pair.second;
  }
  result.push_back( cin );
}


void
mk_adder_pair( const std::vector< std::vector<z3::expr> >& in_nums,
               std::vector< std::vector<z3::expr> >& results ) {
  auto& ctx = in_nums[0][0].ctx();
  unsigned r_count = 0;
  std::vector<z3::expr> dummy;
  for( unsigned i = 0; i < in_nums.size()-1; i = i + 2  ) {
    results.push_back( dummy );
    mk_adder( in_nums[i],  in_nums[i+1], results[r_count] );
    r_count++;
  }
  if( in_nums.size() % 2 == 1 ) {
    auto& last_num = in_nums.back();
    results.push_back( last_num );
    results.back().push_back( ctx.bool_val(false) );
  }
}

void
collect_sum( std::vector< std::vector<z3::expr> >& ins ) {
  assert( ins.size() > 0 );
  while( ins.size() > 1 ) {
    std::vector< std::vector<z3::expr> > results;
    mk_adder_pair( ins, results );
    assert( ins.size() > results.size() );
    ins = results;
  }
}

std::vector<z3::expr>
sum_bits( const std::vector<z3::expr>& in_bits ) {
  std::vector< std::vector<z3::expr> > ins;
  for( auto b : in_bits ) {
    std::vector<z3::expr> bit_vec;
    bit_vec.push_back( b );
    ins.push_back( bit_vec );
  }
  collect_sum( ins );
  return ins[0];
}

std::vector<z3::expr>
sum_bits( const z3::expr_vector& in_bits ) {
  std::vector< std::vector<z3::expr> > ins;
  for( unsigned i =0; i < in_bits.size(); i++ ) {
    std::vector<z3::expr> bit_vec;
    bit_vec.push_back( in_bits[i] );
    ins.push_back( bit_vec );
  }
  collect_sum( ins );
  return ins[0];
}


inline int get_bit(unsigned val, unsigned idx) {
    unsigned mask = 1 << (idx & 31);
    return (val & mask) != 0;
}

z3::expr
mk_bv_le_k( const std::vector<z3::expr>& in_num, unsigned k ) {
  assert( in_num.size() > 0 );
  auto& ctx = in_num[0].ctx();
  z3::expr out(ctx);
  out = get_bit(k, 0) ? ctx.bool_val(true) : !in_num[0];
  for (unsigned idx = 1; idx < in_num.size(); idx++) {
    if( get_bit(k, idx) ) {
      out = out || !in_num[idx];
    } else {
      out = !in_num[idx] && out;
    }
  }
  return out;
}

//------------------------------------------


z3::expr
mk_le_k_bits( const std::vector<z3::expr>& bits, unsigned k ) {
  auto sum = sum_bits( bits );
  return mk_bv_le_k( sum, k );
  // auto& ctx = bits[0].ctx();
  // z3::expr_vector args(c);
  // for (unsigned i = 0; i < es.size(); i++)
  //   args.push_back( es[i] );
  // return z3::atmost( args, k );
}

z3::expr
mk_le_k_bits( const z3::expr_vector& bits, unsigned k ) {
  auto sum = sum_bits( bits );
  return mk_bv_le_k( sum, k );
  // return z3::atmost( args, k );
}
