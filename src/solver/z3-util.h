#ifndef VTS_Z3_UTIL_H
#define VTS_Z3_UTIL_H

#include <vector>
#include <unordered_set>
#include <z3++.h>
// #include <boost/regex.hpp>
#include <boost/algorithm/string.hpp>

//#define toDigit(c) (c-'0')


template <class T>
inline
std::vector<T> to_vector( const std::vector<std::vector<T>>& double_vec ) {
  std::vector<T> output;
  for(typename std::vector<std::vector<T>>::size_type i=0; i < double_vec.size(); ++i) {
    output.insert(output.end(), double_vec[i].begin(), double_vec[i].end());
  }
  return output;
}

struct expr_hash {
  size_t operator () (const z3::expr& a) const {
    Z3_ast ap = a;
    size_t hash = std::hash<Z3_ast>()(ap);
    return hash;
  }
};

struct expr_equal :
  std::binary_function <z3::expr,z3::expr,bool> {
  bool operator() (const z3::expr& x, const z3::expr& y) const {
    return z3::eq( x, y );
    }
};

typedef std::unordered_set<z3::expr,  expr_hash, expr_equal> expr_set;
typedef std::vector< std::vector<z3::expr> > VecsExpr;

typedef std::vector< z3::expr > VecExpr;
typedef std::vector< VecExpr > Vec2Expr;
typedef std::vector< Vec2Expr > Vec3Expr;
typedef std::vector< Vec3Expr > Vec4Expr;

std::string to_string( z3::sort );
std::string to_string( z3::expr e);
std::string to_string( VecExpr es);
void print_expr_set( std::ostream& o, expr_set& es);
void print_exprs( std::ostream& o, VecExpr& es);
void print_expr_vec( std::ostream& o, z3::expr_vector& es);
void print_expr( std::ostream& o, z3::expr e);
void dump( z3::expr e );
void dump( expr_set& e );
void dump( VecExpr& e );
void dump( z3::expr_vector e );


// Create typedef for tuples
typedef std::vector< std::tuple<unsigned,unsigned> > Tup2Expr;
typedef std::vector< std::tuple<unsigned,unsigned,unsigned> > Tup3Expr;
typedef std::vector< std::tuple<unsigned,unsigned,unsigned,unsigned> > Tup4Expr;
typedef std::vector< std::tuple<unsigned,unsigned,unsigned,int> > Tup4SignedExpr;
typedef std::vector< std::tuple<unsigned,unsigned,unsigned,unsigned,unsigned> > Tup5Expr;



// function returns prenex body
// and quantified variables in list of list qs.
// The first entry in qs is for exists
z3::expr prenex(z3::expr& fml, VecsExpr& qs );

// Simplify the given z3 formula and convert it into nnf
//void negform ( z3::context& c, z3::expr& fml );

// Simplify the given z3 formula and convert it into cnf

//std::vector<z3::expr_vector> cnf_converter( z3::expr& fml );
std::vector<z3::expr> cnf_converter( z3::expr& fml );

//void qdimacs_printer( z3::expr& fml, std::vector<z3::expr_vector>& m_var );

void qdimacs_printer(std::vector<z3::expr>& cnf_fml, VecsExpr& m_var );

// Add function for the depqbf file creator.
void depqbf_file_creator(VecExpr& edgeQuant, unsigned int equant_len);

// Add function for DepQbf run
int depqbf_run_with_timeout();

VecExpr  get_fresh_bool_vec( z3::context&, unsigned, std::string = "");
z3::expr get_fresh_bool( z3::context&, std::string = "");
z3::expr get_fresh_int( z3::context&, std::string = "");
z3::expr get_fresh_const( z3::context&, z3::sort , std::string = "");

inline z3::expr make_bool(z3::context& c, std::string& name) {
  return c.bool_const( name.c_str() );
}

inline z3::expr mk_true( z3::context& c ) {
  return c.bool_val( true );
}

inline z3::expr mk_false( z3::context& c ) {
  return c.bool_val( false );
}

inline z3::expr mk_and( z3::context& c, const VecExpr& es ) {
  z3::expr_vector args(c);
  for( auto e : es )
    args.push_back( e );
  return z3::mk_and( args );
}

inline z3::expr mk_forall( const expr_set& es, z3::expr f ) {
  if( es.empty() ) return f;
  z3::context& c = f.ctx();
  z3::expr_vector vs(c);
  for( auto e : es )
    vs.push_back( e );
  return z3::forall( vs, f );
}

inline z3::expr mk_or( z3::context& c, const VecExpr& es ) {
  z3::expr_vector args(c);
  for( auto e : es )
    args.push_back( e );
  return z3::mk_or( args );
}

inline bool is_true( const z3::expr e ) {
  return eq( e, e.ctx().bool_val(true) );
}

inline bool is_false( const z3::expr e ) {
  return eq( e, e.ctx().bool_val(false) );
}

inline bool is_true( z3::expr e,  z3::model m ) {
  z3::expr v = m.eval( e );
  return ( Z3_get_bool_value( v.ctx(), v)  == Z3_L_TRUE );
}

inline bool is_false( z3::expr e,  z3::model m ) {
  z3::expr v = m.eval( e );
  return ( Z3_get_bool_value( v.ctx(), v)  == Z3_L_FALSE );
}

inline int get_int_val( z3::expr e,  z3::model m ) {
  z3::expr num = m.eval( e );
  std::string val = to_string(num);
  if( val[0] == '(' ) {
    val = val.substr( 3, val.length() - 4);
    val = "-"+val;
    // std::cout << val << "\n";
  }
  // int result = 0;
  // if( !e.is_numeral_i( result ) )
  //   result = 0;
  // std::cout << val << " "<< result<< "\n";
  return std::stoi(val);
  // return result;
}

inline std::string  get_int_val_as_string( z3::expr e,
                                           z3::model m ) {
  z3::expr num = m.eval( e );
  return to_string( num );
}

inline std::string sanitise_string(std::string str) {
  boost::trim(str);
  while (str[0]=='(' && str[str.length()-1]==')') {
    str[0] = ' ';
    str[str.length()-1] = ' ';
    boost::trim(str);
  }
  return str;
}

inline z3::expr _xor( z3::expr const & a, z3::expr const & b ) {
  check_context(a, b);
  Z3_ast r = Z3_mk_xor(a.ctx(), a, b);
  return z3::expr(a.ctx(), r);
}

inline z3::expr
atmost( z3::context& c, const std::vector<z3::expr>& es, unsigned k ) {
  if( es.size() < k ) return mk_true( c );
  z3::expr_vector args(c);
  for (unsigned i = 0; i < es.size(); i++)
    args.push_back( es[i] );
  return z3::atmost( args, k );
}

inline z3::expr
atleast( z3::context& c, const std::vector<z3::expr>& es, unsigned k ) {
  z3::expr_vector args(c);
  for (unsigned i = 0; i < es.size(); i++)
    args.push_back( es[i] );
  return z3::atleast( args, k );
}

inline z3::expr
sum_is_k( z3::context& c, const std::vector<z3::expr>& bits, unsigned k ) {
  if( bits.size() < k )
    return mk_false( c );
  else
    return atleast( c, bits, k ) && atmost( c, bits , k );
}

z3::expr mk_le_k_bits( const z3::expr_vector& bits, unsigned k );
z3::expr mk_le_k_bits( const std::vector<z3::expr>& bits, unsigned k );

z3::expr parseFormula( z3::context& c, std::string str,
                       const std::vector< std::string >& names );

inline z3::check_result check( z3::solver& s, z3::expr e ) {
  z3::expr_vector assumptions( e.ctx() );
  assumptions.push_back( e );
  // for (unsigned i = 0; i < es.size(); i++)
  //   assumptions.push_back( es[i] );
  return s.check( assumptions );
}

inline bool is_sat( z3::expr e ) {
  z3::solver s( e.ctx() );
  s.add( e );
  if (s.check() == z3::sat) {
    z3::model m = s.get_model();
    std::cout << m;
    return true;
  }
  return false;
}
// Use Boost for delimiter

void get_variables(const z3::expr& expr, expr_set& result);

#endif
