#ifndef SUGAR_H
#define SUGAR_H

#include "utils/utils.h"
#include "solver/z3-util.h"
#include <map>

class sugar_t;
typedef std::shared_ptr<const sugar_t> sugar_t_ptr;
typedef std::map<std::string,sugar_t_ptr> sugar_t_map;
typedef std::vector<sugar_t_ptr> sugar_t_vec;

class sugar_t{
public:
  sugar_t( std::string symb_, unsigned num_children_, unsigned idx_ )
    : symbol(symb_), num_children( num_children_ ), idx(idx_) {}
  inline std::string get_name() const { return symbol; }
  inline unsigned get_children_num() const { return num_children; }
    inline unsigned get_idx() const { return idx; }
  static sugar_t_ptr make( std::string symb_, unsigned num_cs_, unsigned idx );
private:
  std::string symbol;
  unsigned num_children;
  unsigned idx;
};

class sugar_mol;
typedef std::shared_ptr<sugar_mol> sugar_mol_ptr;
typedef std::vector<sugar_mol_ptr> sugar_mol_vec;
//
class mol_cons;
typedef std::shared_ptr<mol_cons> mol_cons_ptr;
//
class unknown_label;
typedef std::shared_ptr<unknown_label> unknown_label_ptr;

class sugar_mol{
public:
  sugar_mol( sugar_t_ptr& sugar_, sugar_mol_vec& children_ );
  sugar_mol( sugar_t_ptr& sugar_, sugar_mol_ptr child );
  sugar_mol( sugar_t_ptr& sugar_, sugar_mol_ptr child1, sugar_mol_ptr child2 );
  sugar_mol( sugar_t_ptr& sugar_ );
  // void add_children( sugar_mol_vec& children );
  // void add_child( unsigned child_num, sugar_mol_ptr& new_child );
  // void update_child( unsigned child_num, sugar_mol_ptr& new_child );
  void pp( std::ostream& out ) const;
  void dump_dotty( std::ostream& out, z3::model& m ) const;
  void dump_dotty( std::string filename, z3::model& m ) const;
  void dump_dotty( std::string filename ) const;

  inline unsigned     get_depth() const { return depth; }
  inline sugar_mol_ptr get_child( unsigned i ) const { return children.at(i); }
  inline unsigned get_children_num() const { return children.size();}
  inline unsigned get_sugar_number() const { return sugar->get_idx(); }
  inline sugar_t_ptr get_sugar() const { return sugar; }
  inline sugar_mol* get_parent() const { return parent; }
  inline unsigned get_sibling_num() const { return sibling_number; }

  inline mol_cons_ptr get_cons() { return cons_ptr; }
  inline mol_cons_ptr get_sol()  { return sol_ptr;  }
  inline unknown_label_ptr get_unknown_sugar() { return unknown_label; }

  inline void set_cons( mol_cons_ptr cons_ptr_ ) { cons_ptr = cons_ptr_; }
  inline void set_sol ( mol_cons_ptr sol_ptr_  ) { sol_ptr = sol_ptr_;   }
  inline void set_unknown_label( unknown_label_ptr p ) { unknown_label = p; }
  inline void set_parent( sugar_mol_ptr p, unsigned parent_child_num_ ) {
    parent = p.get();
    sibling_number = parent_child_num_;
  }
  // inline void set_is_extended( bool is_extended_) {
  //   is_extended = is_extended_;
  // }

  static sugar_mol_ptr make( sugar_t_ptr& sugar_ );
  static sugar_mol_ptr make( sugar_t_ptr& sugar_,  sugar_mol_vec& children_ );

private:
  const sugar_t_ptr sugar;
  sugar_mol_vec children;

  sugar_mol* parent;
  unsigned sibling_number;

  unsigned depth;

  // for constraints
  mol_cons_ptr cons_ptr = nullptr;
  // for solution of cons
  mol_cons_ptr sol_ptr = nullptr;
  // for unknown cons
  unknown_label_ptr unknown_label = nullptr;
  // // for learned rules
  // bool is_extended = false;
  void  set_depth();
  unsigned dump_dotty( std::ostream&, unsigned p_c, unsigned c, z3::model& ) const;
};

// main function for sugar synthesis
void synth_sugar( std::string input );

#endif
