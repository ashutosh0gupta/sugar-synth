#ifndef VTS_LOAD_H
#define VTS_LOAD_H

#include <fstream>

#include "utils/utils.h"
#include "vts/vts.h"

#define load_error( M, Line ) vts_error( "", "line " << Line << ":"<< M )

class load_vts {

private:
  z3::context& c;
  std::string file_name;
  std::ifstream in;
  vts_ptr v = nullptr;
  unsigned line_num = 0;
  bool initialized_vts = false;

  //model parameters
  unsigned int  M = 0;
  unsigned int  N = 0;
  unsigned int  Q = 0;
  model_version V = UN_INIT;
  unsigned int  D = 0;
  std::vector< std::string > mol_vars;

  char peek_skip_space();
  bool peek_num_or_eol();
  unsigned read_num_line();
  char read_char_line();
  std::string read_string_line();
  char get_command();
  void get_label( std::vector<unsigned>& mols,
                  std::vector<unsigned>& activity);
  void get_mol_num();
  void get_node_num();
  void get_max_edge_num();
  void get_model_version();

  void get_node();
  void get_edge();
  void get_pairing();
  void qr_get_pairing();
  void get_node_function();
  void get_edge_function();
public:
  load_vts( z3::context& c_, std::string file_name_ )
    : c(c_)
    , file_name(file_name_) {}
  void load();
  inline vts_ptr get_vts() { return v; };
};

#endif
