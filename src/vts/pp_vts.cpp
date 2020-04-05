#include <string>
#include <iostream>
#include <fstream>
#include <vts.h>
#include <z3-util.h>
#include <vector>
#include <utility>      // std::pair, std::make_pair
#include <map>
#include <tuple>
#include <algorithm>

#include <boost/algorithm/string.hpp>

#define toDigit(c) (c-'0')


// Printing the Graph
void vts::edge_dump_dot( std::string filename, z3::model mdl ) {
  std::string style = "solid";
  std::string color = "blue";
  std::string node_vec;

  std::cout << "dumping dot file: " << filename << "\n";

  std::ofstream ofs;
  ofs.open ( filename, std::ofstream::out );

  ofs << "digraph prog {\n";
  for( unsigned i = 0 ; i < N; i++ ) {
    node_vec = std::to_string(i) + ":";
    for ( unsigned m = 0; m < M; m++ ) {
      if( is_true( nodes[i][m], mdl ) ) {
        node_vec = node_vec + "1";
      }else if ( is_false ( nodes[i][m], mdl ) ) {
        node_vec = node_vec + "0";
      }else{
        node_vec = node_vec + "*";
      }
      if( is_true( active_node[i][m], mdl) ) {
        node_vec = node_vec + "-";
      }
    }
    ofs << std::to_string(i) << "[label=\"" << node_vec << "\"]\n";

    for( unsigned j = 0 ; j < N; j++ ) {
      if (i == j)
        continue;
      for( unsigned q = 0; q < E_arity; q++ ) {
        if ( q == 0 ) {
          if ( is_true(edges[i][j][q], mdl ) ) {
            style = "solid";
            color = "black";

            std::string label = "m";
            ofs << std::to_string(i) << "-> " << std::to_string(j)
              <<  "[label="  << label << ",color=" << color
              << ",style=" << style << "]\n";
          }
        }

        if ( q == 1 ) {
          if ( is_true( edges[i][j][q], mdl ) ) {
            style = "solid";
            color = "yellow";
            std::string label =  "m" ;
            ofs << std::to_string(i) << "-> " << std::to_string(j)
              <<  "[label="  << label << ",color=" << color
              << ",style=" << style << "]\n";
          }
        }	  
      } // end q
    } // end j
  } // end i

  ofs << "}\n";

} //end function


// Printing the Graph ##########
void vts::dump_dot( std::string filename, z3::model mdl ) {
    std::string style = "solid";
    std::string color = "blue";
    std::string node_vec;

    std::cout << "dumping dot file: " << filename << "\n";

    std::ofstream ofs;
    ofs.open ( filename, std::ofstream::out );

    ofs << "digraph prog {\n";
    for( unsigned i = 0 ; i < N; i++ ) {
      node_vec = std::to_string(i) + ":";
      for ( unsigned m = 0; m < M; m++ ) {
        if( is_true( nodes[i][m], mdl ) ) {
          node_vec = node_vec + "1";
        }else if ( is_false ( nodes[i][m], mdl ) ) {
          node_vec = node_vec + "0";
        }else{
          node_vec = node_vec + "*";
        }
        if( is_true( active_node[i][m], mdl) ) {
          node_vec = node_vec + "-";
        }
      }
      ofs << std::to_string(i) << "[label=\"" << node_vec << "\"]\n";

      for( unsigned j = 0 ; j < N; j++ ) {
        if (i == j)
          continue;
        for( unsigned q = 0; q < E_arity; q++ ) {
       	if ( q == 0 ) {
          for( unsigned m = 0; m < M; m++ ) {
            style = "solid";
            std::string label = std::to_string( m );
            if( is_true( presence_edge[i][j][q][m], mdl ) ) {
              color = "black";
              if( is_true( active_edge[i][j][q][m], mdl ) ) {
                color = "green";
                for( unsigned m1 = 0; m1 < M; m1++ ) {
                  if( is_true( active_node[j][m1], mdl ) and
                      is_true( pairing_m[m][m1], mdl ) ) {
                    color = "red";

                    break;
                  }
                }
              }
              ofs << std::to_string(i) << "-> " << std::to_string(j)
                  <<  "[label="  << label << ",color=" << color
                  << ",style=" << style << "]\n";
            }
          }
        
	  }
    	if ( q == 1 ) {
          for( unsigned m = 0; m < M; m++ ) {
            style = "solid";
            std::string label = std::to_string( m );
            if( is_true( presence_edge[i][j][q][m], mdl ) ) {
            // if( is_true( edges[i][j][q], mdl ) ) {
              color = "yellow";
             if( is_true( active_edge[i][j][q][m], mdl ) ) {
                color = "pink";
                for( unsigned m1 = 0; m1 < M; m1++ ) {
                  if( is_true( active_node[j][m1], mdl ) and
                      is_true( pairing_m[m][m1], mdl ) ) {
                    color = "blue";
                    break;
                  }
                }
              }
              ofs << std::to_string(i) << "-> " << std::to_string(j)
                  <<  "[label="  << label << ",color=" << color
                  << ",style=" << style << "]\n";
            }
          }
        }	  
	  } // end q
      } // end j
    } // end i

    ofs << "}\n";

} //end function

// Print depqbf Graph
void vts::print_graph( z3::context& c, std::string filename, 
                       VecsExpr qs, bool printModel, bool displayGraph, int synthVar ) {
  std::string style = "solid";
  std::string color = "blue";
  std::string node_vec; 

  // denotation_map: print the variable assignment
  std::map<std::string, int> denotation_map;
  std::string depqbfRun;

  // xyPair: edge 
  //std::vector< std::pair <int, int> > xyPair;

  // edge tuple
  Tup3Expr edgeT;
  // presence edge tuple
  Tup4Expr presenceEdgeT;
  // active edge tuple
  Tup4Expr activeEdgeT;
  // active node tuple
  Tup3Expr activeNodeT;
  // presence node tuple
  Tup3Expr nodeT;

  create_map ( c, denotation_map, depqbfRun, nodeT, 
                  activeNodeT, edgeT, presenceEdgeT, activeEdgeT, qs );

  std::cout << depqbfRun << "\n"; 

  //for (auto& var: denotation_map ) {
  //  std::cout << "<" <<  var.first << "," << var.second  << ">" <<"\n" ;
  //}
  //for (auto& var: xyPair ) {
  //  std::cout << "<" <<  var.first << "," << var.second  << ">" <<"\n" ;
  //}
  //}

  if ( printModel == true ) {
    print_denotation_console ( denotation_map, synthVar );
  }

  if ( displayGraph == true ) {
    std::cout << "dumping dot file: " << filename << "\n";

    std::ofstream ofs;
    ofs.open ( filename, std::ofstream::out );

    ofs << "digraph prog {\n";

    unsigned node_id = 0;
    node_vec = std::to_string(0) + ":";

    if ( !activeNodeT.empty() ) { 
      for ( unsigned i = 0; i < activeNodeT.size(); i++ ) {
        unsigned x = std::get<0>( activeNodeT[i] ); 
        //auto y = std::get<1>( activeNodeT[i] ); 
        unsigned z = std::get<2>( activeNodeT[i] ); 

        if ( x != node_id ) {
          ofs << std::to_string(node_id) << "[label=\"" << node_vec << "\"]\n";
          node_vec = std::to_string(x) + ":";
          node_id = x;  
        }

        if ( z == 0 ) {
          if  ( (std::get<2> (nodeT[i]) )  == 0) 
            node_vec = node_vec + "0"; 
          else  
            node_vec = node_vec + "1";
        } else {
          node_vec = node_vec + "1" + "-";
        }
      }
      ofs << std::to_string(node_id) << "[label=\"" << node_vec << "\"]\n";

    } else {
      for( unsigned i = 0 ; i < N; i++ ) {
        node_vec = std::to_string(i);
        ofs << std::to_string(i) << "[label=\"" << node_vec << "\"]\n";
      }
    }

    if ( !activeEdgeT.empty() ) {
      for (auto& i : activeEdgeT ) {
        auto x = std::get<0>( i ); 
        auto y = std::get<1>( i ); 
        auto q = std::get<2>( i ); 
        auto k = std::get<3>( i ); 
        std::cout << "Active molecule: [ " << x << ", " << y << ", " << q << ", " << k << " ]\n";  
      //  std::cout << " Inside active [ " << x << ", " << y <<  ", " << k << " ]\n";  

        if ( q == 0 ) {
          color = "green";
        } else {
          color = "pink";
        }

        std::string label = std::to_string(k);
        //std::cout << "Label is " << label << "\n";

        ofs << std::to_string (x) << "-> " <<  std::to_string (y)
          <<  "[label="  << label << ",color=" << color
          << ",style=" << style << "]\n";

      }
    } 
    if ( !presenceEdgeT.empty() ) {
      for (auto& i : presenceEdgeT ) {
        auto x = std::get<0>( i ); 
        auto y = std::get<1>( i ); 
        auto q = std::get<2>( i ); 
        auto k = std::get<3>( i ); 
        
        if ( std::find( activeEdgeT.begin(), activeEdgeT.end(), i ) != activeEdgeT.end() ) { 
          continue;
        }else {
          if ( q == 0 ) {
            color = "black";
          } else {
            color = "yellow";
          }

          std::string label = std::to_string(k);

          ofs << std::to_string (x) << "-> " <<  std::to_string (y)
            <<  "[label="  << label << ",color=" << color
            << ",style=" << style << "]\n";
        }

      }

    } else {

      //std::cout << "Edge set is : " << edgeT.empty() << "\n";
      if ( !edgeT.empty() ) {
        for (auto& i : edgeT ) {
          auto x = std::get<0>( i ); 
          auto y = std::get<1>( i ); 
          auto z = std::get<2>( i ); 

          //std::cout << z << "\n";

          if ( z == 0 ) {
            color = "red";
          } else {
            color = "blue";
          }

          std::string label = "M";

          ofs << std::to_string (x) << "-> " <<  std::to_string (y)
            <<  "[label="  << label << ",color=" << color
            << ",style=" << style << "]\n";
        }
      }
    }
    ofs << "}\n";
  }

} //end function
