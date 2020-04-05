#define _NO_OMP_
#define _MP_INTERNAL
// #define _MP_GMP

#include "ast/expr_abstract.h"
#include "ast/ast_util.h"
#include "ast/rewriter/quant_hoist.h"
#include "ast/ast_pp.h"
#include "ast/rewriter/th_rewriter.h"
#include "ast/rewriter/expr_replacer.h"
#include "model/model_v2_pp.h"
#include "model/model_evaluator.h"
#include "smt/smt_kernel.h"
#include "smt/params/smt_params.h"
#include "smt/smt_solver.h"
#include "solver/solver.h"
#include "solver/mus.h"
#include "qe/qsat.h"
#include "qe/qe_mbp.h"
#include "qe/qe.h"
#include "ast/rewriter/label_rewriter.h"
#include "api/api_context.h"
#include "ast/rewriter/expr_safe_replace.h"

#include "z3-util.h"

//----------------------------------------------------------------------------
// Basic Z3 utils

ast_manager& get_ast_manager( z3::context& c ) {
  Z3_context z3_ctx = c;
  ast_manager& m = mk_c(z3_ctx)->m();
  return m;
}

ast_manager& get_ast_manager( z3::expr& f ) {
  auto& c = f.ctx();
  return get_ast_manager( c );
}

expr_ref get_z3_internal_expr_ref( z3::expr& f ) {
  auto& m = get_ast_manager( f );
  expr* f_expr = to_expr(f);
  expr_ref f_ref(m);
  expr_safe_replace subst(m);
  subst(f_expr, f_ref);
  return f_ref;
}

z3::expr get_z3_expr_from_internal_expr( z3::context& c, expr_ref& f ) {
  Z3_ast f_middle = of_ast(f);
  return z3::expr( c, f_middle );
}

z3::expr get_z3_expr_from_internal_expr( z3::context& c, ast* const f ) {
  Z3_ast f_middle = of_ast(f);
  return z3::expr( c, f_middle );
}

//z3::expr exp = to_ast(f_ref);
//----------------------------------------------------------------------------
// prenex normal form

// void filter_vars( qe::pred_abs& m_pred_abs, app_ref_vector const& vars ) {
//   for (unsigned i = 0; i < vars.size(); ++i) {
//     // auto decl = vars[i]->get_decl();
//     // m_pred_abs.fmc()->insert(decl);
//     //std::cout << "The filt vars are: " << vars << "\n" ;
//   }
// }

void hoist(ast_manager& m, expr_ref& fml, vector<app_ref_vector>& m_vars) {

  //qe::pred_abs m_pred_abs(m); //some function on this object must be called???

  // todo: why this?
  //proof_ref pr(m);
  //label_rewriter rw(m);
  //rw.remove_labels(fml, pr);

  quantifier_hoister hoist(m);
  app_ref_vector vars(m);
  bool is_forall = false;        
  //m_pred_abs.get_free_vars(fml, vars);
  m_vars.push_back(vars);
  //std::cout << "The current vars are: " << vars << "\n" ;  
  vars.reset();
  hoist.pull_quantifier(is_forall, fml, vars);
  //std::cout << "At the mid run the fml is : " << fml << "\n";
  m_vars.back().append(vars);
  //   filter_vars(m_pred_abs, vars);
  do {
    is_forall = !is_forall;
    vars.reset();
    hoist.pull_quantifier(is_forall, fml, vars);
    //std::cout << "At the mid run the fml is : " << fml << "\n";
    m_vars.push_back(vars);
    //filter_vars( m_pred_abs, vars );
  }
  while (!vars.empty());
  SASSERT(m_vars.back().empty()); 
  /* Print m_var content
  for (auto& x : m_vars ) { 
    std::cout << x << "\n";
    //std::cout << "Hi" << "\n";
  }*/
  // for( auto& m: m_vars ) {
  //   std::cout << m;
  //   //std::cout << "The sort of " << m << " is " << Z3_get_sort (c, m); 
  // // todo: do we need it??
  // //initialize_levels();
  // }
  // std::cout << "\n";
}

z3::expr prenex( z3::expr& f, VecsExpr& m_expr_vectors) {
  // std::cout << "The sort of the formula f is: " << Z3_get_sort_name( c, f);
  auto& c = f.ctx();
  auto& m = get_ast_manager(f);
  auto f_ref = get_z3_internal_expr_ref( f );
  vector<app_ref_vector> m_vars;
  hoist( m, f_ref, m_vars);

  bool first_row = true;
  for( auto& vec : m_vars ) {
    std::vector<z3::expr> e_vec;
   for( auto& v : vec ) {
     z3::expr expr_v = get_z3_expr_from_internal_expr( c, v );
     e_vec.push_back( expr_v );
   }
   if( first_row ) {
     m_expr_vectors.push_back( e_vec );
     first_row = false;
   }else{
     if( e_vec.size() > 0 )
       m_expr_vectors.push_back( e_vec );
   }
 }

  z3::expr f_ret = get_z3_expr_from_internal_expr( c, f_ref );
  return f_ret;
}

