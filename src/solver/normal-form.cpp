#include <vector>
#include "z3-util.h"

std::vector<z3::expr> cnf_converter( z3::expr& fml ) {
  z3::context& c = fml.ctx();
  z3::solver s(c);
  z3::goal g(c);
  g.add( fml );

   z3::tactic t1(c, "simplify");
   z3::tactic t2(c, "tseitin-cnf");
   z3::tactic t = t1 & t2;
   z3::apply_result r = t(g);
   z3::goal r_g = r[0];
   std::vector<z3::expr>  expr_list; 
   for (unsigned int i = 0; i < r_g.size(); i++) {
      expr_list.push_back( r_g[i]);
   }
  
   return expr_list;
}

