#ifndef VTS_H
#define VTS_H

#include <string>
#include <memory>
#include <z3++.h>
#include <z3-util.h>
#include <map>

enum model_version {
  UN_INIT,
  MODEL_1,
  MODEL_2,
  MODEL_3,
  MODEL_4,
  MODEL_5,
  MODEL_6
};


class vts{
public:
  vts( z3::context& _ctx,
       unsigned _M, unsigned _N, unsigned _Q, model_version _V, unsigned _C, unsigned _D )
    : ctx(_ctx), M(_M), N(_N), E_arity(_Q),
      V(_V), C(_C), D(_D),
      knownNodes( _ctx ),
      knownActiveNodes( _ctx),
      knownEdges(_ctx),
      knownPresenceEdges( _ctx ),
      knownActiveEdges( _ctx ),
      knownPairingMatrix( _ctx )
      
      /** vector version *
      knownNodesTuple(),
      knownActiveNodesTuple(),
      knownEdgesTuple(),
      knownPresenceEdgesTuple(),
      knownActiveEdgesTuple(),
      knownPairingMatrixTuple()
      */
      
  {
    init_vts();
  }

private:
  z3::context& ctx;

  unsigned   M;        // Molecules
  unsigned   N;       // Nodes
  unsigned   E_arity;    // Edge arity
  model_version V;   // Variation
  unsigned   C;     // connectedness
  unsigned D;      // CNF conjunction depth 
  unsigned L;

  void init_vts();

  //variables
  Vec2Expr nodes;
  Vec2Expr active_node;

  Vec3Expr node_parameter_var;
  Vec3Expr edge_parameter_var;
  Vec3Expr gate_selector_var_node;
  Vec3Expr gate_selector_var_edge;

  Vec3Expr edges;
  Vec4Expr presence_edge;
  Vec4Expr active_edge;

  Vec2Expr pairing_m;
  Vec4Expr pairing_m_4d;
  std::vector<z3::func_decl> node_funcs;
  std::vector<z3::func_decl> edge_funcs;

  Vec4Expr reach;
  Vec3Expr drop1;
  // Vec2Expr d_reach1;
  Vec3Expr d_reach1;
  Vec3Expr drop2;
  Vec2Expr d_reach2;

  /* 
  // Tuple version
  Tup2Expr knownNodesTuple;
  Tup2Expr knownActiveNodesTuple;
  Tup3Expr knownEdgesTuple;
  Tup4Expr knownPresenceEdgesTuple;
  Tup4SignedExpr knownActiveEdgesTuple;
  Tup2Expr knownPairingMatrixTuple;
  */
  
  z3::expr_vector knownNodes; 
  z3::expr_vector knownActiveNodes;
  z3::expr_vector knownEdges;
  z3::expr_vector knownPresenceEdges;
  z3::expr_vector knownActiveEdges;
  z3::expr_vector knownPairingMatrix;
  
  //flat version of variables
  // VecExpr flat_nodes;
  // VecExpr flat_active_node;

  // VecExpr flat_edges;
  // VecExpr flat_presence_edge;
  // VecExpr flat_active_edge;
  // Vec3Expr dump2;

  //constraints

  void popl1( VecExpr&, unsigned,  std::string);
  void popl2( Vec2Expr&, unsigned, unsigned, std::string);
  void popl3( Vec3Expr&, unsigned, unsigned, unsigned, std::string);
  void popl4( Vec4Expr&, unsigned, unsigned, unsigned, unsigned, std::string);

  void make_func ( std::vector<z3::func_decl>&, std::string);


public:

  // adding known parts
  void add_known_edge( unsigned, unsigned, unsigned, std::vector<unsigned>&,
                       std::vector<unsigned>&);
  void add_known_node( unsigned, std::vector<unsigned>&,
                       std::vector<unsigned>&);
  void add_known_pairing( unsigned, unsigned);
  void qr_add_known_pairing( unsigned m1, unsigned m2, unsigned m3, unsigned m4 );
  void add_known_activity_node_function( unsigned, z3::expr );
  void add_known_activity_edge_function( unsigned, z3::expr );

  //formula makers
  z3::expr always_active_on_node();        // f_nn
  z3::expr always_active_on_edge();        // f_ne
  z3::expr pm_dependent_activity_on_edge();// f_se
  z3::expr func_driven_activity_on_node(); // f_bn
  z3::expr func_driven_activity_on_edge(); // f_be

  z3::expr node_activity_constraint();
  z3::expr edge_activity_constraint();


  z3::expr molecule_presence_require_for_present_edge(); //V1
  z3::expr active_molecule_is_present_on_edge();         //V2
  z3::expr active_molecule_is_present_on_node();         //V3
  z3::expr edge_label_subset_of_node_label();            //V4 
  z3::expr no_self_edges();                              //V5
  z3::expr restriction_on_pairing_matrix();              //V6
  z3::expr qr_restriction_on_pairing_matrix();              //V6
  z3::expr edge_must_fuse_with_target();                 //V7
  z3::expr qr_edge_must_fuse_with_target();                //V7
  z3::expr edge_fuse_only_with_target();                 //V8  
  z3::expr qr_edge_fuse_only_with_target();                 //V8  
  z3::expr qr_vts_basic_constraints ();  

  //qr_4d
  z3::expr is_fuse( VecExpr& edge_vec, VecExpr& node_vec,
                         std::vector< unsigned >& fuse_mols );
  z3::expr qr_4d_edge_must_fuse_with_target();                //V7
  z3::expr qr_4d_edge_fuse_only_with_target();                 //V8  
  z3::expr vts_qr_4d_fusion_constraint ();
  
  // Steady state
  z3::expr reachability_def();           //R1
  z3::expr steady_state_stability_cond(); //R2
  
  z3::expr vts_qr_fusion_constraint ();
  z3::expr create_qr_vts_constraint ();

  /** Flattening of the x-d Vectors. **/
  z3::expr_vector flattern2d ( Vec2Expr& dump, unsigned s1, unsigned s2, bool eq );
  z3::expr_vector flattern3d ( Vec3Expr& dump, unsigned s1, unsigned s2, unsigned s3, bool eq );
  z3::expr_vector flattern4d ( Vec4Expr& dump, unsigned s1, unsigned s2, unsigned s3, unsigned s4, bool eq );
  
  VecExpr flattern_3d ( Vec3Expr& dump );

  /** HELPER FOR VTS */
  void create_formula ( z3::expr_vector& qSnareFml, z3::expr_vector& rSnareFml,
                        z3::expr_vector& molOnEdge, z3::expr_vector& molOnTargetNode,
                        unsigned i, unsigned j, unsigned q, unsigned m, bool flag );
  void fusion_constraint (z3::expr_vector& candidateMoleculeFormula, unsigned i, unsigned j, unsigned q, unsigned m, bool f1Orf2 );

  /*** connectivity constraints ***/
  // At least at most
  z3::expr at_least_one( VecExpr dump );
  z3::expr at_least_two( VecExpr dump );
  z3::expr at_least_three( VecExpr dump );
  z3::expr at_least_four( VecExpr dump );
  z3::expr at_least_five( VecExpr dump  );
  
  // Overloaded function
  z3::expr at_least_one( z3::expr_vector dump  );
  z3::expr at_least_two ( z3::expr_vector dump );
  z3::expr at_least_three ( z3::expr_vector dump  );
  z3::expr at_least_four( z3::expr_vector dump  );

  z3::expr at_most_one ( z3::expr_vector dump );

  z3::expr cardC (z3::expr_vector d1, unsigned drop_count);
  /** connectivity costraints **/
  // todo: variables are needed to be parametrized
  z3::expr only_present_edges_can_be_dropped( Vec3Expr& dump ); //
  z3::expr atleast_k_drops(unsigned k);         //
  z3::expr atmost_k_drops(unsigned k);          //
  z3::expr exactly_k_drops(unsigned k, Vec3Expr& dump);         //
  z3::expr reachability_under_drop_def(Vec2Expr& r_varas, Vec3Expr& dump1, unsigned conn_or_not);//
  z3::expr reachability_under_drop_def( Vec3Expr& r_varas, Vec3Expr& dump1 );
  z3::expr remains_connected( Vec3Expr& r_varas );                 //
  // z3::expr remains_connected( Vec2Expr& r_varas );                 //
  z3::expr gets_disconnected( Vec2Expr& r_varas );                 //

  z3::expr not_k_connected( unsigned k, Vec2Expr& r_varas, Vec3Expr& dump );
  z3::expr k_min_1_connected( unsigned k, Vec2Expr& r_varas, Vec3Expr& dump );
  
  // K conectivity function
  z3::expr k_connected_graph_constraint ( unsigned K, bool eQuantVar );

  /** Cnf encoding constraints **/
  z3::expr literal_cnf (Vec2Expr s, z3::expr_vector&, unsigned i, unsigned k, bool e, unsigned n, unsigned q);
  z3::expr not_a_function( Vec2Expr& nodes, Vec2Expr& active_node );
  z3::expr cnf_function ( Vec3Expr& p_var, Vec3Expr& s_var );

  z3::expr node_cnf ( Vec3Expr& s );
  z3::expr edge_cnf ( Vec3Expr& t );

  /** Gates constraint **/
  z3::expr gates( VecExpr& u, z3::expr x, z3::expr y );
  z3::expr build_rhs_fml ( Vec2Expr& s, Vec2Expr& u, z3::expr_vector&, unsigned i, unsigned k, bool e, unsigned j, unsigned q );
  z3::expr var_fml ( VecExpr& chooseVar, z3::expr_vector& sideConstraintFormula, z3::expr_vector& sideConstraintLiteral, unsigned i, unsigned m, bool e, unsigned j, unsigned q ); 
  z3::expr node_gate_fml ( Vec3Expr& s, Vec3Expr& u );
  z3::expr edge_gate_fml ( Vec3Expr& t, Vec3Expr& v );
  z3::expr logic_gates ( Vec3Expr& s_var, Vec3Expr& t_var, Vec3Expr& gate_selector_var_node, Vec3Expr& gate_selector_var_edge );
  z3::expr_vector reduce_fml ( z3::context& ctx, z3::expr_vector& main_list,  unsigned mLen, Vec3Expr& u, unsigned k, unsigned& gateVar );
  
  /** Synthesis constraints**/
  z3::expr vts_synthesis ( unsigned variation );
  z3::expr annotate_plos_graph();
  z3::expr annotate_mukund_graph ( z3::expr_vector& fixN, z3::expr_vector& fixactiveN, z3::expr_vector& fixE, z3::expr_vector& fixpresenceE, z3::expr_vector& fixactiveE, z3::expr_vector& fixpairingP );

  // Synthesis helper
  // void print_func_node ( std::vector < std::vector< std::vector <int> > >  tVarStr );
  //std::vector<std::string> get_coordinates ( std::string text );
  void print_denotation_console ( std::map<std::string,int> denotation_map, int  synthVar );
  void create_map ( z3::context& c, std::map<std::string,int>& denotation_map, std::string& depqbfRun, Tup3Expr& nodeT, Tup3Expr& activeNodeT, Tup3Expr& edgeT, Tup4Expr& presenceEdgeT, Tup4Expr& activeEdgeT, VecsExpr qs  );
  //bool equality_check( z3::expr_vector fixZ, z3::expr var );
  //std::vector<std::string> get_coordinates ( std::string text, bool splitOnUnder );

  /** Build constraint and model **/
  void use_z3_qbf_solver ( z3::expr cons );
  z3::expr vts_activity_constraint();
  z3::expr vts_self_edges_constraint();
  z3::expr vts_stability_constraint();
  z3::expr vts_fusion_constraint ();
  z3::expr vts_basic_constraints();
  z3::expr create_vts_constraint();
  z3::model get_vts_for_prob1();
  z3::model get_vts_for_synth( z3::expr f );

  z3::expr create_qbf_formula( int funcType );

  // Helper 
  z3::expr_vector node_list();
  z3::expr_vector active_node_list();
  z3::expr_vector presence_edge_list();
  z3::expr_vector active_edge_list();
  z3::expr_vector pairing_m_list();
  z3::expr_vector pairing_m_4d_list();
  z3::expr_vector reach_list();
  z3::expr_vector edge_list();
  
   //helper functions
  z3::expr is_mol_edge_present( unsigned i, unsigned j, unsigned m );
  z3::expr is_qth_edge_present( unsigned i, unsigned j, unsigned q );
  z3::expr is_undirected_dumped_edge( unsigned i, unsigned j, Vec3Expr& dump1 );
  void dump_dot( std::string filename, z3::model mdl );
  void edge_dump_dot( std::string filename, z3::model mdl );
  void print_graph( z3::context& c ,std::string filename, VecsExpr qs, bool printModel, bool displayGraph, int synthVar );
  
};

typedef std::shared_ptr<vts> vts_ptr;

#endif
