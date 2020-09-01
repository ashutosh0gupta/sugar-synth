#ifndef SUGAR_ENCODE_H
#define SUGAR_ENCODE_H

#include <solver/z3-util.h>
#include <sugar/sugar.h>
#include <sugar/rule.h>


class mol_cons {
public:
  mol_cons( z3::context& ctx_, sugar_mol_ptr m_, unsigned rule_size );
  mol_cons( z3::context& ctx_, sugar_mol_ptr m_, unsigned rule_size,
            unsigned matched_rule, bool is_cut, int time, int comp );
  static mol_cons_ptr
  make( z3::context& ctx_, sugar_mol_ptr m_, unsigned rule_size );
  static mol_cons_ptr
  make( z3::context& ctx_, sugar_mol_ptr m_, unsigned rule_size,
        unsigned matched_rule, bool is_cut, int time, int comp );

  inline const VecExpr& get_rule_bits() const { return rule_match; }
  inline z3::expr get_rule_bit( unsigned i ) const { return rule_match.at(i); }
  inline z3::expr get_cut_bit() const { return cut_bit; }
  inline z3::expr get_tstamp() const { return tstamp; }
  inline z3::expr get_compartment() const { return compartment; }
  inline z3::expr get_local_cons() const { return local_constraints; }
  void collect_variables( expr_set& );
  void collect_local_cons(VecExpr& ) const;

  unsigned read_rule( z3::model& ) const;
  bool read_cut_bit( z3::model& ) const;
  int read_tstamp( z3::model& ) const;
  int read_compartment( z3::model& ) const;
  std::string read_tstamp_as_string( z3::model& ) const;
  std::string read_compartment_as_string( z3::model& ) const;

private:
  z3::context& ctx;
  sugar_mol* m;
  VecExpr rule_match;
  z3::expr cut_bit; // if the molecule at the cutpoint
  z3::expr tstamp;  // time stamp for the time for addition
  z3::expr compartment; // in the compartment the node was added

  // collect conditions on the local_cons
  z3::expr local_constraints;
};

class unknown_label {
public:
  unknown_label( z3::context& ctx_, sugar_mol_ptr m_, unsigned sugar_size );
  static unknown_label_ptr
  make( z3::context& ctx_, sugar_mol_ptr m_, unsigned sugar_size );

  inline unsigned get_num_sugar_bits() const { return node_sugar.size(); }
  inline z3::expr get_sugar_bit( unsigned i ) const { return node_sugar.at(i);}
  const VecExpr& get_sugar_bits() const { return node_sugar;}
  z3::expr get_exists_cons();
  void collect_local_cons(VecExpr& ) const;

  z3::context& get_context() const { return ctx; }
  // ~unknown_label() { m = nullptr; }
private:
  z3::context& ctx;
  sugar_mol* m;
  VecExpr node_sugar;
  // collect conditions on the local_cons
  z3::expr local_constraints;
};

class sugar_encoding {
public:
  sugar_encoding( z3::context& ctx, sugar_t_vec& sugars,
                  sugar_mol_vec& seed_ms_, sugar_mol_vec& ms_,
                  std::vector< std::vector<unsigned> > group_mols_,
                  unsigned num_rules, unsigned rule_depth,
                  unsigned max_compartments_ );

  // constraints making calls
  z3::expr distinct_rules();
  // z3::expr sorted_rules( const rule_ptr& r1, const rule_ptr& r2 );

  void encode_mol( sugar_mol_ptr& mol, VecExpr& cons );
  z3::expr encode_neg_cons( sugar_mol_ptr& m );

  z3::expr read_synthesized_rules( z3::model& m );
  void dump_dotty_rules(bool pp);
  void print_result( std::ostream& o );
  void print_fail_result( std::ostream& o );

  void do_synth();
private:
  z3::context& ctx;
  rule_vec rs;
  rule_vec rs_conc;
  sugar_t_vec sugars;
  rule_vec seed_rs;
  sugar_mol_vec pos_ms; // positive samples --
  std::vector< std::vector<unsigned> > group_mols;
  sugar_mol_vec neg_ms; //negative samples -- originally none
  unsigned rule_depth;
  unsigned rule_max_children;
  unsigned max_compartments;
  sugar_mol_ptr unknown_m;
  VecExpr unknown_m_ls_exist;
  unsigned m_depth_max = 0;

  // a number to immidate a large number
  // all other numbers may be asked to be smaller than this
  z3::expr large_number = ctx.int_val( 10000 );

  bool verbose = true;
  VecExpr diagnostics_cons;

  // constraint utils
  z3::expr same_sugar( const VecExpr&, const VecExpr& );

  // internal constraint makking variables
  void initialize_mol_cons( sugar_mol_ptr& m );

  // void
  // collect_match_vars( z3::expr r_parent_occurs, rule_ptr r, sugar_mol_ptr m,
  //                     z3::expr mark_stamp, bool is_ub, VecExpr& sbits );

  void match_tree( rule_ptr r, sugar_mol_ptr m, z3::expr mark, bool is_ub,
                   VecExpr&);
  void match_cuts( z3::expr r_parent_occurs, rule_ptr r, sugar_mol_ptr m,
                    VecExpr& sbits );
  rule_ptr match_condition( rule_ptr curr_r,
                            const std::vector<sugar_mol*>& parent_trail,
                            const std::vector< unsigned >& sibling_trail,
                            z3::expr mark, //z3::expr r_comp,
                            VecExpr& match );

  void apply_rule_cons( sugar_mol_ptr& m, VecExpr& rule_matches );
  void match_cons_rec( sugar_mol_ptr& m, VecExpr& cons );


  // z3::expr sorted_rules( const rule_ptr& r1, const rule_ptr& r2, z3::expr );
  void distinct_mols( const sugar_mol_ptr&, const sugar_mol_ptr&, VecExpr&);
  z3::expr distinct_mol_pairs( const sugar_mol_ptr&, const sugar_mol_ptr&);

  void not_contain_mols( const sugar_mol_ptr& m1, const sugar_mol_ptr& m2,
                         VecExpr& dists, VecExpr& strict, VecExpr& blocked );
  z3::expr not_contain_mols( const sugar_mol_ptr& m1, const sugar_mol_ptr& m2);
  //z3::expr no_repeat_in_branch( const sugar_mol_ptr& m);
  z3::expr no_repeat_in_branch( const sugar_mol_ptr& m1, const  sugar_mol_ptr& m2 );
  z3::expr match( const sugar_mol_ptr& root1, const sugar_mol_ptr& root2,const sugar_mol_ptr& stop);
  z3::expr exact_match(const sugar_mol_ptr& m1, const sugar_mol_ptr& mt);
  z3::expr compare(const sugar_mol_ptr& m1, const sugar_mol_ptr& m2, int d, int r);
  void no_fast_match_cons( sugar_mol* m_parent, unsigned m_child_num,
                           z3::expr m_comp, VecExpr& r_no_matches );
  void no_fast_match_cons( const sugar_mol_ptr& m, VecExpr& no_matches );
  //z3::expr no_fast_match_cons_at_null_leaf( const sugar_mol_ptr& m  );

  //void no_fast_can_extend_this(const sugar_mol_ptr& m, VecExpr& no_matches);
  z3::expr no_match_cons( const sugar_mol_ptr& m  );
  void no_fast_can_extend(const sugar_mol_ptr& m,VecExpr& no_matches);
  
  // reading solutions
  sugar_mol_ptr read_neg_mol( sugar_mol_ptr&, z3::model&, expr_set& );
  sugar_t_ptr read_sugar( const VecExpr& sugar_bits, z3::model& m );
  void construct_conc_rule( rule_ptr&, rule_ptr&, z3::model& m, VecExpr& );
  void construct_seed_rule( rule_ptr& conc_r, sugar_mol_ptr& m );

  void make_unknown_mol(unsigned depth, unsigned num_children, sugar_mol_vec&);
  sugar_mol_ptr make_unknown_mol( unsigned depth, unsigned num_children );

  // Diagnostics functions
  void eval_diagnostic_cons( z3::model& m );
  
};

#endif
