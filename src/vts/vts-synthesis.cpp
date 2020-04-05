#include <utils/utils.h>
#include <vts/vts.h>
#include <solver/z3-util.h>
#include <algorithm>

/** To do list :
 * 1. Take Care of at_least_k and at_most_k with asserts() or false adds
 *
 */

/** Synthesis Variations
 * 1. Add edge to achieve graph stability and k connected.
 * 2. Add flow of molecules to fix fusion.
 *   - use available molecules.
 *   - allow other type of molecule.
 * 3. KCnf
 *   - Low depth circuit
 * 4. Gates: And Or
 *   - Function dependence with var occuring once
 * 5. VTS repair.
 * 6. Biological Operations
 *  - Activate
 *  - Deactivate
 */

z3::expr vts::annotate_mukund_graph(z3::expr_vector& fixN,
                                    z3::expr_vector& fixActiveN,
                                    z3::expr_vector& fixE,
                                    z3::expr_vector& fixPresenceE,
                                    z3::expr_vector& fixActiveE,
                                    z3::expr_vector& fixPairingP) {
  /* M = 14, Molecules subgraph of Mukunds VTS`
   * [ Qa2, Qa5, Qa7, Qa8 ] ::> [ M0, M1, M2, M3 ]
   * [ Qb8 ] ::> [ M4 ]
   * [ Qc7, Qc8 ] ::> [ M5, M6 ]
   * [ Qbc2, Qbc2/3, Qbc7 ] ::> [ M7, M8, M9 ]
   * [ R2, R3, R7, R8 ] ::> [ M10, M11, M12, R13 ]
   *
   * Nodes: N = 3
   *        EE :: 0
   *        LE :: 1
   *        PM :: 2
   */

  /*
  // Straighten out few things.
  z3::expr_vector listE = edge_list();
  z3::expr_vector listN = node_list();
  z3::expr_vector listActiveN = active_node_list();
  z3::expr_vector listPresenceE = presence_edge_list();
  z3::expr_vector listActiveE = active_edge_list();
  z3::expr_vector listPairingM = pairing_m_list();
  z3::expr_vector listReach = reach_list();
  */

  // Fix nodes of the graph

  auto n0 = nodes[0][0];
  auto n1 = nodes[0][8];

  auto n2 = nodes[1][3];
  auto n3 = nodes[1][4];
  auto n4 = nodes[1][6];

  auto n5 = nodes[2][1];
  auto n6 = nodes[2][2];
  auto n7 = nodes[2][7];
  auto n8 = nodes[2][9];

  fixN.push_back(n0);
  fixN.push_back(n1);
  fixN.push_back(n2);
  fixN.push_back(n3);
  fixN.push_back(n4);
  fixN.push_back(n5);
  fixN.push_back(n6);
  fixN.push_back(n7);
  fixN.push_back(n8);

  z3::expr nodes_cons = z3::mk_and(fixN);

  // Fix edges of the graph
  auto e0 = edges[0][1][0];
  auto e1 = edges[1][2][0];
  auto e2 = edges[2][0][0];
  auto e3 = edges[0][2][0];

  fixE.push_back(e0);
  fixE.push_back(e1);
  fixE.push_back(e2);
  fixE.push_back(e3);

  z3::expr edge_cons = z3::mk_and(fixE);

  // Fix edge labels of the graph
  e0 = presence_edge[0][1][0][2];
  e1 = presence_edge[0][1][0][9];
  e2 = presence_edge[0][1][0][12];
  e3 = presence_edge[0][1][0][13];

  auto e4 = presence_edge[0][2][0][11];

  auto e5 = presence_edge[1][2][0][9];

  auto e6 = presence_edge[2][0][0][0];
  auto e7 = presence_edge[2][0][0][2];
  auto e8 = presence_edge[2][0][0][9];
  auto e9 = presence_edge[2][0][0][10];
  auto e10 = presence_edge[2][0][0][12];

  fixPresenceE.push_back(e0);
  fixPresenceE.push_back(e1);
  fixPresenceE.push_back(e2);
  fixPresenceE.push_back(e3);
  fixPresenceE.push_back(e4);
  fixPresenceE.push_back(e5);
  fixPresenceE.push_back(e6);
  fixPresenceE.push_back(e7);
  fixPresenceE.push_back(e8);
  fixPresenceE.push_back(e9);
  fixPresenceE.push_back(e10);

  auto edgel_cons = z3::mk_and(fixPresenceE);

  auto cons = nodes_cons && edge_cons && edgel_cons;
  return cons;

  /* 21 Molecules subgraph of Mukunds VTS'
   * [ Qa2, Qa4, Qa5, Qa6, Qa7 ] ::> [ M0, M1, M2, M3, M4 ]
   * [ Qb1, Qb6 ] ::> [ M5, M6 ]
   * [ Qc4, Qc5 ] ::> [ M7, M8 ]
   * [ Qbc2, Qbc3, Qbc7 ] ::> [ M9, M10, M11 ]
   * [ Qbc2, Qbc2/3, Qbc7 ] ::> [ M12, M13, M14 ]
   * [ R2, R3, R4, R6, R7, R8 ] ::> [ M15, M16, M17, M18, M19, M20 ]
   * */
}

z3::expr vts::annotate_plos_graph() {
  /** Example taken from PLOS paper
   * N = 2, M = 6
   * model 3: Arb on edge, Nothing on node
   */
  /** node: n[i][k] : below.  a[i][k] : use Model_3 **/
  // Node 0 : [111 110]
  z3::expr node_cons = !nodes[0][0] && nodes[0][1] && nodes[0][2] &&
                       nodes[0][3] && nodes[0][4] && nodes[0][5];

  /** edge: e[i][j][q], e[i][j][q][k]: below, b[i][j][q][k]: Model_3 **/
  // auto edge_cons1 = edges[0][1][0] && edges[1][0][0] && edges[1][0][1];
  z3::expr edge_cons1 = edges[0][1][0] && edges[1][0][0];
  z3::expr edge_cons2_1 = presence_edge[0][1][0][2] &&
                          presence_edge[0][1][0][3] &&
                          presence_edge[0][1][0][5];
  z3::expr edge_cons2_2 =
      presence_edge[1][0][0][2] && presence_edge[1][0][0][3];
  z3::expr edge_cons = edge_cons1 && edge_cons2_1 && edge_cons2_2;

  /** Pairing Matrix Constraints **/
  z3::expr pairing_cons_1 =
      pairing_m[5][1] && pairing_m[3][0] && pairing_m[2][4];

  auto cons = edge_cons && node_cons && pairing_cons_1;
  return cons;
}

bool equality_check(z3::expr_vector fixZ, z3::expr var) {
  for (unsigned i = 0; i < fixZ.size(); i++) {
    if ((z3::eq(fixZ[i], var) == true)) {
      return true;
    }
  }
  return false;
}

/* /Tuple Version
//static z3::expr_vector DEFAULT_VECTOR;
//static Z3_ast_vector DEFAULT_VECTOR;

static Vec2Expr DEFAULT_2D_XOR;
static Vec3Expr DEFAULT_3D_XOR;
static Vec4Expr DEFAULT_4D_XOR;

// todo: rewrite 4d ->  using 3d -> using 2d
void unassigned_2d_bits ( std::vector < std::tuple <unsigned, unsigned> >&
knownTuple, z3::expr_vector& knownVector, z3::expr_vector& unassignedVector,
                          Vec2Expr& vec, unsigned arg1, unsigned arg2, bool
edgeCase ) {
                         // Vec2Expr& xorVec = DEFAULT_2D_XOR,
                       //   z3::expr_vector& listVar = {}, bool xorCase ) {
  auto var = knownTuple.begin();
  std::cout << "First element is: " << std::get<0> (*var);
  exit(0);
  for ( unsigned i = 0; i < arg1; i++ ) {
    for ( unsigned j = 0; j < arg2; j++ ) {
      if ( (i == j) && (edgeCase == true) ) continue;
      if ( i != std::get<0> (*var) || j != std::get<1> (*var) ) {
        unassignedVector.push_back( vec[i][j] );
      } else {
      //  if ( xorCase == false ) {
         knownVector.push_back( vec[i][j] );
      //  } else {
      //    knownVector.push_back( vec[i][j] ^ xorVec[i][j] );
        //  listVar.push_back( xorVec[i][j] );
       // }
        var = std::next(var);
      }
    }
  }
}

*/

void unassigned_bits(z3::expr_vector& listZ, z3::expr_vector& knownZ,
                     z3::expr_vector& unknownZ) {
  for (unsigned i = 0; i < listZ.size(); i++) {
    auto var = listZ[i];
    if (equality_check(knownZ, var))
      continue;
    else
      unknownZ.push_back(var);
  }
}

Vec2Expr DEFAULT_2V;
Vec3Expr DEFAULT_3V;
Vec4Expr DEFAULT_4V;

void xor_bits(z3::context& c, z3::expr_vector& listZ, z3::expr_vector& knownZ,
              z3::expr_vector& fixKnown, z3::expr_vector& unknownZ,
              z3::expr_vector& xorZ, unsigned seq, unsigned step,
              Vec2Expr& vec2 = DEFAULT_2V, Vec3Expr& vec3 = DEFAULT_3V,
              Vec4Expr& vec4 = DEFAULT_4V) {
  for (unsigned i = 0; i < listZ.size(); i++) {
    auto var = listZ[i];
    if (equality_check(knownZ, var)) {
      auto coord = get_coordinates(Z3_ast_to_string(c, var), true);
      auto x = std::stoi(coord[1]);
      auto y = std::stoi(coord[2]);
      if (seq == 2) {
        xorZ.push_back(vec2[x][y]);
        if (step == 0 || step == 1 || step == 5) {
          fixKnown.push_back((var && !vec2[x][y]) || (!var && vec2[x][y]));
        }
      } else if (seq == 3) {
        auto z = std::stoi(coord[3]);
        xorZ.push_back(vec3[x][y][z]);
        if (step == 2) {
          fixKnown.push_back((var && !vec3[x][y][z]) ||
                             (!var && vec3[x][y][z]));
        }
      } else if (seq == 4) {
        auto z = std::stoi(coord[3]);
        auto w = std::stoi(coord[4]);
        xorZ.push_back(vec4[x][y][z][w]);
        if (step == 3 || step == 4) {
          fixKnown.push_back((var && !vec4[x][y][z][w]) ||
                             (!var && vec4[x][y][z][w]));
        }
      } else {
        continue;
      }
    } else {
      unknownZ.push_back(var);
    }
  }
}

z3::expr vts::vts_synthesis(unsigned variation) {
  /** Basic Constraints **/
  // z3::expr vtsCons = create_vts_constraint();
  z3::expr vtsCons = create_qr_vts_constraint();
  // z3::expr nodeActive = always_active_on_node();
  // z3::expr vtsActivity = vts_activity_constraint();

  /** Connectedness Constraints */
  z3::expr kConnCons = k_connected_graph_constraint(3, false);
  // z3::expr V5 = no_self_edges();
  // z3::expr inputCons = ctx.bool_val(true);

  /** Known and Unknown Bit variables */
  z3::expr_vector knownNodes(ctx);
  z3::expr_vector knownActiveNodes(ctx);
  z3::expr_vector knownEdges(ctx);
  z3::expr_vector knownPresenceEdges(ctx);
  z3::expr_vector knownActiveEdges(ctx);
  z3::expr_vector knownPairingMatrix(ctx);

  z3::expr_vector unknownN(ctx);
  z3::expr_vector unknownActiveN(ctx);
  z3::expr_vector unknownE(ctx);
  z3::expr_vector unknownPresenceE(ctx);
  z3::expr_vector unknownActiveE(ctx);
  z3::expr_vector unknownPairingM(ctx);

  /** Annotate graph : fix graph known variables **/
  // z3::expr inputCons =  annotate_mukund_graph ( knownNodes, knownActiveNodes,
  // knownEdges, knownPresenceEdges, knownActiveEdges, knownPairingMatrix );

  /** Flattern all the possible Elemsnts */
  z3::expr_vector listN = node_list();
  z3::expr_vector listActiveN = active_node_list();
  z3::expr_vector listE = edge_list();
  z3::expr_vector listPresenceE = presence_edge_list();
  z3::expr_vector listActiveE = active_edge_list();
  z3::expr_vector listReach = reach_list();
  z3::expr_vector listPairing4M = pairing_m_4d_list();
  // z3::expr_vector listPairingM = pairing_m_list();

  z3::expr knownVarConstraint(ctx);

  // TODO: Check how the knownbits are fixed
  if (variation != 5) {
    unassigned_bits(listN, knownNodes, unknownN);
    unassigned_bits(listActiveN, knownActiveNodes, unknownActiveN);
    unassigned_bits(listE, knownEdges, unknownE);
    unassigned_bits(listPresenceE, knownPresenceEdges, unknownPresenceE);
    unassigned_bits(listActiveE, knownActiveEdges, unknownActiveE);
    unassigned_bits(listPairing4M, knownPairingMatrix, unknownPairingM);
    // unassigned_bits ( listPairingM, knownPairingMatrix, unknownPairingM );

    z3::expr_vector list_expr(ctx);

    list_expr.push_back(z3::mk_and(knownEdges));
    list_expr.push_back(z3::mk_and(knownPresenceEdges));
    list_expr.push_back(z3::mk_and(knownNodes));

    if (knownPairingMatrix.size() > 0) {
      list_expr.push_back(z3::mk_and(knownPairingMatrix));
    }
    if (knownActiveNodes.size() > 0) {
      list_expr.push_back(z3::mk_and(knownActiveNodes));
    }
    if (knownActiveEdges.size() > 0) {
      list_expr.push_back(z3::mk_and(knownActiveEdges));
    }

    knownVarConstraint = z3::mk_and(list_expr);

    /*
    for( unsigned i = 0; i < unknownPairingM.size(); i++  ) {
      std::cout << "Nodes is " << unknownPairingM[i] << "\n" ;
    }
    */
  }

  // 1. Add edge to achieve graph stability and k connected.
  if (variation == 1) {
    // auto nodeC = cardC( unknownN, 4 );
    // auto edgeC = cardC( unknownE, 1);
    // auto edgeC =  pbeq(unknownE , 1, int 1);
    // auto edgeC =  atmost(unknownE, 1);
    //     auto edgeC = !at_least_two( unknownE );
    // auto edgeC = !at_least_three( unknownE );
    auto edgeC = mk_le_k_bits(unknownE, 3);
    /*
    for ( unsigned i = 0; i < knownEdges.size(); i++ ) {
      std::cout << "known edge: " << knownEdges[i] << "\n";
    }
    */
    // auto edgeActivityC = !at_least_three( unknownActiveE );
    // auto edgePresenceC = !at_least_four( unknownPresenceE );
    // auto nodeC = !at_least_four( unknownN );
    // auto nodeActivityC = !at_least_three ( unknownActiveN );
    /*
    for( unsigned i = 0; i < unknownN.size(); i++  ) {
      std::cout << "Nodes is " << unknownN[i] << "\n" ;
    }
    */

    // fix all unknwon bits to false.
    auto setUnknownVariablesFalse = ctx.bool_val(true)
        // && !z3::mk_or( unknownN )
        // && !z3::mk_or( unknownActiveN )
        // && !z3::mk_or( unknownE )
        // && !z3::mk_or( unknownPresenceE )
        // && !z3::mk_or( unknownActiveE )
        // && !z3::mk_or ( unknownPairingM )
        ;

    // auto addConstraints = nodeC;
    auto qvtsCons = exists(
        listN,
        exists(listActiveN,
               exists(listPresenceE,
                      exists(listActiveE,
                             //  exists( listPairingM,
                             exists(listPairing4M,
                                    exists(listReach,
                                           vtsCons && knownVarConstraint))))));

    auto cons = exists(listE, qvtsCons && kConnCons && edgeC);
    //   qvtsCons );

    // auto cons = vtsCons && knownVarConstraint && setUnknownVariablesFalse &&
    // edgeC;
    return cons;
  }

  // 2. Add flow of molecules to fix fusion.
  else if (variation == 2) {
    // z3::expr edgeActivityC = !at_least_three( unknownActiveE );
    // z3::expr edgePresenceC = !at_least_three( unknownPresenceE );
    // z3::expr addConstraints = edgeActivityC && edgePresenceC;
    auto edgeC = mk_le_k_bits(unknownPresenceE, 12);

    /*
   // fix all unknwon bits to false.
   //z3::expr setUnknownVariablesFalse = !z3::mk_or( unknownActiveN ) &&
   !z3::mk_or( unknownE ); auto setUnknownVariablesFalse = !z3::mk_or( unknownN
   )  && !z3::mk_or( unknownActiveN )
                                  && !z3::mk_or( unknownE ) && !z3::mk_or(
   unknownPresenceE ) && !z3::mk_or( unknownActiveE );

   z3::expr qvtsCons = exists( listN,
                       exists( listActiveN,
                       exists( listActiveE,
                      // exists( listPairingM,
                      exists( listPairing4M,
                      exists( listReach,
                               vtsCons && knownVarConstraint )))));

   auto cons = exists( listPresenceE,
               exists( listE,
                        qvtsCons && edgeC ));
                       //kConnCons && qvtsCons && edgeC ));
  */
    auto cons = vtsCons && knownVarConstraint && edgeC;

    // kConnCons
    return cons;
  }

  // 3. KCnf  4. Cnf with low depth circuit
  else if (variation == 3) {
    // populate xtra var for node and edge function
    popl3(node_parameter_var, M, D, 2 * M, "s");
    popl3(edge_parameter_var, M, D, 2 * M, "t");

    auto listSvar = flattern3d(node_parameter_var, M, D, 2 * M, false);
    auto listTvar = flattern3d(edge_parameter_var, M, D, 2 * M, false);

    // auto edgeC = mk_le_k_bits( unknownE, 2 ) && mk_le_k_bits(
    // unknownPresenceE, 10); mk_le_k_bits( unknownN, 5);

    //   auto nodeC = !at_least_two( unknownN );

    z3::expr cnfCons = cnf_function(node_parameter_var, edge_parameter_var);

    /*
    auto setUnknownVariablesFalse =  !z3::mk_or( unknownActiveN )
                                   && !z3::mk_or( unknownE ) && !z3::mk_or(
    unknownPresenceE ) && !z3::mk_or( unknownActiveE );
    */

    /*
      z3::expr func3cnf  = exists( listN,
                           exists( listActiveN,
                           exists( listPresenceE,
                           exists( listActiveE,
                           exists( listPairingM,
                           exists( listReach,
                                cnfCons && vtsCons && knownVarConstraint ))))));

      z3::expr cons = exists( listSvar,
                      exists( listTvar,
                      exists( listE,
                              func3cnf )));

        z3::expr cons = exists( listSvar,
                      exists( listTvar,
                      exists( listN,
                      exists( listActiveN,
                      exists( listE,
                      exists( listPresenceE,
                      exists( listActiveE,
                      exists( listPairingM,
                    //  exists( listPairing4M,
                   //   exists( listReach,
                              cnfCons && knownVarConstraint && vtsCons ))))))));
        */
    auto cons = cnfCons && knownVarConstraint && vtsCons;

    return cons;
  } else if (variation == 4) {
    // For gateType == 2 using only 1 place.
    unsigned binaryGateChoice = 1;
    unsigned noOfgates = 3;
    unsigned noOfLeaves = 4;

    // Populate xtra var for node and edge function
    // [M, noOfLeaves, noOfVar: M + T + F ]
    popl3(node_parameter_var, M, noOfLeaves, (2 * M) + 2, "s");
    popl3(edge_parameter_var, M, noOfLeaves, (2 * M) + 2, "t");

    // Populate xtra para for choosing node gates
    // [ M, noOfGates, gateTypes ]
    popl3(gate_selector_var_node, M, noOfgates, binaryGateChoice, "u");
    popl3(gate_selector_var_edge, M, noOfgates, binaryGateChoice, "v");

    // auto edgeC = mk_le_k_bits( unknownE, 3 ) && mk_le_k_bits(
    // unknownPresenceE, 16);

    // auto nodeC = !at_least_two( unknownN );
    // restrict the sreach space by assigning unknown bits to false
    /*
    auto setUnknownVariablesFalse =  //!z3::mk_or( unknownN )
                  !z3::mk_or( unknownActiveN )
                && !z3::mk_or( unknownE )
                // && !z3::mk_or( unknownPresenceE )
                 && !z3::mk_or( unknownActiveE )
                 && !z3::mk_or( unknownPairingM );
    */
    // todo: avoid case M -> M' in first and third arg
    auto listSvar =
        flattern3d(node_parameter_var, M, noOfLeaves, 2 * M + 2, false);
    auto listTvar =
        flattern3d(edge_parameter_var, M, noOfLeaves, 2 * M + 2, false);
    auto listUvar = flattern3d(gate_selector_var_node, M, noOfgates,
                               binaryGateChoice, false);
    auto listVvar = flattern3d(gate_selector_var_edge, M, noOfgates,
                               binaryGateChoice, false);

    z3::expr gateCons =
        logic_gates(node_parameter_var, edge_parameter_var,
                    gate_selector_var_node, gate_selector_var_edge);

    /*
    z3::expr funcGate  = exists( listN,
                         exists( listActiveN,
                         exists( listPresenceE,
                         exists( listActiveE,
                         exists( listPairingM,
                         exists( listReach,
                                 gateCons
                                 &&
                                 vtsCons
                                 &&
                                 knownVarConstraint
                                 && setUnknownVariablesFalse
                                 ))))));

    z3::expr cons = exists( listSvar,
                    exists( listTvar,
                    exists( listUvar,
                    exists( listVvar,
                    exists( listN,
                    exists( listActiveN,
                    exists( listE,
                    exists( listPresenceE,
                    exists( listPairing4M,
                    exists( listActiveE,
                 //   exists( listPairingM,
                    exists( listReach,
                            gateCons && knownVarConstraint && vtsCons  )))))))))
    ));
                            //kConnCons && gateCons && knownVarConstraint &&
    vtsCons  ))))))))) ));
  */
    auto cons = gateCons && knownVarConstraint && vtsCons;
    // todo: V5 has some problems check assing it.

    /** The query to ask
    z3::expr cons = exists( listSvar,
                    exists( listTvar,
                    exists( listUvar,
                    exists( listVvar,z
                    exists( listActiveN,
                    exists( listActiveE,
                             knownVarConstraint && setUnknownVariablesFalse &&
    funcGate )))))));
   */

    return cons;
  } else if (variation == 5) {
    // xor bits collection
    z3::expr_vector xorNVec(ctx);
    z3::expr_vector xorActiveNVec(ctx);
    z3::expr_vector xorPairingMVec(ctx);
    z3::expr_vector xorEVec(ctx);
    z3::expr_vector xorPresenceEVec(ctx);
    z3::expr_vector xorActiveEVec(ctx);
    z3::expr_vector xorReachVec(ctx);

    // xor bits for each boolean var in vts
    Vec2Expr xorNodes;
    Vec2Expr xorActiveNodes;
    Vec3Expr xorEdges;
    Vec4Expr xorPresenceEdges;
    Vec4Expr xorActiveEdges;
    // Vec2Expr xorPairingMatrix;
    Vec4Expr xorPairingMatrix;
    Vec4Expr xorReach;

    popl2(xorNodes, N, M, "xn");
    popl2(xorActiveNodes, N, M, "xa");
    popl3(xorEdges, N, N, E_arity, "xz");
    popl4(xorPresenceEdges, N, N, E_arity, M, "xe");
    popl4(xorActiveEdges, N, N, E_arity, M, "xb");
    //    popl2 ( xorPairingMatrix, M, M, "xp" );
    popl4(xorPairingMatrix, M, M, M, M, "xp");
    //   popl4 ( xorPairingMatrix, M, M, "xp" );
    popl4(xorReach, N, N, M, N - 1, "xr");

    auto listXorN = flattern2d(xorNodes, N, M, false);
    auto listXorActiveN = flattern2d(xorActiveNodes, N, M, false);
    // auto listXorPairingM = flattern2d( xorPairingMatrix, M, M, true );
    auto listXorPairingM = flattern4d(xorPairingMatrix, M, M, M, M, true);
    auto listXorE = flattern3d(xorEdges, N, N, E_arity, true);
    auto listXorPresenceE =
        flattern4d(xorPresenceEdges, N, N, E_arity, M, true);
    auto listXorActiveE = flattern4d(xorActiveEdges, N, N, E_arity, M, true);
    auto listXorReach = flattern4d(xorReach, N, N, M, N - 1, true);

    // known xor bits [xor bits ^ n_i_k]
    z3::expr_vector xorknownN(ctx);
    z3::expr_vector xorknownActiveN(ctx);
    z3::expr_vector xorknownE(ctx);
    z3::expr_vector xorknownPresenceE(ctx);
    z3::expr_vector xorknownActiveE(ctx);
    z3::expr_vector xorknownPairingM(ctx);
    z3::expr_vector xorknownReach(ctx);

    // dummy variables
    Vec2Expr vec2;
    Vec3Expr vec3;
    Vec4Expr vec4;

    // fix known unknown and xor bits
    xor_bits(ctx, listN, knownNodes, xorknownN, unknownN, xorNVec, 2, 0,
             xorNodes);
    xor_bits(ctx, listActiveN, knownActiveNodes, xorknownActiveN,
             unknownActiveN, xorActiveNVec, 2, 1, xorActiveNodes);
    xor_bits(ctx, listPairing4M, knownPairingMatrix, xorknownPairingM,
             unknownPairingM, xorPairingMVec, 4, 4, vec2, vec3,
             xorPairingMatrix);
    // xorPairingMVec, 2, 5, xorPairingMatrix );
    xor_bits(ctx, listE, knownEdges, xorknownE, unknownE, xorEVec, 3, 2, vec2,
             xorEdges);
    xor_bits(ctx, listPresenceE, knownPresenceEdges, xorknownPresenceE,
             unknownPresenceE, xorPresenceEVec, 4, 3, vec2, vec3,
             xorPresenceEdges);
    xor_bits(ctx, listActiveE, knownActiveEdges, xorknownActiveE,
             unknownActiveE, xorActiveEVec, 4, 4, vec2, vec3, xorActiveEdges);

    knownVarConstraint =
        z3::mk_and(xorknownN) && z3::mk_and(xorknownActiveN) &&
        z3::mk_and(xorknownE) && z3::mk_and(xorknownPresenceE) &&
        z3::mk_and(xorknownActiveE) && z3::mk_and(knownPairingMatrix);
    /*
    for( unsigned i = 0; i < xorknownE.size(); i++  ) {
      std::cout << "Nodes is " << xorknownE[i] << "\n" ;
    }
    exit(0);
    *
    // deletion restriction
   auto xadd_node_C = !at_least_two( xorNVec );
    auto xadd_active_node_C = !at_least_two( xorActiveNVec );
    auto xadd_edge_C = !at_least_two( xorEVec );
    auto xadd_p_edge_C = !at_least_two( xorPresenceEVec );
    auto xadd_a_edge_C = !at_least_two( xorActiveNVec );
    */

    auto xadd_node_C = mk_le_k_bits(xorNVec, 2);
    auto xadd_active_node_C = mk_le_k_bits(xorActiveNVec, 2);
    auto xadd_edge_C = mk_le_k_bits(xorEVec, 2);
    auto xadd_p_edge_C = mk_le_k_bits(xorPresenceEVec, 2);
    auto xadd_a_edge_C = mk_le_k_bits(xorActiveEVec, 2);

    auto edgeC = mk_le_k_bits(unknownE, 2);
    auto edgePresenceC = mk_le_k_bits(unknownPresenceE, 20);
    auto nodeC = mk_le_k_bits(unknownN, 20);
    auto edgeActivityC = mk_le_k_bits(unknownActiveE, 8);
    auto nodeActivityC = mk_le_k_bits(unknownActiveN, 8);

    auto addCons = edgeC && edgePresenceC && nodeC;
    auto delCons = xadd_node_C && xadd_active_node_C && xadd_edge_C &&
                   xadd_p_edge_C && xadd_a_edge_C;

    auto setUnknownVariablesFalse =
        !z3::mk_or(unknownN) && !z3::mk_or(unknownActiveN) &&
        !z3::mk_or(unknownE) && !z3::mk_or(unknownPresenceE) &&
        !z3::mk_or(unknownActiveE);

    z3::expr xorCons = knownVarConstraint && delCons && addCons;
    // z3::expr xorCons = knownVarConstraint;

    /*  Tuple version
     unassigned_2d_bits( knownNodesTuple, knownNodes, unknownN, nodes, N, M,
     false, xorNodes, listXorN ); unassigned_2d_bits( knownActiveNodesTuple,
     knownActiveNodes, unknownActiveN, active_node, N, M, false, xorActiveNodes,
     listActiveN ); unassigned_2d_bits( knownPairingMatrixTuple,
     knownPairingMatrix, unknownPairingM, pairing_m, M, M, true,
     xorPairingMatrix, listPairingM );

     unassigned_3d_bits( knownEdgesTuple, knownEdges, unknownE, edges, N, N,
     E_arity, true, xorEdges, listE );

     unassigned_4d_bits( knownPresenceEdgesTuple, knownPresenceEdges,
     unknownPresenceE, presence_edge, N, N, E_arity, M, true, xorPresenceEdges,
     listPresenceE ); unassigned_4d_bits( knownActiveEdgesTuple,
     knownActiveEdges, unknownActiveE, active_edge, N, N, E_arity, M, true,
     xorActiveEdges, listActiveE );

     // todo :: fix this.
     knownVarConstraint =  z3::mk_and( knownEdges )
                               && z3::mk_and( knownActiveNodes )
                               && z3::mk_and( knownPresenceEdges )
                               && z3::mk_and( knownActiveEdges )
                               && z3::mk_and( knownNodes )
                               && z3::mk_and( knownPairingMatrix );

     auto x_node_C = !at_least_three( listXorN );
     auto x_active_nodeC = !at_least_three ( listXorActiveN );
     auto x_edge_C = !at_least_three( listXorE );
     auto x_p_edgeC = !at_least_three( listXorPresenceE );
     auto x_a_edgeC = !at_least_three( listXorActiveN );

     z3::expr delCons = x_node_C && x_active_nodeC && x_edge_C && x_p_edgeC &&
     x_a_edgeC; z3::expr xorCons = knownVarConstraint && delCons;
     */

    // Addition with restriction.
    /*
    auto nodeC = ! at_least_three ( unknownN );
    auto nodeActivityC = ! at_least_two ( unknownActiveE );
    auto edgeC = ! at_least_three ( unknownE );
    auto edgeActivityC = ! at_least_two ( unknownActiveE );
    auto edgePresenceC = ! at_least_two ( unknownPresenceE );

    z3::expr addCons = nodeC && nodeActivityC && edgeC && edgeActivityC &&
    edgePresenceC;

    auto cons = exists( listXorN,
                exists( listXorActiveN,
                exists( listXorPairingM,
                exists( listXorE,
                exists( listXorPresenceE,
                exists( listXorActiveE,
                exists( listE,
                exists( listN,
                    exists( listActiveN,
                    exists( listPresenceE,
                    exists( listActiveE,
                 //   exists( listPairingM,
                     exists( listReach,
                         vtsCons && xorCons ))))))))))));
      */
    //    auto cons = xorCons;
    auto cons = vtsCons && xorCons;
    return cons;

  } else {
    return ctx.bool_val(true);
  }
}

