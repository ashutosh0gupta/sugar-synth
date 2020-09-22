#include <sugar/sugar-encode.h>
#include <stack>

//----------------
// missing constraints
//   * rule children match with sugar children
//   * is sorting constraints correct


unsigned get_true_idx( const VecExpr& bits, z3::model& m ) {
  unsigned i = 0;
  for(; i < bits.size(); i++ ) {
    if( is_true( bits.at(i), m ) ) {
      break;
    }
  }
  return i;
}

//-------------------------------------------------------------------------
//  mol_cons: object for holding constraint bits for molecule nodes
//-------------------------------------------------------------------------

mol_cons::mol_cons( z3::context& ctx_, sugar_mol_ptr m_, unsigned rule_size )
  : ctx(ctx_), m(m_.get()), cut_bit(ctx), tstamp(ctx),
    compartment(ctx), local_constraints(ctx)
{
  cut_bit = get_fresh_bool( ctx, "cut" );
  tstamp = get_fresh_int( ctx, "ts" );
  compartment = get_fresh_int( ctx, "c" );
  rule_match = get_fresh_bool_vec( ctx, rule_size, "rm" );

  local_constraints = atmost( ctx, rule_match, 1 ) &&
    z3::implies( cut_bit, atleast( ctx, rule_match, 1 ) );
}

mol_cons_ptr
mol_cons::make( z3::context& ctx_, sugar_mol_ptr m_, unsigned rule_size ) {
  return std::make_shared<mol_cons>( ctx_, m_, rule_size );
}

void
mol_cons::collect_local_cons( VecExpr& l_cons) const {
  assert(m);
  for( unsigned i = 0; i < m->get_children_num(); i++ ) {
    if( m->get_child(i) ) {
      auto cons_i = m->get_child(i)->get_cons();
      cons_i->collect_local_cons( l_cons );
    }
  }
  l_cons.push_back( local_constraints );
}

// mk concrete constraints for sugar mol node
mol_cons::mol_cons( z3::context& ctx_, sugar_mol_ptr m_, unsigned rule_size,
                    unsigned matched_rule, bool is_cut, int time, int comp )
  : ctx(ctx_), m(m_.get()), cut_bit(ctx), tstamp(ctx),
    compartment(ctx), local_constraints(ctx) {
  cut_bit = is_cut ? mk_true( ctx_ ) : mk_false( ctx_ );
  tstamp = ctx.int_val( time );
  compartment = ctx.int_val( comp );
  for( unsigned i = 0; i < rule_size; i++ ) {
    auto b = (i == matched_rule) ? mk_true( ctx_ ) : mk_false( ctx_ );
    rule_match.push_back( b );
  }
  local_constraints = mk_true( ctx );
}

mol_cons_ptr
mol_cons::make( z3::context& c_, sugar_mol_ptr m_, unsigned r_size,
                unsigned m_rule, bool is_cut, int t, int comp ) {
  return std::make_shared<mol_cons>(c_, m_, r_size, m_rule, is_cut, t, comp);
}

unsigned mol_cons::read_rule( z3::model& m) const {
  return get_true_idx( get_rule_bits(), m );
}

bool mol_cons::read_cut_bit( z3::model& m) const {
  return is_true( cut_bit, m );
}

int mol_cons::read_tstamp( z3::model& m) const {
  return  get_int_val( tstamp, m );
}

std::string mol_cons::
read_tstamp_as_string( z3::model& m) const {
  return get_int_val_as_string( tstamp, m );
}

int mol_cons::read_compartment( z3::model& m) const {
  return  get_int_val( compartment, m );
}

std::string mol_cons::
read_compartment_as_string( z3::model& m) const {
  return get_int_val_as_string( compartment, m );
}

void mol_cons::collect_variables( expr_set& vars ) {
  get_variables( cut_bit, vars );
  get_variables( tstamp, vars );
  get_variables( compartment, vars );
  for( auto v : rule_match) get_variables( v, vars );
}

//-------------------------------------------------------------------------
//  unknown_cons :
//-------------------------------------------------------------------------

unknown_label::unknown_label( z3::context& ctx_, sugar_mol_ptr m_,
                              unsigned sugar_size )
  : ctx(ctx_), m(m_.get()), local_constraints(ctx) {
  node_sugar = get_fresh_bool_vec( ctx, sugar_size, "s" );

  local_constraints = atmost( ctx, node_sugar, 1 );
}

z3::expr unknown_label::get_exists_cons() {
  return mk_or( ctx, node_sugar );
}

unknown_label_ptr
unknown_label::make( z3::context& ctx_, sugar_mol_ptr m_, unsigned sugar_size ) {
  return std::make_shared<unknown_label>( ctx_, m_, sugar_size );
}

void
unknown_label::collect_local_cons( VecExpr& l_cons) const {
  assert(m);
  for( unsigned i = 0; i < m->get_children_num(); i++ ) {
    auto s_cons = m->get_unknown_sugar()->get_exists_cons();
    auto mi = m->get_child(i);
    if( mi ) {
      mi->get_unknown_sugar()->collect_local_cons( l_cons );
      auto si_cons = mi->get_unknown_sugar()->get_exists_cons();
      l_cons.push_back( z3::implies( si_cons, s_cons ) );
    }
  }
  l_cons.push_back( local_constraints );
}

sugar_mol_ptr sugar_encoding::
make_unknown_mol( unsigned depth, unsigned num_children ) {
  if( depth > 0 ) {
    sugar_mol_vec m_vec;
    make_unknown_mol( depth-1, num_children, m_vec );
    sugar_t_ptr s = nullptr;
    auto m = sugar_mol::make( s, m_vec );
    auto m_cons = mol_cons::make( ctx, m, rs.size() );
    m->set_cons( m_cons );
    m->set_unknown_label( unknown_label::make( ctx, m, sugars.size() ) );
    assert( rs.size() > 0 );
    if( depth < m_depth_max ) {
    // if( depth < rs.at(0)->get_depth() ) {
      auto exts = m->get_unknown_sugar()->get_exists_cons();
      unknown_m_ls_exist.push_back( exts );
    }
    return m;
  }else{
    return nullptr;
  }
}

void sugar_encoding::
make_unknown_mol( unsigned depth, unsigned num_children,
                  sugar_mol_vec& rs ) {
  for( unsigned i = 0; i < num_children; i++ ) {
    auto r = make_unknown_mol( depth, num_children );
    rs.push_back( r );
  }
}

//-------------------------------------------------------------------------
//  sugar_encoding : encoding object
//-------------------------------------------------------------------------

sugar_encoding::
sugar_encoding( z3::context& ctx_, sugar_t_vec& sugars,
                sugar_mol_vec& seed_ms_, sugar_mol_vec& ms_,
                std::vector< std::vector<unsigned> > group_mols_,
                unsigned num_rules, unsigned rule_depth_,
                unsigned max_compartments_ )
  : ctx(ctx_)
  , sugars(sugars)
  , pos_ms(ms_)
  , group_mols(group_mols_)
  , rule_depth(rule_depth_)
  , rule_max_children(0)
  , max_compartments( max_compartments_ )
  , unknown_m(nullptr)
{
  for( sugar_t_ptr s : sugars) {
    auto c =  s->get_children_num();
    if( c > rule_max_children ) rule_max_children = c;
  }
  rule::make( ctx, sugars, num_rules, sugars.size(), rule_depth,
              rule_max_children, false, rs );
  // conrete rules
  rule::make( ctx, sugars, num_rules, sugars.size(), rule_depth,
              rule_max_children, true, rs_conc );

  // we needto initialzied compartment variable because
  // it is same for each subnode
  for( auto r : rs) {
    r->initialize_non_recursive( max_compartments );
    // adds constraints that rules must have non empty condition nodes
    r->add_no_empty_condition();
  }

  // seed rules
  unsigned m_seed_depth_max = 0;
  for( auto s_m : seed_ms_ ) {
    if( s_m->get_depth() > m_seed_depth_max )
      m_seed_depth_max = s_m->get_depth();
  }
  rule::make( ctx, sugars, seed_ms_.size(), sugars.size(), m_seed_depth_max,
              rule_max_children, true, seed_rs );
  unsigned i = 0;
  for( auto s_m :  seed_ms_ ) {
    //todo: no initialize called on the variables
    construct_seed_rule( seed_rs[i], s_m );
    i++;
  }
  for( auto m : pos_ms ) {
    if( m->get_depth() > m_depth_max )
      m_depth_max = m->get_depth();
  }
  //negative sample variable tree
  unknown_m = make_unknown_mol( m_depth_max, rule_max_children );
}
//-------------------------------------------------------------------------

void  sugar_encoding::
distinct_mols( const sugar_mol_ptr& m1, const sugar_mol_ptr& m2,
               VecExpr& dists ) {
  if( m1 == nullptr && m2 == nullptr ) return;
  if( m1 && m2 == nullptr ) {
    if( m1->get_sugar() )
      dists.push_back( mk_true( ctx ) );
    else
      dists.push_back( m1->get_unknown_sugar()->get_exists_cons() );
    return;
  }
  if( m2 && m1 == nullptr ) {
    if( m2->get_sugar() )
      dists.push_back( mk_true( ctx ) );
    else
      dists.push_back( m2->get_unknown_sugar()->get_exists_cons() );
    return;
  }
  assert( m1 );
  assert( m2 );
  // match sugar
  if( m1->get_sugar() && m1->get_sugar() == m2->get_sugar() ) {
    // nothing is to be done
  }else if( m1->get_sugar() && m2->get_sugar() == nullptr ) {
    auto m1_s = m1->get_sugar_number();
    dists.push_back( !m2->get_unknown_sugar()->get_sugar_bit( m1_s ) );
  }else if( m2->get_sugar() && m1->get_sugar() == nullptr ) {
    auto m2_s = m2->get_sugar_number();
    dists.push_back( !m1->get_unknown_sugar()->get_sugar_bit( m2_s ) );
  }else if( m2->get_sugar() != m1->get_sugar() ) {
    dists.push_back( mk_true( ctx ) );
    return;
    // no more matching is needed, we have found a concerete distinction
  }else{
    assert( m1->get_sugar() == nullptr );
    assert( m2->get_sugar() == nullptr );
    auto u1 = m1->get_unknown_sugar();
    auto u2 = m2->get_unknown_sugar();
    for( unsigned i = 0; i < u1->get_num_sugar_bits(); i++ ) {
      dists.push_back( u1->get_sugar_bit(i) && !u2->get_sugar_bit(i));
    }
  }

  // distinct children
  auto mp1 =  m2->get_children_num() < m1->get_children_num() ? m2 : m1;
  auto mp2 =  m2->get_children_num() < m1->get_children_num() ? m1 : m2;

  // mp2 has more children than mp1
  for( unsigned i = 0; i < mp1->get_children_num(); i++ ) {
    distinct_mols( mp1->get_child(i), mp2->get_child(i), dists );
  }
  // match the excess children
  for( unsigned i =mp1->get_children_num();i < mp2->get_children_num(); i++) {
    distinct_mols( nullptr, mp2->get_child(i), dists );
  }
}

z3::expr  sugar_encoding::
distinct_mol_pairs( const sugar_mol_ptr& m1, const sugar_mol_ptr& m2 ) {
  VecExpr dists;
  distinct_mols( m1,  m2, dists );
  return mk_or( ctx, dists );
}


// // In a tree v1----------->v2
// // check if there is a repeating subtree rooted at v2

// z3::expr sugar_encoding::
// eq_rules( const sugar_mol_ptr& m1, const sugar_mol_ptr& m2) {
//   auto m1_rule_bits = m1->get_cons()->get_rule_bits();
//   auto m2_rule_bits = m2->get_cons()->get_rule_bits();
//   VecExpr eqs;
//   for( unsigned i=0 ; i < mp_rule_bits.size(); i++ ) {
//     eqs.push_back( m1_rule_bits[i] == m2_rule_bits[i] );
//   }
//   return mk_and( ctx, equal);
// }


// unsigned repeat_search_depth = 3

// z3::expr sugar_encoding::
// is_repeat_subtree_in_branch_with_tail( const sugar_mol_ptr& m,
//                                        const sugar_mol_ptr& tail_to_match ) {
//   // No impact return
//   return mk_true( ctx );
//   auto m_rule_bits = m->get_cons()->get_rule_bits();
//   VecExpr no_equal;
//   VecExpr match_sub_tree;
//   const std::vector<sugar_mol*> parent_trail,
//   parent_trail.push_back( m );
//   sugar_mol* mp_head = m->get_parent();
//   unsigned depth = 1;

//   while( mp_head ) {
//     if( repeat_search_depth < depth ) break;
//     depth = depth + 1;
//     parent_trail.push_back( mp_head );
//     for( unsigned i = 1; i < depth-1; i++ ) {
//       auto& m_head = parent_trail[i];
//       // m_head and m_head subtrees
//       // rule_eq = eq_rules( m_head, mp_head );
//       tree_match_rules = match_sub_tree( m_head, mp_head );
//       match_sub_tree.push_back( tree_match_rules );
//     }

//     //same rules
//     mp_head = mp_head->get_parent();
//   }
//   return mk_and( ctx, no_equal );
// }

z3::expr sugar_encoding::match( const sugar_mol_ptr& root1, const sugar_mol_ptr& root2,const sugar_mol_ptr& stop){
  
  if(root1 == stop){return mk_true(ctx);}
  else if(root1==nullptr && root2==nullptr){return mk_true(ctx);}
  else if((root2==nullptr && root1!=nullptr) || (root1==nullptr && root2!=nullptr)){return mk_false(ctx);}
  else{
    VecExpr v1;
    auto uu = root1->get_unknown_sugar();
    auto uv = root2->get_unknown_sugar();
    auto m1_rule_bits = root1->get_cons()->get_rule_bits();
    auto m2_rule_bits = root2->get_cons()->get_rule_bits();
    for( unsigned i=0 ; i < m1_rule_bits.size(); i++ ) {
        v1.push_back( m1_rule_bits[i] == m2_rule_bits[i] ); // rule same for runaway, comparrtment same for runaway, compare at each level
      }
    for(unsigned i = 0; i < uu->get_num_sugar_bits(); i++ ){
      v1.push_back(uu->get_sugar_bit(i)==uv->get_sugar_bit(i));
    }
    for( unsigned i = 0; i < root1->get_children_num(); i++ ){
        v1.push_back(match(root1->get_child(i),root2->get_child(i), stop));
     }
     return mk_and(ctx,v1);
}}
z3::expr sugar_encoding::exact_match(const sugar_mol_ptr& m1, const sugar_mol_ptr& mt){
  if(m1==nullptr && mt==nullptr){return mk_true(ctx);}
  else if((mt==nullptr && m1!=nullptr) || (m1==nullptr && mt!=nullptr)){return mk_false(ctx);}
  else{
    auto m1_n = m1->get_sugar_number();
    auto us = mt->get_unknown_sugar();
    VecExpr same;
    for(unsigned i = 0; i < us->get_num_sugar_bits(); i++ ){
        if(i==m1_n){same.push_back(us->get_sugar_bit(i));}
        else{same.push_back(!us->get_sugar_bit(i));}
      }
    for( unsigned i = 0; i < m1->get_children_num(); i++ ){
        same.push_back(exact_match(m1->get_child(i),mt->get_child(i)));
      }
      return mk_and(ctx,same);
  }
}

// z3::expr sugar_encoding::compare(const sugar_mol_ptr& m1, const sugar_mol_ptr& m2, int maxdepth, int rmax, int tdmax){  //m1 concrete, m2 unknown
//   if(m1==nullptr && m2==nullptr){return mk_true(ctx);} // termination condn- max r , max total depth. compile (use make)
//   else if((m2==nullptr && m1!=nullptr) || (m1==nullptr && m2!=nullptr)){return mk_false(ctx);} // termination condn- max r , max total depth. compile (use make)
//   else {
//     auto m1_n = m1->get_sugar_number();
//     auto us = m2->get_unknown_sugar();
//     // either same
//     if(tdmax==0){
//       return mk_true(ctx);}
//     else{
//       VecExpr same;
//       for(unsigned i = 0; i < us->get_num_sugar_bits(); i++ ){
//         if(i==m1_n){same.push_back(us->get_sugar_bit(i));}
//         else{same.push_back(!us->get_sugar_bit(i));}
//       }
//       for( unsigned i = 0; i < m1->get_children_num(); i++ ){
//         VecExpr v2;
//         v2.push_back(compare(m1->get_child(i),m2->get_child(i),maxdepth,rmax,tdmax-1));
//         same.push_back(mk_and(ctx,v2)); // trace back from m2 max depth 3 constraint in implies
//       }
//       // or repeat
//       VecExpr rep;
//       rep.push_back(!us->get_sugar_bit(m1_n));
//       sugar_mol_ptr m = m2;
//       sugar_mol_ptr m3 = m2;
//       sugar_mol_ptr m4 = m2;
//       int maxd = maxdepth;
//       if(rmax==0){return mk_true(ctx);}
//       std::cerr<<"@@@@@";
//       while(maxd>0){
//         int d = maxd;
//         std::stack<int> s;
//         VecExpr temp;
//         sugar_mol* mp = m->get_parent();
//         auto m_rule_bits = m->get_cons()->get_rule_bits();
//         while(mp!= nullptr && d>0){
//           s.push(m->get_sibling_num());
//           auto mp_rule_bits = mp->get_cons()->get_rule_bits();
//           for( unsigned i=0 ; i < mp_rule_bits.size(); i++ ) {
//             temp.push_back( m_rule_bits[i] == mp_rule_bits[i] ); // rule same for runaway, comparrtment same for runaway, compare at each level
//           }
//           temp.push_back(m->get_cons()->get_compartment()==mp->get_cons()->get_compartment());
//           mp=mp->get_parent();
//           d--;
//           if(d==0){
//             for(unsigned i = 0; i < us->get_num_sugar_bits(); i++ ){
//               temp.push_back((m->get_unknown_sugar()->get_sugar_bit(i) == us->get_sugar_bit(i)));
//             }
//           }
//         }
//         while(m4!=nullptr && !s.empty()){
//           int n =s.top();
//           s.pop();
//           m4=m4->get_child(n);
//         }

//         VecExpr vec=match(m3,m,m2);
//         vec.push_back(compare(m1,m4,maxdepth,rmax-1,tdmax-maxd));
//         rep.push_back(z3::implies(mk_and(ctx,temp),mk_and(ctx,vec))); // temp => (match for rep && tail)
//         maxd--;
//       }
//       VecExpr final;
//       final.push_back(mk_and(ctx,same));
//       final.push_back(mk_and(ctx,rep));
//       return mk_or(ctx,final);
//     }
//   }
// }
z3::expr sugar_encoding::compare(const sugar_mol_ptr& m1, const sugar_mol_ptr& m2, int d, int r){
  if(m1==nullptr && m2==nullptr){return mk_true(ctx);}
  else if((m2==nullptr && m1!=nullptr) || (m1==nullptr && m2!=nullptr)){return mk_false(ctx);}
  else {
    auto m1_n = m1->get_sugar_number();
    VecExpr rep;
    sugar_mol_ptr m4 = m2; //m2 is const
    for(int i=0;i<r;i++){
      if(m4==nullptr){return mk_false(ctx);}
      auto us = m4->get_unknown_sugar();    
      rep.push_back(!us->get_sugar_bit(m1_n));      
      sugar_mol_ptr m3 = m4; //for matching repeating subtree, stays at curr m2      
      std::stack<int> s;
      sugar_mol* mp = m4->get_parent();
      if(i!=r-1){s.push(m4->get_sibling_num());}      
      for(int j=0;j<d;j++){ 
          if (mp==nullptr)
          {
            return mk_false(ctx);
          } 
          if(j!=d-1){s.push(mp->get_sibling_num());}          
          rep.push_back(m4->get_cons()->get_compartment()==mp->get_cons()->get_compartment());
          if(j!=d-1){mp=mp->get_parent(); }
      }    
      rep.push_back(match( std::make_shared<sugar_mol>(*mp),m3,m4));     
      while(!s.empty()){
      if(m4==nullptr){return mk_false(ctx);}
      int n =s.top();
      s.pop();
      m4=m4->get_child(n);
      }
      if(i==r-1){
        if(m4==nullptr){return mk_false(ctx);}
        rep.push_back(exact_match(m1,m4->get_child(m1->get_sibling_num())));
      }      
    }    
    //dump(rep);
    return mk_and(ctx,rep);
    
  }
}
z3::expr sugar_encoding::no_repeat_in_branch( const sugar_mol_ptr& m1, const sugar_mol_ptr& m2){
  // No impact return
  //return mk_true( ctx );
  VecExpr v;
  int maxd = 3;
  int maxr = 3;
  for(int i=1;i<=maxd;i++){
    for(int j=0;j<=maxr;j++){
      v.push_back(compare(m1,m2,i,j));
    }
  }
  return z3::operator!(mk_or(ctx,v));
}
// add no repeat options
// z3::expr sugar_encoding::
// no_repeat_in_branch( const sugar_mol_ptr& m ) {
//   // No impact return
//   return mk_true( ctx );
//   auto m_rule_bits = m->get_cons()->get_rule_bits();
//   VecExpr no_equal;
//   sugar_mol* mp = m->get_parent();
//   while( mp ) {
//     auto mp_rule_bits = m->get_cons()->get_rule_bits();
//     // if they are same then disable
//     VecExpr diseq;
//     for( unsigned i=0 ; i < mp_rule_bits.size(); i++ ) {
//       diseq.push_back( m_rule_bits[i] != mp_rule_bits[i] ); // rule same for runaway, comparrtment same for runaway, m2 is unknown so use those variables
//     }
//     no_equal.push_back( mk_or( ctx, no_equal) );
//     mp = mp->get_parent();
//     // parent_trail.push_back( mp );
//   }
//   return mk_and( ctx, no_equal );
// }

// m2 is not a sub-tree of m1
void  sugar_encoding::
not_contain_mols( const sugar_mol_ptr& m1, const sugar_mol_ptr& m2,
                  VecExpr& dists,
                  VecExpr& strict,
                  VecExpr& blocked ) {
  if( m2 == nullptr ) {
    // should not happen
    // it does then it needs no extension property
    assert( m1 == nullptr );
    return;
  }
  if( m1 == nullptr ) {
    // If M2 is bigger
    if( m2->get_sugar() ) {
      dists.push_back( no_repeat_in_branch( m1, m2) );
    }else {
      auto u2 = m2->get_unknown_sugar();
      dists.push_back( u2->get_exists_cons() && no_repeat_in_branch( m1, m2 ) );
    }
    // needed if we were counting!
    // for( unsigned i = 0; i < m2->get_children_num(); i++ ) {
    //   not_contain_mols( nullptr, m2->get_child(i), dists );
    // }
    return;
  }
  assert( m1 );
  assert( m2 );
  // match sugar
  if( m1->get_sugar() && m1->get_sugar() == m2->get_sugar() ) {
    // nothing is to be done
  }else if( m1->get_sugar() && m2->get_sugar() == nullptr ) {
    // (m2 there and different from m1)
    auto m1_s = m1->get_sugar_number();
    auto u2 = m2->get_unknown_sugar();
    VecExpr m2_diff_m1;
    for( unsigned i = 0; i < u2->get_num_sugar_bits(); i++ ) {
      if( i != m1_s )
        m2_diff_m1.push_back( u2->get_sugar_bit(i) );
    }
    dists.push_back( mk_or( ctx, m2_diff_m1) && no_repeat_in_branch( m1, m2 )  );
    // if m2 is not there, then m2 no more extendible
    auto not_m2_and_m1 = !u2->get_exists_cons( );
    if( m2->get_parent() ) {
      //std::cout<<"yes, parent exists"<<std::endl;
      not_m2_and_m1 = not_m2_and_m1 &&
        m2->get_parent()->get_unknown_sugar()->get_exists_cons();
    }    
    strict.push_back( not_m2_and_m1 );
    VecExpr no_matches;
    no_fast_can_extend_unknown(m2,no_matches);
    blocked.push_back( z3::implies( not_m2_and_m1, // z3::operator!
                                    (mk_and(ctx,no_matches) ) ));
    //diagnostics_cons.push_back( not_m2_and_m1 );
    //diagnostics_cons.push_back( (mk_and(ctx,no_matches) ) );
  }else if( m2->get_sugar() && m1->get_sugar() == nullptr ) {
    auto m2_s = m2->get_sugar_number();
    //m2 is there, m1 is not there
    dists.push_back( !m1->get_unknown_sugar()->get_sugar_bit( m2_s )
                     && no_repeat_in_branch( m1, m2 ) );
  }else if( m2->get_sugar() != m1->get_sugar() ) {
    dists.push_back( mk_true( ctx ) );
    return;
    // no more matching is needed, we have found a concerete distinction
  }else{
    assert( false ); // never occurs; may occur in future usage
    assert( m1->get_sugar() == nullptr );
    assert( m2->get_sugar() == nullptr );
    auto u1 = m1->get_unknown_sugar();
    auto u2 = m2->get_unknown_sugar();

    VecExpr m2_big_m1;
    for( unsigned i = 0; i < u1->get_num_sugar_bits(); i++ ) {
      m2_big_m1.push_back( !u1->get_sugar_bit(i) && u2->get_sugar_bit(i));
    }
    dists.push_back( mk_or( ctx, m2_big_m1) && no_repeat_in_branch( m1,  m2 )  );
    //m2 is there, m1 is not there
    auto m2_and_not_m1 = !u1->get_exists_cons() && u2->get_exists_cons();
    if( m2->get_parent() ) {
      m2_and_not_m1 = m2_and_not_m1 &&
        m2->get_parent()->get_unknown_sugar()->get_exists_cons();
    }
    strict.push_back( m2_and_not_m1 );
    VecExpr no_matches2;
    no_fast_can_extend_unknown(m2,no_matches2);
    blocked.push_back( z3::implies( m2_and_not_m1, //z3::operator!
                                    (mk_and(ctx,no_matches2)) ) );
  }

  for( unsigned i = 0; i < m2->get_children_num(); i++ ) {
    if( i < m1->get_children_num() ) {
      not_contain_mols(m1->get_child(i),m2->get_child(i),dists,strict,blocked);
    }else{
      not_contain_mols(nullptr, m2->get_child(i), dists,strict,blocked );
    }
  }

}

// prior vs containment
z3::expr  sugar_encoding::
not_contain_mols( const sugar_mol_ptr& m1, const sugar_mol_ptr& m2 ) {
  VecExpr dists;
  VecExpr strict;
  VecExpr blocked;
  not_contain_mols( m1,  m2, dists, strict, blocked );
  //m1 contains m2 and all expansions of m2 are blocked
  // VecExpr no_matches3;
  // no_fast_can_extend(m2,no_matches3);
  //diagnostics_cons.push_back( mk_and(ctx ,blocked) );
  //dump( mk_or(ctx,strict) && mk_and(ctx ,blocked) );
  return mk_or( ctx, dists ) || (mk_or(ctx,strict) && mk_and(ctx ,blocked));
  //return mk_or( ctx, dists ) || mk_or(ctx,strict);
}

// m2 does not contain in m1 or m2 is not extendable




//-------------------------------------------------------------------------

// void
// distinct_rules( const rule_ptr& r1, const rule_ptr& r2, VecExpr& dists ) {
//   assert( r1 );
//   assert( r2 );
//   assert( r1->get_depth() == r2->get_depth() );
//   assert( r1->get_num_sugar_bits() == r2->get_num_sugar_bits() );
//   for( unsigned i = 0; i < r1->get_num_sugar_bits(); i++ ) {
//     dists.push_back(r1->get_sugar_bit(i) && !r2->get_sugar_bit(i));
//   }
//   if( r1->get_depth() > 1 ) {
//     for( unsigned i = 0; i < r1->get_children_num(); i++ ) {
//       distinct_rules( r1->get_child(i), r2->get_child(i), dists );
//     }
//   }
// }

// z3::expr distinct_rule_pairs( const rule_ptr& r1, const rule_ptr& r2 ) {
//   VecExpr dists;
//   distinct_rules( r1,  r2, dists );
//   return mk_or( r1->get_context(), dists );
// }

//-------------------------------------------------------------------------



z3::expr sugar_encoding::distinct_rules() {
  // We only need sorted rules
  VecExpr dists;
  // for( unsigned i = 0; i < rs.size(); i++ ) {
  //   rule_ptr r1 = rs[i];
  //   for( unsigned j = 0; j < rs.size(); j++ ) {
  //     rule_ptr r2 = rs[j];
  //     dists.push_back( distinct_rule_pairs( r1,  r2 ) );
  //   }
  // }
  for( unsigned i = 1; i < rs.size(); i++ ) {
    rule_ptr r1 = rs[i-1];
    rule_ptr r2 = rs[i];
    dists.push_back( rule::sorted( r1, r2 ) );
  }
  if( rs.size() > 0 ) {
    dists.push_back( rs[0]->get_compartment() == 0 );
  }
  return mk_and( ctx, dists );
}

//-------------------------------------------------------------------------

void sugar_encoding::initialize_mol_cons( sugar_mol_ptr& m ) {
  if( m == nullptr ) return;
  for( unsigned i = 0; i < m->get_children_num(); i++ ) {
    sugar_mol_ptr m_i = m->get_child(i);
    initialize_mol_cons( m_i );
  }
  auto cons_ptr = mol_cons::make( ctx, m, rs.size() );
  m->set_cons( cons_ptr );
}

void eq_sugar( const VecExpr& sus1, const VecExpr& sus2, VecExpr& eqs ) {
  assert( sus1.size() == sus2.size());
  for( unsigned i = 0; i < sus1.size(); i++ ) {
    eqs.push_back( !sus1.at(i) || sus2.at(i) );
  }
}

z3::expr sugar_encoding::
same_sugar( const VecExpr& su_bits_1, const VecExpr& su_bits_2 ) {
  VecExpr eqs;
  eq_sugar( su_bits_1, su_bits_2, eqs );
  return mk_and( ctx, eqs );
}

// void sugar_encoding::
// collect_match_vars( z3::expr r_parent_occurs, rule_ptr r, sugar_mol_ptr m,
//                     z3::expr mark_stamp, bool is_match_part, VecExpr& sbits ) {
//   if( r == nullptr && m == nullptr ) return;
//   auto r_occurs = mk_false(ctx);
//   if( r ) r_occurs = r->get_occur_cons();
//   if( !is_match_part ) {
//     // cut matching is needed expand part not in mach part
//     auto cut = mk_true(ctx);
//     if( m ) {
//       cut = m->get_cons()->get_cut_bit();
//       if( m->get_sugar() == nullptr )
//         cut = z3::implies( m->get_unknown_sugar()->get_exists_cons(), cut );
//     }
//     sbits.push_back( z3::implies( r_parent_occurs, r_occurs != cut ) );
//   }
//   if( m ) {
//     if( r ) {
//       z3::expr s_bit(ctx);
//       if( m->get_sugar() ) {
//         s_bit = r->get_sugar_bit( m->get_sugar_number() );
//       }else{
//         s_bit = same_sugar( r->get_sugar_bits(),
//                             m->get_unknown_sugar()->get_sugar_bits());
//       }
//       z3::expr stamp_cons(ctx);
//       if( is_match_part )
//         stamp_cons = mark_stamp > m->get_cons()->get_tstamp();
//       else
//         stamp_cons = m->get_cons()->get_tstamp() >= mark_stamp;
//       sbits.push_back( z3::implies( r_occurs, s_bit && stamp_cons ) );
//       for( unsigned i = 0; i < m->get_children_num(); i++ ) {
//         auto ri = r->get_child(i);
//         auto mi = m->get_child(i);
//         collect_match_vars(r_occurs, ri, mi, mark_stamp, is_match_part, sbits);
//       }
//     }
//   }else{
//     sbits.push_back( !r_occurs );
//   }
// }

bool free_extend_positions = false;

void sugar_encoding::
match_tree(  rule_ptr r, sugar_mol_ptr m, z3::expr mark, bool is_match_part,
             VecExpr& match, rule_ptr r_parent, bool b ) {
  if( r == nullptr ) {
    if( mark ) if( free_extend_positions && is_match_part && m != nullptr ) {
        match.push_back( m->get_cons()->get_tstamp() > mark );
    }
    return;
  }
  if( m == nullptr ) { match.push_back( !r->get_occur_cons() ); return; }
  z3::expr s_bit =
    m->get_sugar() ? r->get_sugar_bit( m->get_sugar_number() )
                   : same_sugar( r->get_sugar_bits(),
                                 m->get_unknown_sugar()->get_sugar_bits() );
  z3::expr tcons = mk_true( ctx );
  if( mark ) {
    auto m_tstamp = m->get_cons()->get_tstamp();
    tcons = is_match_part ? mark > m_tstamp : m_tstamp >= mark;
    if( free_extend_positions && is_match_part ) {
      match.push_back( z3::implies( !r->get_occur_cons(), m_tstamp > mark ) );
    }
  }
  z3::expr r_comp = r->get_compartment();
  z3::expr ccons = is_match_part ? r_comp >= m->get_cons()->get_compartment()
                                 : m->get_cons()->get_compartment() == r_comp;
  match.push_back( z3::implies( r->get_occur_cons(), s_bit && tcons && ccons));
  if(b){
    if (mark){
      match.push_back( z3::implies(  !r->get_occur_cons() && r_parent->get_occur_cons() && r->get_is_hard_end(),  m->get_cons()->get_tstamp() > mark  ));  
    }    
  }  
  for( unsigned i = 0; i < m->get_children_num(); i++ ) {    
    match_tree( r->get_child(i), m->get_child(i), mark, is_match_part, match, r, b );
  }
}

void sugar_encoding::
match_cuts( z3::expr r_parent_occurs, rule_ptr r, sugar_mol_ptr m,
            VecExpr& sbits ) {
  if ( m == nullptr ) return; //since match is done in match trees no worries
  z3::expr cut = m->get_cons()->get_cut_bit();
  if( m->get_sugar() == nullptr )
      cut = z3::implies( m->get_unknown_sugar()->get_exists_cons(), cut );
  if( r == nullptr ) {
    sbits.push_back( z3::implies( r_parent_occurs, cut) );
    return;
  }
  z3::expr r_occurs = r->get_occur_cons();
  sbits.push_back( z3::implies( r_parent_occurs, r_occurs != cut ) );
  for( unsigned i = 0; i < m->get_children_num(); i++ ) {
    match_cuts(r_occurs, r->get_child(i), m->get_child(i), sbits);
  }
}

rule_ptr sugar_encoding::
match_condition( rule_ptr curr_r,
                 const std::vector<sugar_mol*>& parent_trail,
                 const std::vector< unsigned >& sibling_trail,
                 z3::expr mark,
                 // z3::expr r_comp,
                 VecExpr& match ) {
  z3::expr r_comp = curr_r->get_compartment();
  //trail rule
  auto p_rit = parent_trail.rbegin();
  auto rit = sibling_trail.rbegin();
  // rule_ptr curr_r = r;
  for(; rit != sibling_trail.rend(); rit++ ) {
    auto p = *p_rit; p_rit++;
    auto sibling_num = *rit;
    if( p->get_sugar() ) {
      match.push_back( curr_r->get_sugar_bit( p->get_sugar_number() ) );           
    }else{
      eq_sugar( curr_r->get_sugar_bits(),
                p->get_unknown_sugar()->get_sugar_bits(), match );
    }
    match.push_back( curr_r->get_is_extended()                  );
    match.push_back( curr_r->get_is_condition()                 );
    if( mark ) // check if mark is dummpy or not
      match.push_back( mark > p->get_cons()->get_tstamp()       );
    match.push_back( r_comp >= p->get_cons()->get_compartment() );

    // match condition parts parts of rules
    for( unsigned k = 0; k < p->get_children_num(); k++ ) {
      if( k == sibling_num ) continue;
      match_tree( curr_r->get_child(k), p->get_child(k), mark, true, match, curr_r,true );
    }
    curr_r = curr_r->get_child( sibling_num );
  }
  match.push_back( curr_r->get_is_extended()  );
  match.push_back( !curr_r->get_is_condition() );
  return curr_r;
}


//
//           m_parent : parent_comp
//             /
//            /  < r : r_comp
//           /                   \<--- added in r_comp + 1 should be used for matching
//          m : m_comp, m_time   /
//
//

void sugar_encoding::
no_fast_match_cons( sugar_mol* m_parent, unsigned m_child_num,
                    z3::expr m_comp, VecExpr& r_no_matches ) {
  if( m_parent == NULL ) return;
  for( unsigned i=0; i < rs.size(); i++ ) {
    auto r = rs.at(i);

    VecExpr no_matches;
    std::vector<sugar_mol*> parent_trail;
    std::vector< unsigned > sibling_trail;
    auto m_g = m_parent;
    sibling_trail.push_back( m_child_num );
    for( unsigned d = 1; d < r->get_depth(); d++ ) {
      if( m_g == NULL ) break;
      parent_trail.push_back( m_g );
      VecExpr chk_match;
      z3::expr dummy_mark(ctx); // creates null smart pointer
      //todo: some thing is wrong r_comp endtime is not passed?
      match_condition(r, parent_trail, sibling_trail, dummy_mark,chk_match);
      no_matches.push_back( !mk_and(ctx, chk_match ) );
      sibling_trail.push_back( m_g->get_sibling_num() );
      m_g = m_g->get_parent();
    }

    // constraints for infinite stay model!
    auto r_comp = r->get_compartment();
    auto no_m_cond = m_parent->get_cons()->get_compartment() <= r_comp;
    if( m_comp ) {
      // add conditions for saturation of the fast rules in the compartments
      // from rules upto m_comp
      no_m_cond = no_m_cond && r->get_is_fast() ;//&& r_comp <= m_comp;
    }
    no_m_cond = no_m_cond.simplify();
    auto no_match = z3::implies( no_m_cond, mk_and( ctx, no_matches ) );
    //diagnostics_cons.push_back(no_m_cond);
    //diagnostics_cons.push_back(mk_and(ctx,no_matches));
    r_no_matches.push_back( no_match );
  }
}

//todo : rewrite the following two functions to make them natural
void sugar_encoding::
no_fast_match_cons( const sugar_mol_ptr& m, VecExpr& no_matches ) {
  if( m->get_parent() == NULL ) return;
  auto m_comp = m->get_cons()->get_compartment();
  auto m_child_num = m->get_sibling_num();
  auto m_parent = m->get_parent();
  no_fast_match_cons( m_parent, m_child_num, m_comp, no_matches );
}

//
//     [m_parent] <-- point of operation
//       /    
//      /     
//    m=[NULL]  Sibling
//
//  No fast rule is available to create m
// z3::expr sugar_encoding::
// no_fast_match_cons_at_null_leaf( const sugar_mol_ptr& m  ) {
// void sugar_encoding::no_fast_can_extend_this(const sugar_mol_ptr& m, VecExpr& ans){  
//   std::vector<sugar_mol*> parent_trail;
//   std::vector< unsigned > sibling_trail;
//   parent_trail.push_back( m.get() );
//   // sibling_trail.push_back( 0 );
//   for(auto r: rs){
//     if( !r->check_is_empty() ) {
//   for( unsigned d = 1; d < r->get_depth(); d++ ) {
//     // find d deep match in past
//     auto m_g = parent_trail.back();
//     if( m_g->get_parent() ) {
//       parent_trail.push_back( m_g->get_parent() );
//       sibling_trail.push_back( m_g->get_sibling_num() );
//     }else{
//       break;
//     }    
    
//       VecExpr match;
//       match_condition(r,parent_trail,sibling_trail, m->get_cons()->get_tstamp(), match);
//       ans.push_back(r->get_is_fast() && mk_and(ctx,match));
//     }    
//   }
//   }
//   return;
// }
z3::expr sugar_encoding::no_match_cons( const sugar_mol_ptr& m  ) {
  VecExpr no_matches;
  if( m->get_parent() == NULL ) return mk_true(ctx);
  // return mk_false(ctx);
  z3::expr m_comp(ctx);
  //auto m_comp = m->get_cons()->get_compartment();
  auto m_child_num = m->get_sibling_num();
  auto m_parent = m->get_parent();
  //no_fast_can_extend_this(m,no_matches);
  no_fast_match_cons( m_parent, m_child_num, m_comp, no_matches );
  //return mk_and( ctx, no_matches );
  //return z3::operator!(mk_or(ctx,no_matches));
  return mk_false(ctx);
}
void sugar_encoding::
no_fast_can_extend_unknown(const sugar_mol_ptr& m, VecExpr& no_matches){
  //unknown && non-recursive
  if(m->get_parent()){
    //std::cout<<"yes, parent existed"<<std::endl;
    //z3::expr m_comp(ctx);
    //auto m_comp = m->get_cons()->get_compartment();
    VecExpr no_match;
    no_fast_match_cons(m->get_parent(),m->get_sibling_num(),ctx.int_val(max_compartments+1),no_match);
    no_matches.push_back(mk_and(ctx,no_match));
  }
  return;
}
void sugar_encoding::
no_fast_can_extend(const sugar_mol_ptr& m, VecExpr& no_matches){
  if(m==nullptr) {
    return;
  }
  //z3::expr m_comp(ctx);
  auto m_comp = m->get_cons()->get_compartment();
  for(unsigned int i=0;i<m->get_children_num();i++){
    if( m->get_child(i)==nullptr ) {
      //m->pp(std::cout);std::cout<<std::endl;
      VecExpr no_match;
      no_fast_match_cons(m.get(),i,m_comp,no_match);
      no_matches.push_back(mk_and(ctx,no_match));
    }else{
      no_fast_can_extend(m->get_child(i),no_matches);
    }
  }
  return;
}

// z3::expr sugar_encoding::
// no_fast_match_cons_at_all_null_leaves( const sugar_mol_ptr& root_m  ) {
//   // enum all deadend edges call the above function

// }

void sugar_encoding::
apply_rule_cons( sugar_mol_ptr& m, VecExpr& rule_matches ) {
  auto cons_ptr = m->get_cons();
  auto cut_bit = cons_ptr->get_cut_bit();
  auto m_comp = cons_ptr->get_compartment();
  auto mark = cons_ptr->get_tstamp();
  for( unsigned i=0; i < rs.size(); i++ ) {
    auto r = rs.at(i);
    // if no match already decided
    if( is_false( cons_ptr->get_rule_bit(i) ) ) continue;
    VecExpr depth_matches;
    VecExpr depth_only_cond_matches;
    std::vector<sugar_mol*> parent_trail;
    std::vector< unsigned > sibling_trail;
    parent_trail.push_back( m.get() );
    // sibling_trail.push_back( 0 );
    for( unsigned d = 1; d < r->get_depth(); d++ ) {
      // find d deep match in past
      auto m_g = parent_trail.back();
      if( m_g->get_parent() ) {
        parent_trail.push_back( m_g->get_parent() );
        sibling_trail.push_back( m_g->get_sibling_num() );
      }else{
        break;
      }

      VecExpr match;
      auto curr_r = match_condition(r,parent_trail,sibling_trail, mark, match);

      // arg1=false to simulate cut mismatch between rule and molecule
      match_cuts( mk_false(ctx), curr_r, m, match );
      match_tree( curr_r, m, mark, false, match, curr_r, false );

      depth_matches.push_back( mk_and( ctx, match ) );
    }
    auto match_cond = cut_bit && cons_ptr->get_rule_bit( i );
    z3::expr r_match = z3::implies( match_cond, mk_or( ctx, depth_matches ) );
    rule_matches.push_back( r_match );
  }
  if( false ) //ashu: options
    no_fast_match_cons( m, rule_matches );
}


//-------------------------------------------------------------------------


void sugar_encoding::
match_cons_rec( sugar_mol_ptr& m, VecExpr& cons ) {
  if(m) {
    apply_rule_cons( m, cons );
    for( unsigned i = 0; i < m->get_children_num(); i++ ) {
      auto m_i = m->get_child(i);
      match_cons_rec( m_i, cons );
    }
  }
}

void sugar_encoding::
encode_mol( sugar_mol_ptr& m, VecExpr& cons ) {
  match_cons_rec( m,  cons );
  VecExpr seeds_match;
  for( auto& seed: seed_rs ) {
    VecExpr seed_match;
    z3::expr dummy_mark(ctx);
    match_cuts( mk_false(ctx), seed, m, seed_match );
    match_tree( seed, m, dummy_mark, false, seed_match, seed, false );
    seeds_match.push_back( mk_and( ctx, seed_match ) );
  }
  cons.push_back( mk_or( ctx, seeds_match ) );
}

z3::expr sugar_encoding::encode_neg_cons( sugar_mol_ptr& mol ) {
  VecExpr cons;
  mol->get_cons()->collect_local_cons( cons );
  encode_mol( mol, cons );
  // Think : Does it interfere with longer mols if no repetition????
  // cons.add( no_fast_match_cons_at_all_null_leaves( m ) )
  // dump(cons);
  //return !mk_and( ctx, cons ); // \/ mol_is_extendable_by_fast_reactions
  no_fast_can_extend(mol,cons);
  // VecExpr cons1;
  // no_fast_can_extend(mol,cons1);
  // dump(cons1);
  // assert(false);
  return !mk_and( ctx, cons );
}

//-------------------------------------------------------------------------


sugar_t_ptr sugar_encoding::
read_sugar( const VecExpr& sugar_bits, z3::model& m ) {
  unsigned i = get_true_idx( sugar_bits, m );
  return i == sugar_bits.size() ? nullptr : sugars.at(i);
}

void
sugar_encoding::
construct_conc_rule( rule_ptr& r, rule_ptr& conc_r, z3::model& m,
                     VecExpr& conc_rules ) {
  if(!r ) return;
  assert(r);
  assert( conc_r );

  // assign learned values to concrete rules
  conc_r->set_is_extended( is_true( r->get_is_extended() , m ) );
  conc_r->set_is_condition( is_true( r->get_is_condition() , m ) );
  conc_r->set_compartment( get_int_val( r->get_compartment(), m ) );
  conc_r->set_is_fast( is_true( r->get_is_fast(), m ) );
  conc_r->set_is_hard_end( is_true( r->get_is_hard_end(), m ) );

  //dump(conc_r->get_is_fast());
  unsigned idx = get_true_idx( r->get_sugar_bits(), m );
  conc_r->set_sugar( idx );
  for( unsigned i = 0; i < r->get_children_num(); i++ ) {
    rule_ptr r_i = r->get_child(i);
    rule_ptr conc_r_i = conc_r->get_child(i);
    construct_conc_rule( r_i, conc_r_i, m, conc_rules);
  }

  // record learned rules in a constraint
  conc_rules.push_back( r->get_is_extended() == conc_r->get_is_extended() );
  conc_rules.push_back( r->get_is_condition() == conc_r->get_is_condition());
  conc_rules.push_back( r->get_compartment() == conc_r->get_compartment() );
  conc_rules.push_back( r->get_is_fast() == conc_r->get_is_fast() );
  conc_rules.push_back( r->get_is_hard_end() == conc_r->get_is_hard_end() );

  for( unsigned i = 0; i < r->get_sugar_bits().size(); i++  ) {
    if( i == idx )
      conc_rules.push_back( r->get_sugar_bit( i ) );
    else
      conc_rules.push_back( !r->get_sugar_bit( i ) );
  }
}

void
sugar_encoding::
construct_seed_rule( rule_ptr& conc_r, sugar_mol_ptr& m ) {
  if( conc_r == NULL ) return;
  assert(conc_r);
  conc_r->set_is_condition( false );
  conc_r->set_compartment( 0 );
  if( m == NULL ) {
    conc_r->set_is_extended( false );
  }else{
    conc_r->set_is_extended( true );
    conc_r->set_sugar( m->get_sugar_number() );
  }
  for( unsigned i = 0; i < conc_r->get_children_num(); i++ ) {
    rule_ptr conc_r_i = conc_r->get_child(i);
    sugar_mol_ptr m_i = nullptr;
    if( i < m->get_children_num() )
      m_i = m->get_child(i);
    construct_seed_rule( conc_r_i, m_i );
  }
}

z3::expr sugar_encoding::
read_synthesized_rules( z3::model& m ) {
  unsigned i = 0;
  VecExpr conc_cons;
  for( auto r : rs ) {
    //r->pp(std::cout);
    construct_conc_rule( r, rs_conc.at(i), m, conc_cons );
    i++;
  }
  if(verbose) {
    dump_dotty_rules(true);
    i=0;
    for( auto mol : pos_ms) {
      std::string fname = "/tmp/m"+ std::to_string(i++) + ".dot";
      mol->dump_dotty( fname, m );
    }
  }
  return mk_and( ctx, conc_cons);
}


void dump_neg_mol( sugar_mol_ptr& neg_mol, z3::model& m ) {
  static unsigned i = 0;
  std::string fname = "/tmp/neg-m"+ std::to_string(i++) + ".dot";
  neg_mol->dump_dotty( fname, m );
}

void sugar_encoding::dump_dotty_rules( bool pp ) {
  static unsigned round = 0;
  round++;
  unsigned i = 0;
  for( auto r : rs_conc ) {
    if( !r->check_is_empty() ) {
      if(pp) { rs_conc.at(i)->pp(std::cout); std::cout << "\n"; }
      std::string fname =
        "/tmp/r-"+ std::to_string(round) +"-"+std::to_string(i) + ".dot";
      rs_conc.at(i)->dump_dotty( fname );
    }
    i++;
  }
}

void sugar_encoding::print_result( std::ostream& o ) {
  o << "---------------------------\n";
  o << "Synthesized rules:\n";
  for( auto r : rs_conc ) {
    if( !r->check_is_empty() ) {
      r->pp( o );
      o << "\n";
    }
  }
  o << "# Also dumped in the following files:\n";
  dump_dotty_rules(false);
  if( true ) {
    o << "# Meanwhile discovered the following negative trees:\n";
    for( auto mol : neg_ms ) {
      mol->pp( o );
      o << "\n";
    }
  }
}

void sugar_encoding::print_fail_result( std::ostream& o ) {
  o << "Failed to synthesize rules!\n";
  o << "Discovered the following negative trees:\n";
  for( auto mol : neg_ms ) {
    mol->pp( o );
    o << "\n";
  }
}
//-------------------------------------------------------------------------

sugar_mol_ptr sugar_encoding::
read_neg_mol( sugar_mol_ptr& ukn_m, z3::model& m, expr_set& q_vars ) {
  if( !ukn_m ) return nullptr;
  assert( ukn_m );
  assert( !ukn_m->get_sugar() );
  auto sugar = read_sugar(ukn_m->get_unknown_sugar()->get_sugar_bits(),m);
  if( sugar == nullptr ) return nullptr;
  sugar_mol_vec children;
  for( unsigned i = 0; i < ukn_m->get_children_num(); i++ ) {
    auto m_i = ukn_m->get_child(i);
    auto child = read_neg_mol( m_i, m, q_vars );
    if( i < sugar->get_children_num() ) {
      children.push_back( child );
    }else{
      assert( child == nullptr );
    }
  }

  auto mol = sugar_mol::make( sugar, children);
  auto cut_cons = ukn_m->get_cons();
  unsigned m_idx = cut_cons->read_rule(m);
  bool is_cut = cut_cons->read_cut_bit(m);
  int tstamp = cut_cons->read_tstamp(m);
  int compartment = cut_cons->read_compartment(m);
  auto m_sols = mol_cons::make( ctx, mol, rs.size(), m_idx, is_cut,
                                tstamp, compartment );
  // to use quantified negative constraints or not!!
  if( is_quantified_neg_cons ) {
    mol->set_sol( m_sols );
    auto quantified = mol_cons::make( ctx, mol, rs.size() );
    quantified->collect_variables( q_vars );
    mol->set_cons( quantified );
  }else{
    mol->set_cons( m_sols );
  }
  return mol;
}

void sugar_encoding::add_diagnostic_cons( VecExpr& es ) {
  for( auto e : es ) {
    diagnostics_cons.push_back(e);
  }
}
void sugar_encoding::add_diagnostic_cons( z3::expr& e ) {
  diagnostics_cons.push_back(e);
}

void sugar_encoding::eval_diagnostic_cons( z3::model& m ) {
  // std::cout << m;
  for( auto d_cons : diagnostics_cons ) {
     dump( d_cons );
    expr_set d_vars;
    get_variables( d_cons, d_vars);
    dump( m.eval(d_cons));
    // std::cout << "{\n";
    // for( auto v : d_vars ) {
    //   std::cout<<"@@@@"<<std::endl;
    //   dump( v );
    //   dump( m.eval(v) );
    //   // std::cout<<"@@@@"<<std::endl;
    // }
    // std::cout << "}\n";
  }
}

// #define RULES_OPTIMIZE

void sugar_encoding::do_synth() {
  VecExpr rule_cons;
  VecExpr rule_bits;
  for( auto& r : rs ) r->collect_local_cons(rule_cons, rule_bits);
  z3::expr sorted = distinct_rules();
  rule_cons.push_back( sorted );
  // dump( sorted );

  VecExpr pos_cons;
  unsigned group_idx = 0;
  unsigned i = 0;
  VecExpr group_guards;
  for( auto mol : pos_ms ) {
    unsigned in_a_group = false;
    if( group_idx < group_mols.size() ) {
      auto& grp = group_mols[group_idx];
      in_a_group = ( i >= grp[0] && i <= grp.back() );
    }

    initialize_mol_cons( mol );
    if( in_a_group ) {
      VecExpr p_cons;
      mol->get_cons()->collect_local_cons( p_cons );
      encode_mol( mol, p_cons );
      auto g = get_fresh_bool(ctx);
      pos_cons.push_back( z3::implies( g, mk_and( ctx, p_cons ) ) );
      group_guards.push_back( g );
    }else{
      mol->get_cons()->collect_local_cons( pos_cons );
      encode_mol( mol, pos_cons );
    }

    //Detect and of the group and flush
    if( group_guards.size() > 0 &&  i == group_mols[group_idx].back() ) {
      pos_cons.push_back( atleast( ctx, group_guards, 1 ) );
      group_guards.clear();
      group_idx++;
    }

    i++;
  }
  // dump(pos_cons);
  z3::expr rules_and_pos = mk_and( ctx, rule_cons) && mk_and( ctx, pos_cons);
  // std::cout<<"######################"<<std::endl;
  // dump(rule_cons);
  // std::cout<<"######################"<<std::endl;
#ifdef RULES_OPTIMIZE
  // enable optimization
  z3::optimize find_rules(ctx);
  find_rules.add( rules_and_pos );
  for( z3::expr b : rule_bits) find_rules.add( b, 1 );
#else
  // disable optimization
  z3::solver find_rules(ctx);
  find_rules.add( rules_and_pos );
#endif

  VecExpr neg_cons;
  unknown_m->get_cons()->collect_local_cons( neg_cons );
  unknown_m->get_unknown_sugar()->collect_local_cons( neg_cons );
  neg_cons.push_back( mk_or( ctx, unknown_m_ls_exist ) );
  encode_mol( unknown_m, neg_cons );
  for( auto m : pos_ms )
    neg_cons.push_back( not_contain_mols( m, unknown_m ) );
    // neg_cons.push_back( distinct_mol_pairs( m, unknown_m ) );
  
  //dump(neg_cons);
  z3::solver find_neg_mol(ctx);
  for( auto n_con : neg_cons ) find_neg_mol.add( n_con );
  // find_neg_mol.add( mk_and( ctx, neg_cons ) );

  //find_neg_mol.add( no_fast_can_extend(unknown_m)  );

  unsigned round = 1;
  for(;;) {
    if(verbose) std::cout << "-------------------------------------\n";
    std::cout << "Round " << round++ << "\n";
    // if(verbose) std::cout <<"\n"; else std::cout <<"\r";
    // find rules
    z3::expr conc_rule_cons(ctx);
    if (find_rules.check() == z3::sat) {
      z3::model m = find_rules.get_model();
      // if( verbose ) {
      //     eval_diagnostic_cons( m );
      // }
      //std::cout << "HEY\n";
      //dump( m.eval(sorted) );
      conc_rule_cons = read_synthesized_rules( m );
      //print_result(std::cout);
    } else {
      print_fail_result( std::cout );
      return;
      // bio_synth_error( "sugar_synth", "failed to synthesize rules!" );
    }
    z3::expr guard = get_fresh_bool( ctx, "g" );
    find_neg_mol.add( z3::implies( guard, conc_rule_cons ) );
    //std::cout<<find_neg_mol<<std::endl;
    // find negative tree; add to neagtive constraints
    if ( check( find_neg_mol, guard ) == z3::sat) {
      z3::model m = find_neg_mol.get_model();
      //eval_diagnostic_cons( m );
      assert(false);
      expr_set q_vars;
      sugar_mol_ptr neg_mol = read_neg_mol( unknown_m, m, q_vars );
      if( verbose ) dump_neg_mol( neg_mol, m );
      neg_ms.push_back( neg_mol );
      z3::expr neg_c = mk_forall( q_vars, encode_neg_cons( neg_mol ) );
      // dump( neg_c.body() );
      // std::cout << is_sat( neg_c );
      find_rules.add( neg_c  );
    } else {
      break;
    }
  }
  print_result( std::cout );
}
