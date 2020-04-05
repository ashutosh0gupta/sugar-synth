#include <vts.h>
#include <z3-util.h>
#include <algorithm>
#include <tuple>  // std::tuple
#include <vector>

template <int index>
struct TupleCompare {
  template <typename Tuple>
  bool operator()(const Tuple& left, const Tuple& right) const {
    return std::get<index>(left) < std::get<index>(right);
  }
};
// define n dimentional vectors
//#include "boost/multi_array.hpp"
//#include <cassert>

/*
// Functor to compare by the Mth element
template<int M, template<typename> class F = std::less>
struct TupleCompare
{
    template<typename T>
    bool operator()(T const &t1, T const &t2)
    {
        return F<typename tuple_element<M, T>::type>()(std::get<M>(t1),
std::get<M>(t2));
    }
};
*/

void vts::add_known_edge(unsigned n1, unsigned n2, unsigned q,
                         std::vector<unsigned>& mols,
                         std::vector<unsigned>& act) {
  // std::vector < std::tuple <unsigned, unsigned> > knownEdgesTuple;
  // std::vector < std::tuple <unsigned, unsigned, unsigned, unsigned> >
  // knownPresenceEdgesTuple; std::vector < std::tuple <unsigned, unsigned,
  // unsigned, unsigned> > knownActiveEdgesTuple;
  assert(n1 < N);
  assert(n2 < N);
  assert(q < E_arity);

  knownEdges.push_back(edges[n1][n2][q]);
  for (unsigned i = 0; i < mols.size(); i++) {
    assert(mols[i] < M);
    knownPresenceEdges.push_back(presence_edge[n1][n2][q][mols[i]]);
    if (act[i] == 0) {
      knownActiveEdges.push_back(!active_edge[n1][n2][q][mols[i]]);
    } else if (act[i] == 1) {
      knownActiveEdges.push_back(active_edge[n1][n2][q][mols[i]]);
    }
  }

  /*
  // Tuple version
  for ( unsigned i = 0; i < mols.size(); i++ ) {
     knownPresenceEdgesTuple.push_back( std::make_tuple (n1, n2, q, mols[i]) );
     knownEdgesTuple.push_back( std::make_tuple( n1, n2, q ) );
     if ( act[i] == 2 ) {
       continue;
     } else if( act[i] == 0 ) {
       knownActiveEdgesTuple.push_back( std::make_tuple( n1, n2, q, -mols[i] )
  ); } else { knownActiveEdgesTuple.push_back( std::make_tuple( n1, n2, q,
  mols[i] ) );
     }


  std::sort(begin(knownPresenceEdgesTuple), end(knownPresenceEdgesTuple),
  TupleCompare<0>()); std::sort(begin(knownActiveEdgesTuple),
  end(knownActiveEdgesTuple), TupleCompare<0>());
  std::sort(begin(knownEdgesTuple), end(knownEdgesTuple), TupleCompare<0>());
  }
  */
}

void vts::add_known_node(unsigned n, std::vector<unsigned>& mols,
                         std::vector<unsigned>& act) {
  for (unsigned i = 0; i < mols.size(); i++) {
    knownNodes.push_back(nodes[n][mols[i]]);
    // std::cout << "Added: " << nodes[n][mols[i]] << '\n';
    if (act[i] == 0) {
      knownActiveNodes.push_back(!active_node[n][mols[i]]);
    } else if (act[i] == 1) {
      knownActiveNodes.push_back(active_node[n][mols[i]]);
    }
  }

  /*
  // Tuple Version
  std::vector < std::tuple <unsigned, unsigned> > knownNodesTuple;
  for ( unsigned i = 0; i < mols.size(); i++ ) {
     knownNodesTuple.push_back ( std::make_tuple( n, i ) );
  }
 std::sort(begin(knownNodesTuple), end(knownNodesTuple), TupleCompare<0>());
  * */
}

void vts::add_known_pairing(unsigned m1, unsigned m2) {
  knownPairingMatrix.push_back(pairing_m[m1][m2]);
  /*
  std::vector < std::tuple <unsigned, unsigned> > knownPairingTuple;
  knownPairingTuple.push_back ( std::make_tuple( m1, m2 ) );
  std::sort(begin(knownPairingTuple), end(knownPairingTuple),
  TupleCompare<0>());
  */
}

void vts::qr_add_known_pairing(unsigned m1, unsigned m2, unsigned m3,
                               unsigned m4) {
  knownPairingMatrix.push_back(pairing_m_4d[m1][m2][m3][m4]);
  /*
  std::vector < std::tuple <unsigned, unsigned> > knownPairingTuple;
  knownPairingTuple.push_back ( std::make_tuple( m1, m2 ) );
  std::sort(begin(knownPairingTuple), end(knownPairingTuple),
  TupleCompare<0>());
  */
}
void vts::add_known_activity_node_function(unsigned m, z3::expr f) {}

void vts::add_known_activity_edge_function(unsigned m, z3::expr f) {}

//============================================================================
// Populates the vectors with z3 variables

void vts::popl1(VecExpr& m, unsigned arg1, std::string prefix) {
  for (unsigned int i = 0; i < arg1; i++) {
    std::string name = prefix + "_" + std::to_string(i);
    m.push_back(make_bool(ctx, name));
  }
}

void vts::popl2(Vec2Expr& m, unsigned arg1, unsigned arg2, std::string prefix) {
  m.resize(arg1);
  for (unsigned int i = 0; i < arg1; i++)
    popl1(m[i], arg2, prefix + "_" + std::to_string(i));
}

void vts::popl3(Vec3Expr& m, unsigned arg1, unsigned arg2, unsigned arg3,
                std::string prefix) {
  m.resize(arg1);
  for (unsigned int i = 0; i < arg1; i++)
    popl2(m[i], arg2, arg3, prefix + "_" + std::to_string(i));
}

void vts::popl4(Vec4Expr& m, unsigned arg1, unsigned arg2, unsigned arg3,
                unsigned arg4, std::string prefix) {
  m.resize(arg1);
  for (unsigned int i = 0; i < arg1; i++)
    popl3(m[i], arg2, arg3, arg4, prefix + "_" + std::to_string(i));
}

void vts::make_func(std::vector<z3::func_decl>& fs, std::string prefix) {
  // It should be an array.
  z3::sort_vector sorts(ctx);
  // std::vector<z3::sort> sorts;
  for (unsigned int m = 0; m < M; m++) {
    sorts.push_back(ctx.bool_sort());  // not sure
  }
  for (unsigned int m = 0; m < M; m++) {
    std::string name = prefix + "_" + std::to_string(m);
    z3::func_decl f = ctx.function(name.c_str(), sorts, ctx.bool_sort());
    fs.push_back(f);
  }
}

void vts::init_vts() {
  // Populate edges: e(i,j,q)
  popl3(edges, N, N, E_arity, "z");

  // Populate nodes: n(i,j)
  popl2(nodes, N, M, "n");

  // Populate active_node (i, k)
  popl2(active_node, N, M, "a");

  // Populate presence_edge(i,j,q,k)
  popl4(presence_edge, N, N, E_arity, M, "e");

  // Populate active_edge(i,j,q,k)
  popl4(active_edge, N, N, E_arity, M, "b");

  // Populate pairing_m(k,k1)
  popl2(pairing_m, M, M, "p");
  popl4(pairing_m_4d, M, M, M, M, "p");  // todo: more efficent implementation
                                         /*
                                         // Populate xtra var s_var : var for node function
                                           //popl2 ( s_var, 2 * (M-1), J, "s" );
                                           popl2 ( s_var, 2 * M, J, "s" );
                                       
                                         // Populate xtra var t_var : var for node function
                                           //popl2 ( t_var, 2 * (M-1), J, "t" );
                                           popl2 ( t_var, 2 * M, J, "t" );
                                         */
  // Populate node_funcs : Currently not handled.
  make_func(node_funcs, "yn");

  // Populate edge_funcs : Currently not handled.
  make_func(edge_funcs, "yn");

  // Populate reach(i,j,k,z)
  popl4(reach, N, N, M, N - 1, "r");

  // Populate drop1(i,j,q)
  popl3(drop1, N, N, E_arity, "d1");

  // Populate d_reach1(i,j)
  // popl2 ( d_reach1, N , N, "r1" );
  popl3(d_reach1, N, N, N, "r1");

  // Populate drop2(i,j,q)
  popl3(drop2, N, N, E_arity, "d2");

  // Populate d_reach(i,j)
  popl2(d_reach2, N, N, "r2");
}

z3::expr vts::is_mol_edge_present(unsigned i, unsigned j, unsigned m) {
  z3::expr_vector p_list(ctx);
  for (unsigned q = 0; q < E_arity; q++) {
    p_list.push_back(presence_edge[i][j][q][m]);
  }
  return z3::mk_or(p_list);
}

z3::expr vts::is_qth_edge_present(unsigned i, unsigned j, unsigned q) {
  z3::expr_vector p_list(ctx);
  for (unsigned m = 0; m < M; m++) {
    p_list.push_back(presence_edge[i][j][q][m]);
  }
  return z3::mk_or(p_list);
}

/* Activity Contraints ------ */

// Regulation : No regulation on the node.
// The present molecules at nodes are all active.
z3::expr vts::always_active_on_node() {  // f_nn
  z3::expr_vector ls(ctx);
  z3::expr lhs(ctx);
  for (unsigned i = 0; i < N; i++) {
    for (unsigned m = 0; m < M; m++) {
      lhs = (nodes[i][m] == active_node[i][m]);
      ls.push_back(lhs);
    }
  }
  return z3::mk_and(ls);
}

// Regulation : No regulation on the edge.
// The present molecules on the edge are all active.
z3::expr vts::always_active_on_edge() {  // f_ne
  z3::expr_vector ls(ctx);
  z3::expr lhs(ctx);
  for (unsigned i = 0; i < N; i++) {
    for (unsigned j = 0; j < N; j++) {
      if (j == i) continue;
      for (unsigned q = 0; q < E_arity; q++) {
        for (unsigned m = 0; m < M; m++) {
          lhs = (presence_edge[i][j][q][m] == active_edge[i][j][q][m]);
          ls.push_back(lhs);
        }
      }
    }
  }
  return z3::mk_and(ls);
}

// Regulation : SNARE-SNARE Inhibition.
z3::expr vts::pm_dependent_activity_on_edge() {  // f_se
  z3::expr_vector ls(ctx);
  z3::expr lhs(ctx);
  z3::expr l1(ctx);
  z3::expr l2(ctx);
  z3::expr x1(ctx);
  z3::expr x2(ctx);

  for (unsigned i = 0; i < N; i++) {
    for (unsigned j = 0; j < N; j++) {
      if (j == i) continue;
      for (unsigned q = 0; q < E_arity; q++) {
        for (unsigned m = 0; m < M; m++) {
          lhs = presence_edge[i][j][q][m];
          l1 = ctx.bool_val(false);
          l2 = ctx.bool_val(true);
          for (unsigned m1 = 0; m1 < M; m1++) {
            if (j == i) continue;
            l1 = (pairing_m[m][m1] || l1);
            l2 = (implies(pairing_m[m][m1], presence_edge[i][j][q][m1]) && l2);
          }
          x1 = implies((lhs && l1 && l2), !active_edge[i][j][q][m]);
          x2 = implies(lhs && !(l1 && l2), active_edge[i][j][q][m]);
          ls.push_back(x1 && x2);
        }
      }
    }
  }
  return z3::mk_and(ls);
}

z3::expr vts::func_driven_activity_on_node() {  // f_bn
  z3::expr_vector ls(ctx);
  z3::expr_vector s(ctx);
  z3::expr lhs(ctx);
  // todo : type f, f_app;
  // todo : type f, f_app;
  for (unsigned m = 0; m < M; m++) {
    auto f = node_funcs[m];
    for (unsigned i = 0; i < N; i++) {
      for (unsigned m1 = 0; m1 < M; m1++) {
        if (m1 == i) continue;
        s.push_back(nodes[i][m1]);
      }
      auto f_app = f(s);
      lhs = implies(nodes[i][m], active_node[i][m] == f_app);
      ls.push_back(lhs);
    }
  }
  return z3::mk_and(ls);
}

// f_be: BOolean function on edge.
z3::expr vts::func_driven_activity_on_edge() {  // f_be
  z3::expr_vector ls(ctx);
  z3::expr_vector s(ctx);
  z3::expr lhs(ctx);
  // // todo : type f, f_app;
  for (unsigned i = 0; i < N; i++) {
    for (unsigned j = 0; j < N; j++) {
      if (j == i) continue;
      for (unsigned q = 0; q < E_arity; q++) {
        for (unsigned m = 0; m < M; m++) {
          auto f = edge_funcs[m];
          for (unsigned m1 = 0; m1 < M; m1++) {
            if (j == i) continue;
            s.push_back(presence_edge[i][j][q][m1]);
          }
          lhs = implies(presence_edge[i][j][q][m],
                        active_edge[i][j][q][m] == f(s));
          ls.push_back(lhs);
        }
      }
    }
  }
  return z3::mk_and(ls);
}

// todo: the following functions need correct implementation

z3::expr vts::node_activity_constraint() {
  switch (V) {
    case MODEL_1:
      return always_active_on_node();
      break;
    // case MODEL_2: return func_driven_activity_on_node(); break;
    case MODEL_2:
      return ctx.bool_val(true);
      break;
    case MODEL_3:
      return always_active_on_node();
      break;
    // case MODEL_4: return func_driven_activity_on_node(); break;
    case MODEL_4:
      return ctx.bool_val(true);
      break;
    case MODEL_5:
      return always_active_on_node();
      break;
    // case MODEL_6: return func_driven_activity_on_node(); break;
    case MODEL_6:
      return ctx.bool_val(true);
      break;
    default:
      return ctx.bool_val(true);
  }
}

z3::expr vts::edge_activity_constraint() {
  switch (V) {
    case MODEL_1:
      return always_active_on_edge();
      break;
    case MODEL_2:
      return always_active_on_edge();
      break;
    // case MODEL_3: return func_driven_activity_on_edge(); break;
    case MODEL_3:
      return ctx.bool_val(true);
      break;
    // case MODEL_4: return func_driven_activity_on_edge(); break;
    case MODEL_4:
      return ctx.bool_val(true);
      break;
    case MODEL_5:
      return pm_dependent_activity_on_edge();
      break;
    case MODEL_6:
      return pm_dependent_activity_on_edge();
      break;
    default:
      return ctx.bool_val(true);
  }
}

/* Basic Constraints ... V1-V8.*/
// V1: For an edge to exist it should have one molecule present.
z3::expr vts::molecule_presence_require_for_present_edge() {
  z3::expr_vector ls(ctx);
  for (unsigned i = 0; i < N; i++) {
    for (unsigned j = 0; j < N; j++) {
      if (j == i) continue;
      for (unsigned q = 0; q < E_arity; q++) {
        ls.push_back(implies(is_qth_edge_present(i, j, q), edges[i][j][q]));
        // z3::expr rhs = ctx.bool_val(false);
        // for ( unsigned m = 0; m < M; m++ ) {
        //   rhs = presence_edge[i][j][q][m] || rhs;
        // }
        // ls.push_back( implies (rhs, edges[i][j][q]) );
      }
    }
  }
  return z3::mk_and(ls);
}

// V2: If molecule is active on an edge then it should be present on the edge.
z3::expr vts::active_molecule_is_present_on_edge() {  // V2
  z3::expr_vector ls(ctx);
  for (unsigned i = 0; i < N; i++) {
    for (unsigned j = 0; j < N; j++) {
      if (j == i) continue;
      for (unsigned q = 0; q < E_arity; q++) {
        for (unsigned m = 0; m < M; m++) {
          z3::expr e =
              implies(active_edge[i][j][q][m], presence_edge[i][j][q][m]);
          ls.push_back(e);
        }
      }
    }
  }
  return z3::mk_and(ls);
}

// V3: A mmolecule should be present to be active.
z3::expr vts::active_molecule_is_present_on_node() {  // V3
  z3::expr_vector ls(ctx);
  for (unsigned i = 0; i < N; i++) {
    for (unsigned m = 0; m < M; m++) {
      z3::expr e = z3::implies(active_node[i][m], nodes[i][m]);
      ls.push_back(e);
    }
  }
  return z3::mk_and(ls);
}

// V4: The edge label are subset of the source and target.
z3::expr vts::edge_label_subset_of_node_label() {  // V4
  z3::expr_vector ls(ctx);
  for (unsigned i = 0; i < N; i++) {
    for (unsigned j = 0; j < N; j++) {
      if (j == i) continue;
      for (unsigned q = 0; q < E_arity; q++) {
        for (unsigned m = 0; m < M; m++) {
          z3::expr e = z3::implies(presence_edge[i][j][q][m],
                                   nodes[i][m] && nodes[j][m]);
          ls.push_back(e);
        }
      }
    }
  }
  return z3::mk_and(ls);
}

// V5 : that no self edges are allowed.
z3::expr vts::no_self_edges() {  // V5
  z3::expr_vector ls(ctx);
  for (unsigned i = 0; i < N; i++) {
    for (unsigned q = 0; q < E_arity; q++) {
      ls.push_back(!edges[i][i][q]);
    }
  }
  return z3::mk_and(ls);
}

// V6: Only Q R entry has possible non zero entry.
// pairing restrictions: Only self dependency is disallowed.
z3::expr vts::qr_restriction_on_pairing_matrix() {  // V6
  z3::expr_vector ls(ctx);
  z3::expr e(ctx);
  // for( unsigned x = 0 ; x < M; x++ ) {
  //    ls.push_back( !pairing_m[x][x] );
  // }
  for (unsigned x = 0; x < M; x++) {
    for (unsigned y = 0; y < M; y++) {
      if (((x < 3 * M / 4) && (y < 3 * M / 4)) ||
          ((x >= 3 * M / 4) && (y >= 3 * M / 4))) {
        e = !pairing_m[x][y];
        ls.push_back(e);
      }
    }
  }
  return z3::mk_and(ls);
}

/*
unsigned qSnareCount = 23;
//unsigned rSnareCount = M - 7;
z3::expr vts::restriction_on_pairing_matrix() {              //V6
  z3::expr_vector ls(ctx);
  z3::expr e(ctx);
  for( unsigned x = 0 ; x < M; x++ ) {
    for( unsigned y = 0 ; y < M; y++ ) {
      if ( ((x < qSnareCount) && (y < qSnareCount)) || ((x >= qSnareCount) && (y
>= qSnareCount)) ) { e = !pairing_m[x][y]; ls.push_back( e );
      }
    }
  }
  return z3::mk_and( ls );
}
*/

z3::expr vts::restriction_on_pairing_matrix() {  // V6
  z3::expr_vector ls(ctx);
  z3::expr e(ctx);
  for (unsigned x = 0; x < M; x++) {
    for (unsigned y = 0; y < M; y++) {
      if (((x < M / 2) && (y < M / 2)) || ((x >= M / 2) && (y >= M / 2))) {
        e = !pairing_m[x][y];
        ls.push_back(e);
      }
    }
  }
  return z3::mk_and(ls);
}

// hack: FIXING NUMBER OF QSNARES, QSANRES always starts from 0.
unsigned qSnareCount = 16;

void vts::create_formula(z3::expr_vector& qSnareFml, z3::expr_vector& rSnareFml,
                         z3::expr_vector& molOnEdge,
                         z3::expr_vector& molOnTargetNode, unsigned i,
                         unsigned j, unsigned q, unsigned m, bool flag) {
  for (unsigned m1 = 0; m1 < M; m1++) {
    if (m == m1) continue;
    auto ae = active_edge[i][j][q][m1] && pairing_m[m][m1];
    auto an = active_node[j][m1] && pairing_m[m][m1];
    molOnEdge.push_back(ae);
    molOnTargetNode.push_back(an);
    // todo: if both are present the molecule becomes inactive.
    auto fml = (ae || an) && (!ae || !an);
    if (flag == true) {
      qSnareFml.push_back(fml);
    } else {
      rSnareFml.push_back(fml);
    }
  }
}

///---------------------------------------------------------------------------
// Ashutosh version
z3::expr vts::is_fuse(VecExpr& edge_vec, VecExpr& node_vec,
                      std::vector<unsigned>& fuse_mols) {
  z3::expr_vector node_active(ctx);
  z3::expr_vector edge_active(ctx);
  z3::expr_vector some_where_active(ctx);
  for (unsigned l = 0; l < 4; l++) {
    unsigned m = fuse_mols[l];
    auto either_place = edge_vec[m] || node_vec[m];
    some_where_active.push_back(either_place);
    edge_active.push_back(edge_vec[m]);
    node_active.push_back(node_vec[m]);
  }
  some_where_active.push_back(mk_or(edge_active));
  some_where_active.push_back(mk_or(node_active));
  return mk_and(some_where_active) &&
         pairing_m_4d[fuse_mols[0]][fuse_mols[1]][fuse_mols[2]][fuse_mols[3]];
}

z3::expr vts::qr_4d_edge_must_fuse_with_target() {  // V7
  z3::expr_vector ls(ctx);
  std::vector<unsigned> fuse_mols;
  fuse_mols.resize(4);
  for (unsigned i = 0; i < N; i++) {
    for (unsigned j = 0; j < N; j++) {
      if (j == i) continue;
      VecExpr& an = active_node[j];
      for (unsigned q = 0; q < E_arity; q++) {
        VecExpr& ae = active_edge[i][j][q];
        z3::expr_vector candidateMoleculeFormula(ctx);
        // lhs = ctx.bool_val(false);
        for (unsigned m1 = 0; m1 < qSnareCount; m1++) {
          fuse_mols[0] = m1;
          for (unsigned m2 = m1 + 1; m2 < qSnareCount; m2++) {
            fuse_mols[1] = m2;
            for (unsigned m3 = m2 + 1; m3 < qSnareCount; m3++) {
              fuse_mols[2] = m3;
              for (unsigned m4 = qSnareCount; m4 < M; m4++) {
                fuse_mols[3] = m4;
                // if( m3 > qSnareCount || m4 < qSnareCount ) continue; // QR
                // boundry
                z3::expr is_f = is_fuse(ae, an, fuse_mols);
                candidateMoleculeFormula.push_back(is_f);
              }
            }
          }
        }
        auto cons = mk_or(candidateMoleculeFormula);
        z3::expr e = implies(edges[i][j][q], cons);
        ls.push_back(e);
      }
    }
  }
  return z3::mk_and(ls);
}

z3::expr vts::qr_4d_edge_fuse_only_with_target() {  // V8
  z3::expr_vector ls(ctx);
  std::vector<unsigned> fuse_mols;
  fuse_mols.resize(4);

  for (unsigned i = 0; i < N; i++) {
    for (unsigned j = 0; j < N; j++) {
      if (j == i) continue;
      for (unsigned q = 0; q < E_arity; q++) {
        VecExpr& ae = active_edge[i][j][q];
        z3::expr_vector rhs(ctx);
        for (unsigned jp = 0; jp < N; jp++) {
          if (jp == j) continue;
          VecExpr& an = active_node[jp];
          // For each possible active candidate molecule
          z3::expr_vector candidateMoleculeFormula(ctx);
          for (unsigned m1 = 0; m1 < qSnareCount; m1++) {
            fuse_mols[0] = m1;
            for (unsigned m2 = m1 + 1; m2 < qSnareCount; m2++) {
              fuse_mols[1] = m2;
              for (unsigned m3 = m2 + 1; m3 < qSnareCount; m3++) {
                fuse_mols[2] = m3;
                for (unsigned m4 = qSnareCount; m4 < M; m4++) {
                  fuse_mols[3] = m4;
                  // if( m3 > qSnareCount || m4 < qSnareCount ) continue; // QR
                  // boundry
                  z3::expr is_f = is_fuse(ae, an, fuse_mols);
                  rhs.push_back(!is_f);
                }
              }
            }
          }
        }
        auto cons = mk_and(rhs);
        z3::expr e = implies(edges[i][j][q], cons);
        ls.push_back(e);
      }
    }
  }
  return z3::mk_and(ls);
}

z3::expr vts::vts_qr_4d_fusion_constraint() {
  z3::expr v7 = qr_4d_edge_must_fuse_with_target();  // V7
  z3::expr v8 = qr_4d_edge_fuse_only_with_target();  // V8
  auto cons = v7 && v8;
  return cons;
}

//-----------------------------------------------------------

void vts::fusion_constraint(z3::expr_vector& candidateMoleculeFormula,
                            unsigned i, unsigned j, unsigned q, unsigned m,
                            bool f1Orf2) {
  // Used to make sure QCount = 3 and RCount = 1
  z3::expr_vector qSnareFml(ctx);
  z3::expr_vector rSnareFml(ctx);
  z3::expr sideCons(ctx);
  // For constraint \/b_ijqm1 and \/a_jm1
  z3::expr_vector molOnEdge(ctx);
  z3::expr_vector molOnTargetNode(ctx);
  // For subconstraint [b_ijqm1 || a_jm1] && p_mm1
  z3::expr fusionMoleculeFormula(ctx);
  auto flag = false;
  if (m > qSnareCount) {
    flag = true;
  }
  auto ae = active_edge[i][j][q][m];
  auto an = active_node[j][m];
  // fusion candidate that cause fusion
  if (f1Orf2 == true) {
    // fusionMoleculeFormula =  ae || an;
    fusionMoleculeFormula = (ae || an) && (!ae || !an);
    // increase the possible count of mole on edge o node
    molOnEdge.push_back(ae);
    molOnTargetNode.push_back(an);
  } else {
    fusionMoleculeFormula = ae;
  }
  // fusionMoleculeFormula.push( ( ae || an ) && ( !ae || !an ) );

  create_formula(qSnareFml, rSnareFml, molOnEdge, molOnTargetNode, i, j, q, m,
                 flag);

  if (flag == true) {
    if (f1Orf2 == true) {
      auto sideCons1 = at_least_two(qSnareFml);
      // auto sideCons1 = !at_least_three(qSnareFml) && at_least_two(qSnareFml);
      auto sideCons2 = at_least_one(rSnareFml);
      sideCons = sideCons1 && sideCons2;
      // auto sideCons2 = at_least_one(rSnareFml) && at_most_one(rSnareFml);
    } else {
      auto sideCons1 = !at_least_two(qSnareFml);
      // auto sideCons1 = !at_least_three(qSnareFml) && at_least_two(qSnareFml);
      auto sideCons2 = !at_least_one(rSnareFml);
      sideCons = sideCons1 && sideCons2;
    }
  } else {
    if (f1Orf2 == true) {
      sideCons = at_least_three(qSnareFml);
    } else {
      sideCons = !at_least_three(qSnareFml);
    }
    // sideCons = at_least_three(qSnareFml) && !at_least_four(qSnareFml);
  }
  auto atleastOneMolOnEdge = mk_or(molOnEdge);
  auto atleastOneMolOnTargetNode = mk_or(molOnTargetNode);
  auto molCons = atleastOneMolOnEdge && atleastOneMolOnTargetNode;
  fusionMoleculeFormula =
      fusionMoleculeFormula && mk_or(qSnareFml) && mk_or(rSnareFml);

  auto cf = fusionMoleculeFormula && sideCons && molCons;
  candidateMoleculeFormula.push_back(cf);
}

z3::expr vts::qr_edge_must_fuse_with_target() {  // V7
  z3::expr_vector ls(ctx);
  for (unsigned i = 0; i < N; i++) {
    for (unsigned j = 0; j < N; j++) {
      if (j == i) continue;
      for (unsigned q = 0; q < E_arity; q++) {
        // Exists a molecule m that is a candidate for fusion
        z3::expr_vector candidateMoleculeFormula(ctx);
        // lhs = ctx.bool_val(false);
        for (unsigned m = 0; m < M; m++) {
          fusion_constraint(candidateMoleculeFormula, i, j, q, m, true);
        }
        auto cons = mk_or(candidateMoleculeFormula);
        z3::expr e = implies(edges[i][j][q], cons);
        ls.push_back(e);
      }
    }
  }
  return z3::mk_and(ls);
}

// For each active molecule on the edge there is no possibility
// of fusion with anyother node than it's target.
z3::expr vts::qr_edge_fuse_only_with_target() {  // V8
  z3::expr_vector ls(ctx);
  //  z3::expr lhs(ctx);
  for (unsigned i = 0; i < N; i++) {
    for (unsigned j = 0; j < N; j++) {
      if (j == i) continue;
      for (unsigned q = 0; q < E_arity; q++) {
        // For each possible active candidate molecule
        for (unsigned m = 0; m < M; m++) {
          z3::expr_vector candidateMoleculeFormula(ctx);
          z3::expr_vector rhs(ctx);
          // For every j1 != j
          for (unsigned j1 = 0; j1 < N; j1++) {
            fusion_constraint(candidateMoleculeFormula, i, j1, q, m, false);
            rhs.push_back(mk_or(candidateMoleculeFormula));
          }
          auto cons = mk_and(rhs);
          z3::expr e = implies(active_edge[i][j][q][m], cons);
          ls.push_back(e);
        }
      }
    }
  }
  return z3::mk_and(ls);
}

// V7 : There should be an active pair corresponding to pairing matrix
z3::expr vts::edge_must_fuse_with_target() {  // V7
  z3::expr_vector ls(ctx);
  z3::expr lhs(ctx);
  for (unsigned i = 0; i < N; i++) {
    for (unsigned j = 0; j < N; j++) {
      if (j == i) continue;
      for (unsigned q = 0; q < E_arity; q++) {
        lhs = ctx.bool_val(false);
        for (unsigned m = 0; m < M; m++) {
          for (unsigned m1 = 0; m1 < M; m1++) {
            if (m == m1) continue;
            lhs = ((active_edge[i][j][q][m] && active_node[j][m1] &&
                    pairing_m[m][m1]) ||
                   lhs);
          }
        }
        z3::expr e = implies(edges[i][j][q], lhs);
        ls.push_back(e);
      }
    }
  }
  return z3::mk_and(ls);
}

//  V8: Edge should not be able to potentially fuse with
//      any node other than it's target.
z3::expr vts::edge_fuse_only_with_target() {  // V8
  z3::expr_vector ls(ctx);
  z3::expr lhs(ctx);
  for (unsigned i = 0; i < N; i++) {
    for (unsigned j = 0; j < N; j++) {
      if (j == i) continue;
      for (unsigned q = 0; q < E_arity; q++) {
        for (unsigned m = 0; m < M; m++) {
          lhs = ctx.bool_val(false);
          for (unsigned j1 = 0; j1 < N; j1++) {
            if (j1 == j) continue;
            for (unsigned m11 = 0; m11 < M; m11++) {
              if (m == m11) continue;
              lhs = ((active_node[j1][m11] && pairing_m[m][m11]) || lhs);
            }
          }
          z3::expr e = implies(active_edge[i][j][q][m], !lhs);
          ls.push_back(e);
        }
      }
    }
  }
  return z3::mk_and(ls);
}

/*** Reachability  Definition ***/
z3::expr vts::reachability_def() {
  z3::expr_vector a_list(ctx);
  for (unsigned i = 0; i < N; i++) {
    for (unsigned j = 0; j < N; j++) {
      if (i == j) continue;
      for (unsigned m = 0; m < M; m++) {
        for (unsigned z = 0; z < N - 1; z++) {
          if (z == 0) {
            a_list.push_back(
                z3::implies(reach[i][j][m][0], is_mol_edge_present(i, j, m)));
          } else {
            z3::expr_vector rhs_list(ctx);
            for (unsigned l = 0; l < N; l++) {
              if (l == i || l == j) continue;
              rhs_list.push_back(reach[l][j][m][z - 1] &&
                                 is_mol_edge_present(i, l, m));
            }
            auto rhs = z3::mk_or(rhs_list);
            a_list.push_back(z3::implies(reach[i][j][m][z], rhs));
          }
        }
      }
    }
  }
  return z3::mk_and(a_list);
}

z3::expr vts::steady_state_stability_cond() {  // R2
  z3::expr_vector a_list(ctx);
  for (unsigned i = 0; i < N; i++) {
    for (unsigned j = 0; j < N; j++) {
      if (i == j) continue;
      for (unsigned m = 0; m < M; m++) {
        z3::expr_vector path_list(ctx);
        for (unsigned z = 0; z < N - 1; z++) {
          path_list.push_back(reach[j][i][m][z]);
        }
        z3::expr path_i_j_m = z3::mk_or(path_list);
        // todo: the following is correct most likely
        // z3::expr path_i_j_m = reach[j][i][m][N-2];
        a_list.push_back(z3::implies(is_mol_edge_present(i, j, m), path_i_j_m));
      }
    }
  }
  return z3::mk_and(a_list);
}

//-------------------------------
/// Connectivity  ----------------------------
//------------------------

// Constraint D1 -------------
// D1: Only present edges can be dropped.
z3::expr vts::only_present_edges_can_be_dropped(Vec3Expr& dump) {  //
  z3::expr_vector ls(ctx);
  z3::expr lhs(ctx);
  for (unsigned i = 0; i < N; i++) {
    for (unsigned j = 0; j < N; j++) {
      if (i == j) continue;
      for (unsigned q = 0; q < E_arity; q++) {
        ls.push_back(implies(dump[i][j][q], edges[i][j][q]));
      }
    }
  }
  return z3::mk_and(ls);
}

// falttening the array's

// VecExpr vts::flattern_2d ( Vec2Expr& dump ) {
//  VecExpr d1;
//  for ( unsigned int i = 0; i < N; i++ ) {
//    for( unsigned j = 0 ; j < N; j++ ) {
//      if (j == i)
//        continue;
//        d1.push_back( dump[i][j] );
//     }
//    }
//  return d1;
//}

z3::expr vts::is_undirected_dumped_edge(unsigned i, unsigned j,
                                        Vec3Expr& dump) {  //
  z3::expr_vector p_list(ctx);
  // p_list.push_back ( ctx.bool_val(false) );
  for (unsigned q = 0; q < E_arity; q++) {
    p_list.push_back((edges[i][j][q] && !dump[i][j][q]) ||
                     (edges[j][i][q] && !dump[j][i][q]));
  }
  return z3::mk_or(p_list);
}

// undirected reachability
z3::expr vts::reachability_under_drop_def(Vec2Expr& r_vars, Vec3Expr& dump,
                                          unsigned conn_or_not) {  //
  z3::expr_vector cond_list(ctx);
  for (unsigned i = 0; i < N; i++) {
    for (unsigned j = 0; j < N; j++) {
      if (j == i) continue;
      z3::expr ud_i_j = is_undirected_dumped_edge(i, j, dump);
      z3::expr_vector paths_list(ctx);
      for (unsigned l = 0; l < N; l++) {
        if (l == i || l == j) continue;
        paths_list.push_back(is_undirected_dumped_edge(i, l, dump) &&
                             r_vars[l][j]);
      }
      // auto cond = z3::implies( r_vars[i][j], ud_i_j || z3::mk_or( paths_list
      // ) );
      if (conn_or_not == 0) {
        auto cond = z3::implies(r_vars[i][j], ud_i_j || z3::mk_or(paths_list));
        cond_list.push_back(cond);
      } else {
        auto cond = z3::implies(ud_i_j || z3::mk_or(paths_list), r_vars[i][j]);
        cond_list.push_back(cond);
      }
    }
  }

  return z3::mk_and(cond_list);
}

z3::expr vts::reachability_under_drop_def(Vec3Expr& r_vars,
                                          Vec3Expr& dump) {  //
  z3::expr_vector cond_list(ctx);
  for (unsigned i = 0; i < N; i++) {
    for (unsigned j = 0; j < N; j++) {
      if (j == i) continue;
      z3::expr ud_i_j = is_undirected_dumped_edge(i, j, dump);
      for (unsigned p = 0; p < N - 1; p++) {
        if (p == 0) {
          cond_list.push_back(z3::implies(r_vars[i][j][0], ud_i_j));
        } else {
          z3::expr_vector paths_list(ctx);
          for (unsigned l = 0; l < N; l++) {
            if (l == i || l == j) continue;
            paths_list.push_back(is_undirected_dumped_edge(i, l, dump) &&
                                 r_vars[l][j][p - 1]);
          }
          auto cond =
              z3::implies(r_vars[i][j][p], ud_i_j || z3::mk_or(paths_list));
          cond_list.push_back(cond);
        }
      }
    }
  }

  return z3::mk_and(cond_list);
}

// z3::expr vts::remains_connected( Vec2Expr& r_varas ){                 //
//   z3::expr_vector cond_list(ctx);
//   for ( unsigned i = 0; i < N; i++ ) {
//     for( unsigned j = i+1 ; j < N; j++ ) {
//       if (j == i) continue;
//       cond_list.push_back( r_varas[i][j] );
//     }
//   }
//   return z3::mk_and( cond_list );
// }

z3::expr vts::remains_connected(Vec3Expr& r_varas) {  //
  z3::expr_vector cond_list(ctx);
  for (unsigned i = 0; i < N; i++) {
    for (unsigned j = i + 1; j < N; j++) {
      if (j == i) continue;
      cond_list.push_back(r_varas[i][j][N - 1]);
    }
  }
  return z3::mk_and(cond_list);
}

z3::expr vts::gets_disconnected(Vec2Expr& r_varas) {  //
  z3::expr_vector cond_list(ctx);
  // Might not be neccessary
  cond_list.push_back(ctx.bool_val(false));
  for (unsigned i = 0; i < N; i++) {
    for (unsigned j = i + 1; j < N; j++) {
      if (j == i) continue;

      cond_list.push_back(!r_varas[i][j]);
      // cond_list.push_back( r_varas[i][j] );
    }
  }
  // std::cout << z3::mk_or( cond_list );
  // exit(0);
  return z3::mk_or(cond_list);
  // return ! (z3::mk_and( cond_list ) );
}

z3::expr vts::not_k_connected(unsigned k, Vec2Expr& r_varas, Vec3Expr& dump) {
  return only_present_edges_can_be_dropped(dump) && exactly_k_drops(k, dump) &&
         reachability_under_drop_def(r_varas, dump, 1) &&
         gets_disconnected(r_varas);
  //&& remains_connected( r_varas );
}

// z3::expr vts::k_min_1_connected( unsigned k, Vec2Expr& r_varas, Vec3Expr&
// dump ) {
//   return only_present_edges_can_be_dropped( dump )
//     && exactly_k_drops( k, dump )
//     && reachability_under_drop_def( r_varas, dump, 0 )
//     && remains_connected( r_varas );
// }

z3::expr vts::not_a_function(Vec2Expr& nodes, Vec2Expr& active_node) {
  z3::expr_vector cond_list(ctx);

  for (unsigned i = 0; i < N; i++) {
    for (unsigned j = i + 1; j < N; j++) {
      z3::expr_vector node_eq(ctx);
      z3::expr_vector actv_node_eq(ctx);
      for (unsigned int k = 0; k < M; k++) {
        node_eq.push_back(nodes[i][k] == nodes[j][k]);
        actv_node_eq.push_back(active_node[i][k] == active_node[j][k]);
      }
      auto cond = z3::implies(z3::mk_and(node_eq), z3::mk_and(actv_node_eq));
      cond_list.push_back(cond);
    }
  }

  return (!(z3::mk_and(cond_list)));
}

/** Create constraint formula for vts **
 *
 * 1. Basic constraints : (V1...V6) - V5
 * 2. Self edge disallowed : V5
 * 3. Stability : R1, R2
 * 4. Fusion : V7, V8
 * Create_vts_constraint = 1 + 2 + 3 + 4
 *
 */

z3::expr vts::vts_activity_constraint() {
  z3::expr a1 = node_activity_constraint();
  z3::expr a2 = edge_activity_constraint();
  auto cons = a1 && a2;
  return cons;
}

z3::expr vts::vts_basic_constraints() {
  z3::expr v1 = molecule_presence_require_for_present_edge();  // V1
  z3::expr v2 = active_molecule_is_present_on_edge();          // V2
  z3::expr v3 = active_molecule_is_present_on_node();          // V3
  z3::expr v4 = edge_label_subset_of_node_label();             // V4
  z3::expr v6 = restriction_on_pairing_matrix();               // V6

  z3::expr base_cons = v1 && v2 && v3 && v4 && v6;
  // z3::expr base_cons = v1 && v2 && v3 && v4;
  return base_cons;
}

z3::expr vts::qr_vts_basic_constraints() {
  z3::expr v1 = molecule_presence_require_for_present_edge();  // V1
  z3::expr v2 = active_molecule_is_present_on_edge();          // V2
  z3::expr v3 = active_molecule_is_present_on_node();          // V3
  z3::expr v4 = edge_label_subset_of_node_label();             // V4
  z3::expr v6 = qr_restriction_on_pairing_matrix();            // V6

  z3::expr base_cons = v1 && v2 && v3 && v4 && v6;
  // z3::expr base_cons = v1 && v2 && v3 && v4;
  return base_cons;
}

z3::expr vts::vts_self_edges_constraint() {
  z3::expr v5 = no_self_edges();  // V5
  return v5;
}

z3::expr vts::vts_stability_constraint() {
  z3::expr r1 = reachability_def();             // R1
  z3::expr r2 = steady_state_stability_cond();  // R2
  z3::expr stab = r1 && r2;
  return stab;
}

z3::expr vts::vts_fusion_constraint() {
  z3::expr v7 = edge_must_fuse_with_target();  // V7
  z3::expr v8 = edge_fuse_only_with_target();  // V8
  auto cons = v7 && v8;
  return cons;
}

z3::expr vts::vts_qr_fusion_constraint() {
  std::cout << "Making qr fusion constraints!\n";
  z3::expr v7 = qr_4d_edge_must_fuse_with_target();  // V7
  z3::expr v8 = qr_4d_edge_fuse_only_with_target();  // V8
  auto cons = v7 && v8;
  // auto cons = v7;
  std::cout << "Finished qr fusion constraints!\n";
  // auto cons = ctx.bool_val(true);
  return cons;
}

z3::expr vts::create_vts_constraint() {
  auto cons = vts_basic_constraints() && vts_self_edges_constraint() &&
              vts_stability_constraint() && vts_fusion_constraint();
  return cons;
}

z3::expr vts::create_qr_vts_constraint() {
  auto cons = qr_vts_basic_constraints() && vts_self_edges_constraint() &&
              vts_stability_constraint() && vts_qr_fusion_constraint();
  return cons;
}

z3::model vts::get_vts_for_synth(z3::expr f) {
  z3::solver s(ctx);
  s.add(f);
  if (s.check() == z3::sat) {
    z3::model m = s.get_model();
    // std::cout << m << "\n";
    return m;
  } else {
    std::cout << "model is not feasible!";
    assert(false);
  }
  return s.get_model();  // dummy call
}

/**
 * z3 model by solving built vts constraints
 */
z3::model vts::get_vts_for_prob1() {
  z3::expr V5 = no_self_edges();  // V5
  z3::expr basic_constraints = create_vts_constraint();
  z3::expr connectivity = not_k_connected(3, d_reach2, drop1);
  // z3::expr connectivity = not_k_connected( 3, d_reach1, drop1 );
  z3::expr cons = basic_constraints && V5 && edges[0][1][0] && connectivity;
  // z3::expr cons =  basic_constraints_with_stability && edges[0][1][0];

  z3::solver s(ctx);
  s.add(cons);
  if (s.check() == z3::sat) {
    z3::model m = s.get_model();
    // std::cout << m << "\n";
    return m;
  } else {
    std::cout << "model is not feasible!";
    assert(false);
  }
  return s.get_model();  // dummy call
}
