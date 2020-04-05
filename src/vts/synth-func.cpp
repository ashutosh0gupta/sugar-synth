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
#include <utils/utils.h>

//#include <boost/algorithm/string.hpp>

//#define toDigit(c) (c-'0')

void dVar_update ( unsigned m, unsigned M, std::string& depth_var, 
                   unsigned noOfLiterals, unsigned litCount, bool lit, bool gateCase ) {
  std::string var;
  if ( lit == true ) 
    var = "arg";
  else 
    var = "!arg";
    
  //if ( m == M-1 || gateCase == true ) {
  if ( litCount == noOfLiterals || gateCase == true ) {
    depth_var = depth_var + var + std::to_string (m);
  }else { 
    depth_var = depth_var + var + std::to_string (m) + " || ";
  }
}

void chosen_var ( unsigned candidateVar, std::vector <int> & func_arg, 
                  std::vector<std::string>& chosenVars ) {
  for ( unsigned v = 0; v < candidateVar; v++ ) {
    int var = func_arg[v]; 
    if ( var == 1 ) {
        if ( v == candidateVar - 2 ){
          chosenVars.push_back( "true" );
        } else if ( v == candidateVar - 1 ) {
          chosenVars.push_back( "false" );
        } else {
          auto pickedVar = "arg" + std::to_string (v);
          chosenVars.push_back( pickedVar );
        }
    } else if( var == -1 ) {
        auto pickedVar = "!arg" + std::to_string (v);
        chosenVars.push_back( pickedVar );
    }else {
        continue;
    }
  }
  //std::cout << "The size of Candiate var is: " << chosenVars.size() << "\n";
}

void chosen_gates ( std::vector <int>& wVarStr, 
                    std::vector<std::string>& chosenGates ) {
  if ( wVarStr[0] == 1 ) {
    chosenGates.push_back( "AND" );
  } else { 
    chosenGates.push_back( "OR" );
  }
}  

void print_learned_function ( std::vector<std::string>& chosenVars, 
                              std::vector<std::string> chosenGates, 
                              unsigned m, bool edgeVar ) { 
  unsigned local_leaf_num = 4;
  unsigned gate_counter = 0;

  while( local_leaf_num > 1 ) {
    for( unsigned l = 0; l < local_leaf_num; l = l + 2 ) {
      if( l == local_leaf_num - 1 ) {
         chosenVars[l>>1] = chosenVars[l];
      }else{
         chosenVars[l>>1] =  "(" + chosenVars[l] + ' ' + 
                             chosenGates[gate_counter] + 
                             ' ' + chosenVars[l+1] + ")";
         gate_counter++;
      }
     }
    local_leaf_num = (local_leaf_num+1) >> 1;
  }
//  assert( local_leaf_num == 1);
  if ( edgeVar == true ) {
    std::cout << "b_" << std::to_string(m) << " = "  <<  chosenVars[0] << " \n";
  } else {
    std::cout << "a_" << std::to_string(m) << " = "  <<  chosenVars[0] << "\n";
  }
}

void print_func_gates ( unsigned noOfMolecules, unsigned noOfLeaves, unsigned candidateVar, 
                        std::vector < std::vector< std::vector <int> > > func_arg, 
                        std::vector < std::vector< std::vector <int> > > uVarStr, bool e ) {    
  unsigned noOfGates = 3;

  for ( unsigned m = 0; m < noOfMolecules; m ++ )  {
    std::vector<std::string> chosenVars; 
    std::vector<std::string> chosenGates; 
    std::string fVar;
    std::string fGate;
    std::string f_val;
    
    for ( unsigned l = 0; l < noOfLeaves; l++ ) {
      chosen_var ( candidateVar, func_arg[m][l], chosenVars );
    } 
    
    for (unsigned g = 0; g < noOfGates; g++ ) {
      chosen_gates ( uVarStr[m][g], chosenGates );
    }
    
    //std::cout << "The size of chosenGate is " << chosenGates.size() << "\n";
    //std::cout << "The number of leafs are  " << noOfLeaves << "\n";
    
    assert( chosenVars.size() == noOfLeaves );
    //assert( chosenGates.size() == noOfGates );

    print_learned_function( chosenVars, chosenGates, m, e );
    
    /*
    for ( unsigned i = 0; i < chosenVars.size(); i++ ) {
      fVar = fVar + chosenVars[i] + " ";
    }
    
    for ( unsigned i = 0; i < chosenGates.size(); i++ ) {
      fGate = fGate + chosenGates[i] + " ";
    }

     f_val =  "[" + fVar + "]" + " [ " +  fGate + " ] ";
           
     if ( e != true ) 
        std::cout <<  "a_fun" + std::to_string (m) << " = " << "func ( " + f_val + " ) \n";
     else 
        std::cout <<  "e_fun" + std::to_string (m) << " = " << "func ( " + f_val + " ) \n";
  */
  }
} 


void print_func_cnf ( unsigned M, int D, std::vector < std::vector< std::vector <int> > > func_arg, bool e ) {

 // Printing: todo: check if the sVar needs [M, 2M, D] or [M, D, 2M] 
  for ( unsigned k = 0; k < M; k ++ )  {
    std::string func_val;
    for ( int d = 0; d < D; d ++ ) {
      std::string depth_var;
      unsigned noOfLiterals = 1; 
      unsigned litCount = 0;
      for ( unsigned k1 = 0; k1 < M; k1++ ) {
        if ( k1 == k ) continue;
        auto var = func_arg[k][d][k1]; 
        if ( var == 1 ) { 
          litCount += 1;
          //std::cout << "Literal count is: " << litCount << " noOfLiterals " << noOfLiterals <<'\n'; 
          dVar_update ( k1, M, depth_var, noOfLiterals, litCount, true, false );
        } else if ( var == -1 ) { 
          litCount += 1;
          //std::cout << "Literal count is: " << litCount << " noOfLiterals " << noOfLiterals <<'\n'; 
          dVar_update ( k1, M, depth_var, noOfLiterals, litCount, false, false );
        }
      }
      if ( d == D-1 ) 
        func_val = func_val + "[" + depth_var + "]";
      else 
        func_val = func_val + "[" + depth_var + "] && ";
    }
    if ( e != true ) {
     std::cout <<  "a_fun" + std::to_string (k) << " = " << "func ( " + func_val + " ) \n";
    } else {
     std::cout <<  "e_fun" + std::to_string (k) << " = " << "func ( " + func_val + " ) \n";
    } 
  }
}

void final_map ( unsigned noOfMolecules, unsigned arg2, unsigned arg3,
                 std::vector < std::vector< std::vector <int> > >& tVarStr,
                 std::vector < std::vector< std::vector <int> > >& func_arg,  
                 bool e , int synthVar ) {
  // [2M -> M] conversion. Now: mapping [M,M,D]
  // Take care of neg case of a var 
  for ( unsigned i = 0; i < noOfMolecules; i++ ) {
    for ( unsigned j = 0; j < arg2; j++ ) { 
      for ( unsigned k = 0; k < arg3; k++ ) { 
        if ( i == k ) continue; 
        //if ( synthVar == 3 && i == k ) continue; 
        // Case for true and false 2M ++:
        if ( synthVar == 4 && k >= noOfMolecules ) {
          if ( tVarStr[i][j][k+noOfMolecules] == 1) {
            func_arg[i][j][k] = 1;
            continue;
          } else {
            func_arg[i][j][k] = 0;
            continue;
          }
        } 
        if ( tVarStr[i][j][k] == 1 ) { 
            func_arg[i][j][k] = 1;
        } else if ( tVarStr[i][j][k+noOfMolecules] == 1 ) {  
            func_arg[i][j][k] = -1;
        }else {
            func_arg[i][j][k] = 0;
        }
      }
    }
  }
}

/*
void model_map_2 ( std::vector < std::vector <int> >& tVarStr, std::string fst, int snd ) {
  auto coord = get_coordinates( fst );
  
  auto func_mol = std::stoi( coord[1] );
  auto dept_mol = std::stoi( coord[2] );
    if ( snd > 0 ) {
      tVarStr[func_mol][dept_mol] = 1;
    } else {
    // Maping from depqbf model to new variables.
      tVarStr[func_mol][dept_mol] = -1;
    }
}
*/

void model_map_3 ( std::vector < std::vector< std::vector <int> > >& wVarStr, 
                   std::string fst, int snd, unsigned synthVar, bool gateCase ) { 
  auto coord = get_coordinates( fst, true );
  //std::cout << "The passed variable is : " << fst << "\n";
  auto func_mol = std::stoi( coord[1] );
  auto depth_id = std::stoi( coord[2] );
  auto dept_mol = std::stoi( coord[3] );
  
  //std::cout << "[" << func_mol << ", " << dept_mol << ", " << depth_id << "] -> " <<  snd << "\n\n"; 
  if ( snd > 0 ) {
    if ( gateCase == true || func_mol != dept_mol ) {
      //std::cout << "The true var " << fst << "\n";
      //std::cout << "m: " << func_mol << '\n';
      //std::cout << "m1: " << dept_mol << '\n'; 
      //std::cout << "d: " << depth_id << '\n';
      //std::cout << "Value is " <<  snd << "\n"; 
      wVarStr[func_mol][depth_id][dept_mol] = 1; 
    }
  } else {
    wVarStr[func_mol][depth_id][dept_mol] = 0;
  }
}
 
void vts::print_denotation_console ( std::map<std::string,int> denotation_map, int synthVar ) {
     
    //unsigned typeOfGates = 2;
    unsigned binaryGateChoice = 1;
    unsigned noOfLeaves = 4;
    unsigned noOfGates = 3;

    std::vector < std::vector< std::vector <int> > > sVarStr( M, 
                  std::vector< std::vector <int> > ( D, std::vector<int>( 2*M ) ) );
    std::vector < std::vector< std::vector <int> > > tVarStr( M, 
                  std::vector< std::vector <int> > ( D, std::vector<int>( 2*M ) ) );  

    std::vector < std::vector< std::vector <int> > > sGVarStr( M, 
                  std::vector< std::vector <int> > ( noOfLeaves, std::vector<int>( (2 * M) + 2 ) ) );
    std::vector < std::vector< std::vector <int> > > tGVarStr( M, 
                  std::vector< std::vector <int> > ( noOfLeaves, std::vector<int>( (2 * M ) + 2 ) ) ); 
    std::vector < std::vector< std::vector <int> > > uVarStr ( M, 
                  std::vector< std::vector <int> > ( noOfGates, std::vector<int>( binaryGateChoice ) ) );
    std::vector < std::vector< std::vector <int> > > vVarStr ( M, 
                  std::vector< std::vector <int> > ( noOfGates, std::vector<int>( binaryGateChoice ) ) );
    
    /** Map model of depqbf to back to var. todo; get the model directly:*/
    for ( auto& dm : denotation_map ) {
      //std::cout << " dm is : [" << dm.first << ", " << dm.second << "]\n\n";
      std::string val;  
      dm.second > 0 ? val = "True" : val = "False";
      
      /*
      if ( dm.second > 0 ) {
        std::cout << "True var is "  << dm.first << "\n";  
      }
      */
      std::cout << dm.first << " = " << val << "\n";  

      // Map the depqbf output model to created vars
      if ( synthVar == 3 ) {
        if ( dm.first[0] == 't' ) {
          model_map_3 ( tVarStr, dm.first, dm.second, synthVar, false );
        } else if ( dm.first[0] == 's' ) {
          model_map_3 ( sVarStr, dm.first, dm.second, synthVar, false );
        } 
      }
      
      // In case of synth Var 4 you have to create model-mapping for additioanal var u, v
      if ( synthVar == 4 ) {
        if ( dm.first[0] == 't' ) {
          model_map_3 ( tGVarStr, dm.first, dm.second, synthVar, false );
        } else if ( dm.first[0] == 's' ) {
          model_map_3 ( sGVarStr, dm.first, dm.second, synthVar, false );
        } else if ( dm.first[0] == 'u' ) {
          model_map_3 ( uVarStr, dm.first, dm.second, synthVar, true );
        } else if ( dm.first[0] == 'v' ) {
          model_map_3 ( vVarStr, dm.first, dm.second, synthVar, true );
        }
      }
   }
   
    if ( synthVar == 3 ) {
      
      std::vector < std::vector< std::vector <int> > > func_para_n ( M, std::vector< std::vector <int> > ( D, std::vector<int>( M ) ) );
      //std::vector < std::vector< std::vector <int> > > func_para_n ( M, std::vector< std::vector <int> > ( M, std::vector<int>( D ) ) );
      final_map ( M, D, M, sVarStr, func_para_n, false, synthVar );
      //final_map ( M, M, D, sVarStr, func_para_n, false, synthVar );
      print_func_cnf ( M, D, func_para_n, false );
      
      std::vector < std::vector< std::vector <int> > > func_para_e ( M, std::vector< std::vector <int> > ( D, std::vector<int>( M ) ) );
      //std::vector < std::vector< std::vector <int> > > func_para_e ( M, std::vector< std::vector <int> > ( M, std::vector<int>( D ) ) );
      final_map ( M, D, M, tVarStr, func_para_e, true, synthVar );
      //final_map ( M, M, D, tVarStr, func_para_e, true, synthVar );
      print_func_cnf ( M, D, func_para_e, true );
    }
    else if ( synthVar == 4 ){

      std::vector < std::vector< std::vector <int> > > func_para_n ( M, 
                    std::vector< std::vector <int> > ( noOfLeaves, std::vector<int>( M + 2 ) ) );
      final_map ( M, noOfLeaves, M+2, sGVarStr, func_para_n, false, synthVar );
      print_func_gates ( M, noOfLeaves, M+2, func_para_n, uVarStr, false );
      //print_func_gates ( M, noOfLeaves, M+2, func_para_n, sGVarStr, uVarStr, false );
      
      std::vector < std::vector< std::vector <int> > > func_para_e ( M, 
                    std::vector< std::vector <int> > ( noOfLeaves, std::vector<int>( M + 2 ) ) );
      final_map ( M, noOfLeaves, M+2, tGVarStr, func_para_e, true, synthVar );
      print_func_gates ( M, noOfLeaves, M+2, func_para_e, vVarStr, true );
      //print_func_gates ( M, noOfLeaves, M+2, func_para_e, tGVarStr, vVarStr, true );
    }
}

void vts::create_map ( z3::context& c, std::map<std::string,int>& denotation_map, 
                       std::string& depqbfRun,  Tup3Expr& nodeT, Tup3Expr& activeNodeT, Tup3Expr& edgeT, 
                       Tup4Expr& presenceEdgeT, Tup4Expr& activeEdgeT, VecsExpr qs  ) {
                         
  unsigned int step = 0;
  std::ifstream myfile ( "/tmp/out.txt" );
  std::string line;

  // Gives list of first level existential quantifier
  auto firstLvlQuant = qs[0];

  /*
     for ( auto& i : firstLvlQuant ) {
     std::cout << i << "\n";
     }
  */

  if ( myfile ) {
    while (std::getline( myfile, line )) {
      //std::stringstream  stream(line);
      //std::string  word;
      if (step == 0) { 
        int lit = toDigit ( line.at(6) ); 
        if ( lit == 0 ) {
          depqbfRun = "Formula is UNSAT";
          std::cout << depqbfRun << "\n";
          exit(0);
          break;
        }else {
          depqbfRun = "Formula is SAT";
        }
        step += 1;
        continue;
      } else {
        std::stringstream  stream(line);
        std::string  word;
        stream >> word; stream >> word;
        int lit = std::stoi( word );
        
        // The output of Depqbf run giving model to variables. 
        //std::cout << "LINE IS : "  << lit << "\n";
        //std::cout << "AT line : " << line.at(1) << "\n";
 
        // OuterMost Variable in first level quantifier.
        std::string var = Z3_ast_to_string  ( c, firstLvlQuant [step - 1] );
        //std ::cout << "Var is " << var << "\n";

        denotation_map [ var ] = lit; 
        //std::cout << "Denotation map is " << denotation_map[ var ] << "\n";

        //std::cout << "Var2 = " << toDigit(var[2]) << "\t Var4 = " << toDigit( var[4] ) << "\n";
        // Variable is e and denotation(e) is True.

        unsigned x, y, k, q;
        if ( var[0] == 'n' && var[1] == '_' ) {
          unsigned active = 0;
          if (lit > 0) {
            active = 1; 
          } 
          nodeT.emplace_back( toDigit( var[2] ), toDigit( var[4] ), active );
        }

        else if ( var[0] == 'a' && var[1] == '_' ) {
          unsigned active = 0;
          if (lit > 0) {
            active = 1; 
          } 
          activeNodeT.emplace_back( toDigit( var[2] ), toDigit( var[4] ), active );
        }

        else if ( var[0] == 'z' && var[1] == '_' && lit > 0 ) {
          edgeT.emplace_back( toDigit(var[2]), toDigit(var[4]), toDigit(var[6]) );
        }

        else if ( var[0] == 'e' && var[1] == '_' && lit > 0 ) {
          auto coord = get_coordinates( var, true );
          x = std::stoi( coord[1] );
          y = std::stoi( coord[2] );
          q = std::stoi( coord[3] );
          k = std::stoi( coord[4] );
          presenceEdgeT.emplace_back( x, y, q, k );
        }

        else if ( var[0] == 'b' && var[1] == '_' && lit > 0 ) {
          auto coord = get_coordinates( var, true );
          x = std::stoi( coord[1] );
          y = std::stoi( coord[2] );
          q = std::stoi( coord[3] );
          k = std::stoi( coord[4] );
          activeEdgeT.emplace_back( x, y, q, k );
        }

        step += 1;
      }
    }

    myfile.close();
  }else {
    std::cout << "No file named out.txt in temp folder... \n" << std::endl;
  }
  myfile.close();
}
