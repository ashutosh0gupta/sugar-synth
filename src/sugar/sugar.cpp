#include "sugar/sugar.h"
#include "sugar/sugar-encode.h"
#include "solver/z3-util.h"
#include <fstream>

// #include <iostream>

sugar_t_ptr sugar_t::make( std::string symb_, unsigned num_children_,
                           unsigned idx_ ) {
  return std::make_shared<sugar_t>( symb_, num_children_, idx_ );
}

//-----------------------------------------------------------------------------
// sugar_mol utils

sugar_mol::sugar_mol( sugar_t_ptr& sugar_, sugar_mol_vec& children_ )
  : sugar(sugar_), children( children_ ), parent(nullptr), sibling_number(0) {
  if( sugar ) assert( sugar->get_children_num() == children.size() );
  set_depth();
}

sugar_mol::sugar_mol( sugar_t_ptr& sugar_, sugar_mol_ptr child )
  : sugar(sugar_), children(), parent(nullptr), sibling_number(0)
{
  if( sugar ) assert( sugar->get_children_num() == 1 );
  children.push_back( child );
  set_depth();
}

sugar_mol::sugar_mol( sugar_t_ptr& sugar_, sugar_mol_ptr child1, sugar_mol_ptr child2 )
  : sugar(sugar_), parent(nullptr), sibling_number(0)
  {
    if(sugar) assert( sugar->get_children_num() == 2 );
    children.push_back( child1 );
    children.push_back( child2 );
    set_depth();
  }

sugar_mol::sugar_mol( sugar_t_ptr& sugar_ )
    : sugar(sugar_)
  {
    if(sugar) {
      for( unsigned i = 0; i < sugar->get_children_num(); i++ ) {
        sugar_mol_ptr null_sugar;
        children.push_back( null_sugar );
      }
    }
    set_depth();
  }

// void sugar_mol::update_child( unsigned child_num, sugar_mol_ptr& new_child ) {
//   assert( sugar->get_num_children() > child_num );
//   children[child_num] == new_child;
// }

// void sugar_mol::add_child( unsigned child_num, sugar_mol_ptr& new_child ) {
//   if( new_child ) {
//     if( children[child_num] ) {
//       bio_synth_error( "sugar", "adding already existing child!" );
//     }else{
//       update_child( child_num, new_child );
//     }
//   }
// }

// void sugar_mol::add_children( sugar_mol_vec& new_children ) {
//   assert( sugar->get_num_children() == new_children.size() );
//   for( unsigned i = 0; i < new_children.size(); i++ ) {
//     if( new_children[i] ) {
//       add_child( i, new_children[i] );
//     }
//   }
// }

void sugar_mol::set_depth() {
  depth = 1;
  for( unsigned i = 0; i < children.size(); i++ ) {
    if( children[i] ) {
      unsigned new_depth = children[i]->get_depth() + 1;
      if( new_depth > depth ) depth = new_depth;
    }
  }
}

void sugar_mol::pp( std::ostream& out ) const {
  bool is_children = false;
  for( unsigned i = 0; i < children.size(); i++ ) {
    if( children[i] ) {
      is_children = true;
      break;
    }
  }
  std::string s = sugar ? sugar->get_name() : "__unknown__";
  if( is_children ) {
    out << "(" << s;
    for( unsigned i = 0; i < children.size(); i++ ) {
      out << " ";
      if( children[i] ) {
        children[i]->pp( out );
      }else{
        out << "_";
      }
    }
    out << ")";
  }else{
    out << s;
  }
}

unsigned sugar_mol::
dump_dotty( std::ostream& out, unsigned parent, unsigned count, z3::model& m) const {
  bool verbose = true;
  // bool verbose = false;
  unsigned me = count;
  std::string label;
  if( sugar )
    label = sugar->get_name();
  else
    label = "__unknown__";
  if( cons_ptr ) {
    label = cons_ptr->read_compartment_as_string(m) + ":" + label;
    if( verbose ) {
      label = to_string( cons_ptr->get_compartment() ) + "=" + label;
    }
  }
  out << std::to_string(me) << "[label=\"" << label << "\"]\n";
  if( parent ) {
    std::string color = "black";
    std::string edge_label = "";
    auto c_ptr = cons_ptr;
    if( sol_ptr )
      c_ptr = sol_ptr;
    if( c_ptr ) {
      if(m) {
        if( c_ptr->read_cut_bit( m ) )
          color = "red";
        edge_label = c_ptr->read_tstamp_as_string(m);
        if( verbose ) {
          edge_label = to_string( c_ptr->get_tstamp() ) + "=" + edge_label;
        }
        edge_label = std::to_string( c_ptr->read_rule(m) ) + ":" + edge_label;
    // if( verbose ) {
    //  edge_label = to_string( c_ptr->get_compartment() ) + "=" + edge_label;
    // }
      }
    }
    // if( is_extended ) color = "red";
    out << std::to_string(parent) << "-> " << std::to_string(me)
        <<  "["
        << "label=\""  << edge_label << "\","
        << "color=" << color  << ","
        // << "style=" << style
        << "]\n";
  }
  count++;
  for( unsigned i = 0; i < children.size(); i++ ) {
    if( children[i] ) count =  children[i]->dump_dotty( out, me, count, m);
  }
  return count;
}

void sugar_mol::
dump_dotty( std::ostream& out, z3::model& m ) const {
  out << "digraph prog {\n";
  dump_dotty( out, 0, 1, m );
  out << "}\n";
}

void sugar_mol::
dump_dotty( std::string filename, z3::model& m ) const {
  std::cout << "dumping molecule in: " << filename << "\n";
  std::ofstream o;
  o.open ( filename, std::ofstream::out );
  if( o.fail() ) {
    bio_synth_error( "sugar_dump", "failed to open " << filename << "!" );
  }
  dump_dotty( o, m );
}

sugar_mol_ptr sugar_mol::make( sugar_t_ptr& sugar ) {
  sugar_mol_ptr m = std::make_shared<sugar_mol>( sugar );
  return m;
}


sugar_mol_ptr sugar_mol::make( sugar_t_ptr& sugar, sugar_mol_vec& children_ ) {
  sugar_mol_ptr m = std::make_shared<sugar_mol>( sugar, children_ );
  for( unsigned i = 0; i < children_.size(); i++ ) {
    auto m_child = children_.at(i);
    if( m_child ) {
      m_child->set_parent( m, i );
    }
  }
  return m;
}

//-----------------------------------------------------------------------------
// file Format

// (sugar A 2)
// (sugar B 2)
// (sugar Ca 3)
// (sugar Das 4)
// (mol (A (A B ) (Ca _ C _)) )

#include <fstream>
#include <ctype.h>

unsigned new_line_count = 1;

#define AT_L " at line " << std::to_string( new_line_count ) << "!"

bool peek_space( std::istream& in ) {
  char c = in.peek();
  if( c == '\n' || c == '\t' || c == '\r' || c == ' ' ){
    if( c == '\n' ) new_line_count ++;
    return true;
  }
  if( c == '#' ) {
    do{
      in.get();
      c = in.peek();
      if( c == '\n') {
        new_line_count ++;
        return true;
      }
    } while( !in.eof() );
    return true;
  }
  return false;
}

bool peek_alpha_numeric_or_underscore( std::istream& in ) {
  char c  = in.peek();
  return std::isalnum( c ) || c == '_' || c == '-';
}

void consume_spaces( std::istream& in ) {
  while( peek_space( in ) ) {
    in.get();
  }
}

bool read_or_check_open_parentheses( std::istream& in ) {
  consume_spaces( in );
  char c = in.peek();
  if( c == '(' ) {in.get(); return true; }
  return false;
}

bool check_close_parentheses( std::istream& in ) {
  consume_spaces( in );
  char c = in.peek();
  if( c == ')' ) return true;
  return false;
}

bool read_close_parentheses( std::istream& in ) {
  consume_spaces( in );
  char c = in.get();
  if( c != ')' ) {
    bio_synth_error( "sugar_parse",
                     "syntax error, expecting closed parentheses" << AT_L );
  }
  return false;
}


std::string read_symbol( std::istream& in ) {
  consume_spaces( in );
  std::string word;
  while( !peek_space(in) && peek_alpha_numeric_or_underscore(in) && !in.eof() )
    {
      word = word.append( 1, in.get() );
    }
  return word;
}

unsigned read_unsigned( std::istream& in ) {
  consume_spaces( in );
  unsigned i;
  in >> i;
  return i;
}

std::string read_command( std::istream& in ) {
  consume_spaces( in );
  if( read_or_check_open_parentheses( in ) ) {
    return read_symbol( in );
  }else if( in.eof() ) {
    return "eof";
  }else{
    bio_synth_error( "sugar_parse", "syntax error" << AT_L );
  }
}
sugar_t_ptr read_sugar( std::istream& in, unsigned sugar_idx ) {
  std::string symb = read_symbol( in );
  unsigned num = read_unsigned( in );
  auto sugar = sugar_t::make( symb, num, sugar_idx );
  return sugar;
}

sugar_t_ptr read_sugar_ref( std::string s_name, sugar_t_map sugar_map ) {
  if( sugar_map.find(s_name) != sugar_map.end() ) {
    return sugar_map[s_name];
    // return sugar_mol::make( sugar_map[s_name] );
  }else{
    bio_synth_error( "sugar_parse",
                     "undeclared sugar " << s_name << " found" << AT_L );
  }
}

sugar_mol_ptr read_mol( std::istream& in, const sugar_t_map& sugar_map ) {
  std::string s_name;
  if( read_or_check_open_parentheses(in) ) {
    std::string s_name = read_symbol( in );
    auto sugar = read_sugar_ref( s_name, sugar_map );
    sugar_mol_vec mols;
    assert( sugar );
    for( unsigned i = 0; i < sugar->get_children_num(); i++ ) {
      auto mol_i = read_mol( in, sugar_map );
      mols.push_back( mol_i );
    }
    read_close_parentheses(in);
    return sugar_mol::make( sugar, mols );
  }else if( check_close_parentheses(in) ) {
    bio_synth_error( "sugar_parse", "unexpected close parentheses" << AT_L );
  }else{
    std::string s_name = read_symbol( in );
    if( s_name == "_" ) return nullptr;
    auto sugar = read_sugar_ref( s_name, sugar_map );
    return sugar_mol::make( sugar );
  }
}

void read_file( std::istream& in, sugar_t_map& sugar_map,
                sugar_mol_vec& seed_mols,
                sugar_mol_vec& mols,
                std::vector< std::vector<unsigned> > group_mols,
                unsigned& num_rules, unsigned& rule_depth,
                unsigned& max_compartments,
                bool& quantified_neg_cons ) {
  std::vector<unsigned> local_group;
  bool group_active = false;
  unsigned sugar_idx = 0;
  for(;;){
    std::string cmd = read_command( in );
    if( cmd == "eof" ) {
      return;
    }else if( cmd == "seed-mol" ) {
      seed_mols.push_back( read_mol( in, sugar_map ) );
    }else if( cmd == "mol" ) {
      if( group_active )
        local_group.push_back( mols.size() );
      auto m = read_mol( in, sugar_map );
      mols.push_back( m );
    }else if( cmd == "sugar" ) {
      auto sugar = read_sugar( in, sugar_idx++ );
      auto s_name = sugar->get_name();
      if( sugar_map.find(s_name) != sugar_map.end() ) {
        bio_synth_error( "sugar_parse", "sugar " << s_name
                         << " declared twice" << AT_L );
      }else{
        sugar_map[s_name] = sugar;
      }
    }else if( cmd == "start-group" ) {
      if( group_active == true )
        bio_synth_error( "sugar_parse", "nested start-group found");
      group_active = true;
    }else if( cmd == "end-group" ) {
      if( !local_group.empty() )
        group_mols.push_back( local_group );
      local_group.clear();
      group_active = false;
    }else if( cmd == "number-of-rules-to-learn" ){
      num_rules = read_unsigned( in );
    }else if( cmd == "max-rule-depth" ){
      rule_depth = read_unsigned( in );
    }else if( cmd == "max-compartments" ){
      max_compartments = read_unsigned( in );
    }else if( cmd == "quantified-neg-cons" ){
      quantified_neg_cons = (read_unsigned( in ) > 0);
    }else{
      bio_synth_error( "sugar_parse", "unknown command " << cmd << " !");
    }
    read_close_parentheses(in);
  }
  if( !local_group.empty() ) {
    group_mols.push_back( local_group );
  }
}

//-----------------------------------------------------------------------------
// synthesis code

void synth_sugar( std::string input ) {
  z3::context ctx;
  sugar_mol_vec seed_mols;
  sugar_mol_vec mols;
  std::vector< std::vector<unsigned> > group_mols;
  sugar_t_map sugar_map;
  unsigned num_rules = 0;
  unsigned rule_depth = 0;
  unsigned max_compartments = 1;
  bool quantified_neg_cons = true;

  std::ifstream in;
  in.open( input );
  if( in.fail() ) {
    bio_synth_error( "sugar_parse", "failed to open " << input << "!" );
    //File does not exist code here
  }
  read_file( in, sugar_map, seed_mols, mols, group_mols, num_rules, rule_depth,
             max_compartments, quantified_neg_cons );
  if( seed_mols.size() == 0 ) {
    bio_synth_error( "sugar_parse", "no seed mols declared!" );
  }
  sugar_t_vec sugars;
  for( unsigned i=0; i < sugar_map.size(); i++ ) sugars.push_back( nullptr );
  // sugars.reserve( sugar_map.size() );
  for( auto pair : sugar_map ) {
    sugar_t_ptr s  = pair.second;
    sugars[ s->get_idx() ] = s;
  }
  // for( auto m : mols ) {
  //   m->pp( std::cout );
  //   std::cout << "\n";
  // }
  sugar_encoding s( ctx, sugars, seed_mols, mols, group_mols, num_rules,
                    rule_depth, max_compartments,
                    // solving configurations
                    quantified_neg_cons
                    );

  s.do_synth();


}
