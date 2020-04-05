#include "load-vts.h"
#include <iostream>
#include <fstream>

// Format
// M
// num
// N
// num
// n
// 2 | 2+ 3-
// e
// 2 3 | 2* 3* 4 5 6 7 
// e
// 2 3 | 2* 3* 4 5 6 7 

char load_vts::peek_skip_space() {
  while( in.peek() == ' ') {
    in.ignore();
  }
  return in.peek();
}

bool load_vts::peek_num_or_eol() {
  char ch = in.peek();
  if( ch == '\n') return true;
  if( ch >= '0' && ch <= '9' ) return true;
  return false;
}

unsigned load_vts::read_num_line() {
  std::string line;
  std::getline ( in, line);
  line_num++;
  return (unsigned)atoi( line.c_str() );
}

char load_vts::read_char_line() {
  std::string line;
  std::getline ( in, line);
  line_num++;
  if( line.size() == 0 ) {
    return 0;
  }else{
    return line[0];
  }
}

std::string load_vts::read_string_line() {
  std::string line;
  std::getline ( in, line);
  line_num++;
  return line;
}

char load_vts::get_command() {
  return read_char_line();
}

void load_vts::get_mol_num() {
  if( M != 0 ) load_error( "re intializing number of molecules", line_num );
  M = read_num_line();
  mol_vars.clear();
  for( unsigned i = 0 ; i < M ; i++ ) {
    mol_vars.push_back( "m" + std::to_string(i));
  }
}

void load_vts::get_node_num() {
  if( N != 0 ) load_error( "re intializing number of nodes", line_num );
  N = read_num_line();
}

void load_vts::get_max_edge_num() {
  if( Q != 0 ) load_error( "re intializing max edges", line_num );
  Q = read_num_line();
}

void load_vts::get_model_version() {
  if( V != UN_INIT ) load_error( "re intializing model version", line_num );
  std::string line = read_string_line();
  if      ( line  == "MODEL_1" ) { V = MODEL_1;
  }else if( line  == "MODEL_2" ) { V = MODEL_2;
  }else if( line  == "MODEL_3" ) { V = MODEL_3;
  }else if( line  == "MODEL_4" ) { V = MODEL_4;
  }else if( line  == "MODEL_5" ) { V = MODEL_5;
  }else if( line  == "MODEL_6" ) { V = MODEL_6;
  }else{
    load_error( "unrecognizable model version", line_num );
  }
}

// activity may conatain dummy expressions
void load_vts::get_label( std::vector<unsigned>& mols,
                          std::vector<unsigned>& activity) {
  char ch;
  in >> ch;
  if( ch != '|' ) load_error( "no | at start of label!", line_num );
  while( peek_skip_space() != '\n' ) {
    unsigned m_idx;
    in >> m_idx;
    mols.push_back( m_idx );
    peek_skip_space();
    if( !peek_num_or_eol() ) {
      char act;
      in >> act;
      switch( act ) {
      case '+': activity.push_back(1); break;
      case '-': activity.push_back(0); break;
      default: goto ERROR;
      }
    }else{
      activity.push_back(2); // unknown
    }
  }
  return;
 ERROR:
  load_error( "bad label!", line_num );
}

void load_vts::get_node() {
  std::vector<unsigned> mols;
  std::vector<unsigned> activity;
  unsigned n ;
  in >> n;
  unsigned next = peek_skip_space();
  if( next == '|' ) {
    get_label( mols, activity );
  }else if( next != '\n' ) {
    load_error( "bad node!", line_num );
  }
  assert( v );
  v->add_known_node( n, mols, activity );
  line_num++;
  return;
}

void load_vts::get_edge() {
  std::vector<unsigned> mols;
  std::vector<unsigned> activity;
  unsigned n1;
  unsigned n2;
  unsigned q;
  in >> n1 >> n2 >> q;
  char next = peek_skip_space();
  if( next == '|' ) {
    get_label( mols, activity );
  }else if( next != '\n' ) {
    load_error( "bad edge!", line_num );
  }
  assert( v );
  v->add_known_edge( n1, n2, q, mols, activity );
  line_num++;
  return;
}

void load_vts::get_pairing() {
  unsigned m1,m2;
  in >> m1 >> m2;
  v->add_known_pairing( m1, m2 );
}

void load_vts::qr_get_pairing() {
  unsigned m1,m2,m3,m4;
  in >> m1 >> m2 >> m3 >> m4;
  v->qr_add_known_pairing( m1, m2, m3, m4 );
}

void load_vts::get_node_function() {
  unsigned m;
  in >> m;
  std::string f_line;
  std::getline( in, f_line );
  z3::expr e = c.bool_val(true);
  parseFormula( c, f_line, mol_vars);
  v->add_known_activity_node_function( m, e );
}

void load_vts::get_edge_function() {
  unsigned m;
  in >> m;
  std::string f_line;
  std::getline( in, f_line );
  z3::expr e = c.bool_val(true);
  parseFormula( c, f_line, mol_vars);
  v->add_known_activity_edge_function( m, e );
}

void load_vts::load() {
  in.open( file_name );
  line_num = 0;
  if( in.is_open() ) {
    while( !in.eof() ) {
      char cmd = get_command();
      switch( cmd ) {
      case 0  :
      case '-': break;
      case 'M': get_mol_num(); break;
      case 'N': get_node_num(); break;
      case 'Q': get_max_edge_num(); break;
      case 'T': get_model_version(); break;
      case 'n': get_node(); break;
      case 'e': get_edge(); break;
      case 'f': get_node_function(); break;
      case 'g': get_edge_function(); break;
   //   case 'p': get_pairing(); break;
      case 'p': qr_get_pairing(); break;
      default:
        load_error( "expecting a command at line", line_num );
      }
      if( !initialized_vts ) {
        V = UN_INIT;
        unsigned C = 3;
        D = 3;
        if( M != 0 && N != 0 && Q != 0 ) {
          initialized_vts = true;
          v = std::shared_ptr<vts>(new vts(c, M, N, Q, V, C, D ));
        }
      }
    }
    in.close();
  }
  else std::cout << "Unable to open " << file_name << " !!\n";
}
