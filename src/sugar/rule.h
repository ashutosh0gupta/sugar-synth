#ifndef SUGAR_RULE_H
#define SUGAR_RULE_H

#include <solver/z3-util.h>
#include <sugar/sugar.h>

class rule;
typedef std::shared_ptr<rule> rule_ptr;
typedef std::vector<rule_ptr> rule_vec;

class rule {
public:
  rule( z3::context&, sugar_t_vec&, unsigned num_sugars,
        rule_vec& children_, bool is_conc );

  static void
  make( z3::context&, sugar_t_vec&, unsigned num_rules, unsigned num_sugars,
        unsigned depth, unsigned num_children, bool is_conc, rule_vec& rs );
  void initialize_non_recursive( unsigned max_compartments_ );

  unsigned get_children_num() const { return children.size(); }
  rule_ptr get_child( unsigned i ) const { return children[i]; }
  unsigned get_depth() const { return depth; }

  unsigned get_num_sugar_bits() const { return node_sugar.size(); }
  const VecExpr& get_sugar_bits() const { return node_sugar; }
  z3::expr get_sugar_bit( unsigned i ) const { return node_sugar[i]; }
  z3::expr get_is_extended() const { return is_extended; }
  z3::expr get_is_condition() const { return is_condition; }
  z3::expr get_occur_cons() const { return is_condition || is_extended; }
  z3::expr get_is_expand_cons() const { return is_extended && !is_condition; }
  z3::expr get_compartment() const { return compartment; }
  z3::expr get_is_fast() const {
    //return mk_true(ctx);
    return is_fast;
  }
  void collect_local_cons(VecExpr& , VecExpr& ) const;

  // for concerete rules
  bool check_is_empty();
  void set_sugar( unsigned idx);
  void set_is_extended( bool );
  void set_is_condition( bool );
  void set_compartment( unsigned num );
  void set_is_fast( bool );
  void add_no_empty_condition();

  z3::context& get_context() const { return ctx; }

  void dump_dotty( std::ostream& out ) const;
  void dump_dotty( std::string filename ) const;
  void pp( std::ostream& out ) const;

  static z3::expr sorted( const rule_ptr& r1, const rule_ptr& r2 );
private:
  z3::context& ctx;
  sugar_t_vec& sugars;
  rule_vec children;
  unsigned depth;

  // ------------------
  // Encoding variables
  VecExpr node_sugar;
  z3::expr is_extended; // bit endcodes that node will be extended
  z3::expr is_condition;// bit encodes that the node is monitored
                        // if both the above bits are false; node is donot care
  z3::expr is_hard_end; // true only if node does not exists i.e. (!is_extended and !is_condition)

  // rule specific property; unnecessarily copies are created for subfromulas
  z3::expr compartment;
  z3::expr is_fast; // to recode a rule is fast or slow wrt of exit rates

  // ------------------
  // Encoding constraints
  z3::expr local_constraints;

  static rule_ptr make( z3::context& ctx, sugar_t_vec&, unsigned num_sugars,
                        unsigned depth, unsigned num_children, bool is_conc );

  void initialize_non_recursive(z3::expr compartment_ , z3::expr is_fast_);

  unsigned dump_dotty( std::ostream&, unsigned p_c, unsigned c) const;
  void pp( std::ostream& out, bool parent_cond ) const;
  static z3::expr sorted( const rule_ptr& r1, const rule_ptr& r2,z3::expr next );
};

#endif
