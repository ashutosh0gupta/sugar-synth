#include <sugar/sugar-encode.h>
#include <fstream>

//-------------------------------------------------------------------------
//  rule: object for holding to be synthesized rules
//-------------------------------------------------------------------------

rule::rule( z3::context& ctx_, sugar_t_vec& sugars_, unsigned num_sugars,
            rule_vec& children_, bool is_conc )
  : ctx(ctx_), sugars(sugars_), children( children_ ), depth(1),
    is_extended(ctx_), is_condition(ctx_), is_hard_end(ctx_), compartment(ctx_), is_fast(ctx_), 
    local_constraints(ctx_) {
  //set depth
  for( unsigned i = 0; i < children.size(); i++ ) {
    if( children[i] ) {
      unsigned new_depth = children[i]->get_depth() + 1;
      if( new_depth > depth ) depth = new_depth;
    }
  }
  if( is_conc ) {
    is_extended = mk_false(ctx);
    is_condition = mk_false(ctx);
    is_hard_end = mk_false(ctx);
    compartment = ctx.int_val( 0 );
    is_fast = ctx.bool_val(false);
    for(  unsigned i = 0; i < num_sugars; i++ ) {
      node_sugar.push_back( mk_false( ctx ) );
    }
    return;
  }else{
    is_extended = get_fresh_bool( ctx, "r_e" );
    is_condition = get_fresh_bool( ctx, "r_c" );
    is_hard_end = get_fresh_bool( ctx, "r_h" );
    node_sugar = get_fresh_bool_vec( ctx, num_sugars, "r_s" );
  }
  //make constraints
  VecExpr cs;
  //only one of sugar occurs
  cs.push_back( atmost( ctx, node_sugar, 1 ) );
  cs.push_back( z3::implies( get_occur_cons(), atleast( ctx, node_sugar, 1 )) );

  // number of children should match with sugar type
  for( unsigned i = 0; i < node_sugar.size(); i++ ) {
    unsigned bound = sugars.at(i)->get_children_num();
    VecExpr not_occurs;
    for( unsigned j = bound; j < children.size(); j++ ) {
      if( children[j] )
        not_occurs.push_back( !children[j]->get_occur_cons() );
    }
    cs.push_back( z3::implies( node_sugar.at(i), mk_and( ctx, not_occurs )));
  }

  // constraints for monomer additions.
  z3::expr is_expand_tree = is_extended && !is_condition;
  for( unsigned j = 0; j < children.size(); j++ ) {
    if( children[j] )
      cs.push_back(z3::implies(is_expand_tree,!children[j]->get_occur_cons()));
  }

  for( unsigned i = 0; i < children.size(); i++ ) {
    if( children[i] ) {
      //if children occur then node must occur
      cs.push_back( z3::implies( children[i]->get_occur_cons(),
                                 get_occur_cons() ) );
      // if extended node, all children must not be condition nodes
      cs.push_back( z3::implies( is_extended && !is_condition,
                                 !children[i]->get_is_condition() ) );
      // if condition node not on path, none extension node
      cs.push_back( z3::implies( !is_extended && is_condition,
                                 !children[i]->get_is_extended() ) );
    }
  }


  // if condition node on path, at most one extension node
  VecExpr extendeds;
  for( unsigned i = 0; i < get_children_num(); i++ ) {
    if( children[i] )
      extendeds.push_back( get_child(i)->get_is_extended() );
  }
  cs.push_back( z3::implies( is_extended && is_condition,
                             sum_is_k( ctx, extendeds, 1 ) ) );

  local_constraints = mk_and( ctx, cs );
}

void rule::initialize_non_recursive( unsigned max_compartments_ ) {
  z3::expr _compartment = get_fresh_int( ctx, "r_comp" );
  z3::expr is_fast_ = get_fresh_bool( ctx, "r_f" );
  
  initialize_non_recursive( _compartment, is_fast_ );
  if( max_compartments_ ) max_compartments_ = max_compartments_ - 1;
  local_constraints = local_constraints
    && 0 <= compartment && compartment <= (int)max_compartments_;

  if( false ) {
    // constraints for expand is immediate after root
    VecExpr expand_tree;
    for( unsigned j = 0; j < children.size(); j++ ) {
      if( children[j] )
        expand_tree.push_back( children[j]->get_is_expand_cons() );
    }
    local_constraints = local_constraints &&
    z3::implies( get_occur_cons(), mk_or( ctx, expand_tree ) );
  }
}

void rule::initialize_non_recursive(z3::expr compartment_, z3::expr is_fast_) {
  compartment = compartment_;
  is_fast = is_fast_;
  for( unsigned i = 0; i < get_children_num(); i++ ) {
    if( children[i] )
      get_child(i)->initialize_non_recursive( compartment_, is_fast_ );
  }
}

void rule::add_no_empty_condition() {
  local_constraints = local_constraints
    && ( !is_condition || is_extended );
}

rule_ptr
rule::make( z3::context& ctx, sugar_t_vec& sugars, unsigned num_sugars,
            unsigned depth, unsigned num_children, bool is_conc ) {
  if( depth > 0 ) {
    rule_vec r_vec;
    make( ctx, sugars, num_children, num_sugars, depth-1,
          num_children, is_conc, r_vec);
    return std::make_shared<rule>( ctx, sugars, num_sugars, r_vec, is_conc );
  }else{
    return nullptr;
  }
}

void
rule::make( z3::context& ctx, sugar_t_vec& sugars, unsigned num_rules,
            unsigned num_sugars, unsigned depth, unsigned num_children,
            bool is_conc, rule_vec& rs ) {
  for( unsigned i = 0; i < num_rules; i++ ) {
    auto r = make( ctx, sugars, num_sugars, depth, num_children, is_conc );
    rs.push_back( r );
  }
}

void
rule::collect_local_cons( VecExpr& l_cons, VecExpr& rule_bits ) const {
  for( unsigned i = 0; i < get_children_num(); i++ ) {
    if( children[i] ) {
      children[i]->collect_local_cons( l_cons, rule_bits );
    }
  }
  l_cons.push_back( local_constraints );
  rule_bits.push_back( !get_is_extended() );
  rule_bits.push_back( !get_is_condition() );
}

bool rule::check_is_empty() {
  bool is_ext = is_true( is_extended );
  bool is_cond = is_true( is_condition );
  return !is_ext && !is_cond;
}

void rule::set_sugar( unsigned idx) {
  for( unsigned i = 0; i < node_sugar.size(); i++ )
    node_sugar[i] = ctx.bool_val( i == idx );
}
void rule::set_is_extended ( bool v    ) { is_extended = ctx.bool_val( v ); }
void rule::set_is_condition( bool v    ) { is_condition = ctx.bool_val( v ); }
void rule::set_compartment ( unsigned c) {compartment = ctx.int_val( c ); }
void rule::set_is_fast     ( bool v    ) { is_fast = ctx.bool_val( v ); }
void rule::set_is_hard_end     ( bool v    ) { is_hard_end = ctx.bool_val( v ); }

z3::expr sort_bit( z3::expr b1, z3::expr b2, z3::expr next ) {
  return (!b1 && b2) || ( b1 == b2 && next );
}

z3::expr sort_int( z3::expr b1, z3::expr b2, z3::expr next ) {
  return (b1 < b2) || ( b1 == b2 && next );
}

z3::expr rule::sorted( const rule_ptr& r1, const rule_ptr& r2, z3::expr next ){
  assert( r1 );
  assert( r2 );
  assert( r1->get_depth() == r2->get_depth() );
  z3::expr orig_next = next;
  if( r1->get_depth() > 1 ) {
    for( unsigned i = 0; i < r1->get_children_num(); i++ ) {
      next = sorted( r1->get_child(i), r2->get_child(i), next );
    }
  }
  for(  unsigned i = 0; i < r1->get_num_sugar_bits(); i++ ) {
    next = sort_bit( r1->get_sugar_bit(i) , r2->get_sugar_bit(i) , next );
  }
  auto o_cons = r1->get_occur_cons();
  next = (o_cons && next ) || ( !o_cons && orig_next );
  next = sort_bit( r1->get_is_extended() , r2->get_is_extended() , next );
  next = sort_bit( r1->get_is_condition(), r2->get_is_condition(), next );
  // next = sort_int( r1->get_phase()       , r2->get_phase()       , next );
  return next;
  // auto& ctx = r1->get_context();
  // VecExpr gt;
  // VecExpr eq;
  // for( unsigned i = 0; i < r1->get_num_sugar_bits(); i++ ) {
  //   VecExpr conj;
  //   for( unsigned j = i; j < r1->get_num_sugar_bits(); j++ ) {
  //     conj.push_back( !r1->get_sugar_bit(j) );
  //   }
  //   auto c = z3::implies( r2->get_sugar_bit(i), mk_and( ctx, conj ) );
  //   gt.push_back( c );
  //   eq.push_back( r1->get_sugar_bit(i) == r2->get_sugar_bit(i) );
  // }
  // return mk_and( ctx, gt ) || z3::implies( mk_and( ctx, eq ), next );
}

z3::expr rule::sorted( const rule_ptr& r1, const rule_ptr& r2 ) {
  z3::expr null_rules = ( !r1->get_occur_cons() && !r2->get_occur_cons() );
  z3::expr sub_sort = null_rules || sorted(r1, r2,mk_false(r1->get_context()));
  return sort_int(r1->get_compartment(), r2->get_compartment(), sub_sort );
  // return sorted( r1, r2, mk_false( r1->get_context() ) );
}



unsigned rule::
dump_dotty( std::ostream& out, unsigned parent, unsigned count ) const {
  unsigned me = count;
  bool is_ext = is_true( is_extended );
  bool is_cond = is_true( is_condition );
  // if( !is_ext && !is_cond) return count;
  std::string label = "_";
  sugar_t_ptr sugar = nullptr;
  if( is_ext || is_cond ) {
    for(unsigned i = 0; i < node_sugar.size(); i++ ) {
      if( is_true( node_sugar.at(i) ) ) {
        sugar = sugars.at(i);
        label = sugar->get_name();
        break;
      }
    }
  }

  out << std::to_string(me) << "[label=\"" << label << "\"]\n";
  if( parent ) {
    std::string color = "black";
    if(is_ext) color = is_cond ? "red" : "blue";
    out << std::to_string(parent) << "-> " << std::to_string(me)
        <<  "["
        // << "label="  << label << ","
        << "color=" << color  << ","
        // << "style=" << style
        << "]\n";
  }
  count++;
  if( !is_ext && !is_cond) return count;
  for( unsigned i = 0; i < sugar->get_children_num(); i++ ) {
  // for( unsigned i = 0; i < children.size(); i++ ) {
    if( children[i] ) count =  children[i]->dump_dotty( out, me, count );
  }
  return count;
}

void rule::
dump_dotty( std::ostream& out ) const {
  out << "digraph prog {\n";
  dump_dotty( out, 0, 1 );
  out << "}\n";
}

void rule::
dump_dotty( std::string filename ) const {
  std::cout << "dumping rule in: " << filename << "\n";
  std::ofstream o;
  o.open ( filename, std::ofstream::out );
  if( o.fail() ) {
    bio_synth_error( "rule_dump", "failed to open " << filename << "!" );
  }
  dump_dotty( o );
}

void rule::pp( std::ostream& out ) const {
  std::cout << get_compartment() ;
  if( is_true( is_fast ) ){
    std::cout<< ":";
    // if( is_true( is_hard_end )){
    //   std::cout<< ": hard ";
    // }
    // else{
    //   std::cout<< ": soft";
    // }
  }
  else{
    std::cout<< "::";
    // if( is_true( is_hard_end )){
    //   std::cout<< ":: hard ";
    // }
    // else{
    //   std::cout<< ":: soft";
    // }
  } 
  pp( out, true );
}

void rule::pp( std::ostream& out, bool parent_cond ) const {
  bool is_children = false;
  for( unsigned i = 0; i < children.size(); i++ ) {
    if( children[i] ) {
      is_children = true;
      break;
    }
  }

  bool is_ext = is_true( is_extended );
  bool is_cond = is_true( is_condition );
  // if( !is_ext && !is_cond) return count;
  std::string label = "_";
  sugar_t_ptr sugar = nullptr;
  if( is_ext || is_cond ) {
    for(unsigned i = 0; i < node_sugar.size(); i++ ) {
      if( is_true( node_sugar.at(i) ) ) {
        sugar = sugars.at(i);
        label = sugar->get_name();
        break;
      }
    }
  }
  if( sugar ) {
    if( parent_cond && !is_cond ) out << "[";
    if( is_children ) out << "(";
    out << sugar->get_name();
    if( is_children ) {
      for( unsigned i = 0; i < sugar->get_children_num(); i++ ) {
        out << " ";
        if( children[i] ) {
          children[i]->pp( out, is_cond );
        }else{
          out << "_";
        }
      }
    }
    if( is_children ) out << ")";
    if( parent_cond && !is_cond ) out << "]";
    // out << ")";
  }else{
   if( is_true( is_hard_end )){
     out << "<";
   }else{
     out << "_";
   }
  }

}
