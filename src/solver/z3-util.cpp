#include "z3-util.h"

std::string to_string(z3::sort e) {
  std::stringstream ss; ss << e; return ss.str();
}

std::string to_string(z3::expr e) {
  std::stringstream ss; ss << e; return ss.str();
}

std::string to_string( VecExpr es) {
  std::stringstream ss;
  for( auto& e : es ) ss << e << ",";
  return ss.str();
}

void print_exprs( std::ostream& o, VecExpr& es) {
  for( auto& e : es ) o << e << "\n";
}

void print_expr_vec( std::ostream& o, z3::expr_vector& es) {
  for( unsigned i = 0; i < es.size(); i++ ) o << es[i] << "\n";
}

void print_expr_set( std::ostream& o, expr_set& es) {
  for( auto& e : es ) o << e << "\n";
}

void print_expr( std::ostream& o, z3::expr e) {
    o << e << "\n";
}

void dump( z3::expr e )         { print_expr( std::cerr, e );      }
void dump( VecExpr& es )         { print_exprs( std::cerr, es );    }
void dump( expr_set& es )        { print_expr_set( std::cerr, es ); }
void dump( z3::expr_vector& es ) { print_expr_vec( std::cerr, es ); }

z3::expr parseFormula( z3::context& c, std::string str,
                       const std::vector< std::string >& names ) {
  unsigned s = names.size();
  Z3_symbol* symbols = new Z3_symbol[s];
  Z3_func_decl* decls = new Z3_func_decl[s];
  std::vector<z3::expr> vars; // for keeping smart pointer alive
  for (unsigned i=0; i < s; i++) {
    const char* name = names[i].c_str();
    z3::expr v = c.bool_const( name );
    vars.push_back( v );
    decls[i] = v.decl();
    symbols[i] = Z3_mk_string_symbol(c, name);
  }
  str = sanitise_string(str);
  std::string cmd = str.find_first_of(' ')!=std::string::npos
    ? "(assert (" + str + "))" : "(assert " + str + ")";
  z3::expr ast(c);
  try {
    Z3_ast_vector e = Z3_parse_smtlib2_string(c, cmd.c_str(), 0, NULL, NULL, s, symbols, decls);
    delete[] symbols;
    delete[] decls;
    ast = to_expr(c, Z3_ast_vector_get(c,e,0));
  }
  catch( z3::exception e ) {
    std::cerr << "Error parsing line \"" << str << "\"." << std::endl;
    throw;
  }
  return ast;
}

// must execute atomically in concurrent settings
unsigned name_counter = 0;
std::string get_fresh_name_count() {
  unsigned c = name_counter;
  name_counter++;
  return std::to_string(c);
}

z3::expr get_fresh_bool( z3::context& c, std::string suff ) {
  std::string s = "b_" + get_fresh_name_count() + "_" + suff;
  return c.bool_const( s.c_str());
}

VecExpr
get_fresh_bool_vec( z3::context& c, unsigned length , std::string suff ) {
  VecExpr vec;
  for( unsigned i = 0; i < length; i++ ) {
    vec.push_back( get_fresh_bool( c, suff ) );
  }
  return vec;
}

z3::expr get_fresh_int( z3::context& c, std::string suff ) {
  std::string s = "i_" + get_fresh_name_count() + "_" + suff;
  return c.int_const( s.c_str() );
}

// creates fresh FOL constants of any sort
z3::expr get_fresh_const( z3::context& c, z3::sort sort, std::string suff ) {
  std::string s = "c_" + get_fresh_name_count() + "_" + suff;
  return c.constant( s.c_str(), sort );
}


void get_variables(const z3::expr& expr, expr_set& result) {
  switch(expr.kind()) {
    case Z3_APP_AST:
      {
        z3::func_decl d = expr.decl();
      Z3_decl_kind dk = d.decl_kind();
      switch(dk) {
        case Z3_OP_UNINTERPRETED :
          {
            result.insert(expr);
          }
          break;
        default:
          {
          unsigned args = expr.num_args();
          for (unsigned i = 0; i<args; i++) {
            z3::expr arg = expr.arg(i);
            get_variables(arg, result);
          }
          }
          break;
      }
      }
      break;
    case Z3_NUMERAL_AST:
      break;
    default: 
      throw "unsupported";
      break;
  }
}
