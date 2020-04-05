#include <fstream>      
#include <iostream>
#include <sstream>
#include <string>
#include "z3-util.h"



// Adds all the var to the depqbf file. 
void add_vars( std::string s, std::ofstream& ofs) {
  std::stringstream iss(s);
  do 
  {
    std::string subs;
    iss >> subs;
    ofs << "qdpll_add (depqbf," << subs << ");\n"; 
    if ( subs == "0" ) {
      ofs << "\n";
      break;
    }
  } while (iss);
}

void depqbf_file_creator(VecExpr& edgeQuant, unsigned int equant_len) {

  // Begin the printing basic structure
  std::ofstream ofs;
  //ofs.open( "/tmp/depqbf.c", std::ofstream::out );
  ofs.open( "./build/depqbf/examples/depqbf-file.c", std::ofstream::out );

  ofs << "#include <string.h>\n";
  ofs << "#include <stdlib.h>\n";
  ofs << "#include <stdio.h>\n";
  ofs << "#include <assert.h>\n";
  ofs << "#include \"../qdpll.h\"\n\n";
  //ofs << "#include <map>"

  ofs << "int main (int argc, char** argv) \n";
  ofs << "{ \n";

  ofs << "unsigned int interpretation[" << equant_len << "];\n";
  ofs << "unsigned int index = 0; \n"; 
  ofs << "int num;\n";
  ofs << "FILE *fptr;\n";
  ofs << "fptr = fopen ( \"/tmp/out.txt\",\"w\");\n";
  ofs << "if(fptr == NULL) { \n";
  ofs << "  printf(\"Error!\");\n";   
  ofs << "  exit(1);\n } \n";             
  //ofs << "std::map<VecExpr,bool> interpretation;"; 
  ofs << "/* Create solver instance. */\n";
  ofs << "QDPLL *depqbf = qdpll_create ();\n";

  ofs << "/* Use the linear ordering of the quantifier prefix. */\n";
  ofs << "qdpll_configure (depqbf, \"--dep-man=simple\");\n";
  ofs << "/* Enable incremental solving. */\n";
  ofs << "qdpll_configure (depqbf, \"--incremental-use\");\n\n";

  ofs << "/* Enable solution. */\n";
  ofs << "qdpll_configure (depqbf, \"--qdo\");\n\n";


  //Open the qdimacs file.
  //std::ifstream infile("/tmp/myfile.qdimacs");
  std::ifstream file( "/tmp/myfile.qdimacs" );
  unsigned int qid = 1;
  unsigned int oquant = 0;
  std::string line;
  std::string outquantvar;
  while( std::getline( file , line ) ) 
  {
    std::string s1 = line.substr(0, line.find(' '));
    //std::cout << "The valuse of s1 is : " << s1 << "\n";
    if ( s1 == "c" || s1 == "p" ) {
      continue;
    }
    if ( s1 == "e" || s1 == "a" ) {
      std::string quant;
      if ( s1 == "e") {
        qid == 1 ? oquant = 5 : true;
        ofs << "// Add a new leftmost existential quantifier at nested level " << qid << "\n"; 
        ofs << "qdpll_new_scope_at_nesting (depqbf, QDPLL_QTYPE_EXISTS," << qid << ");\n";
        quant = "existential";
      }
      else {
        ofs << "// Add a new leftmost universal quantifier at nested level " << qid << "\n"; 
        ofs << "qdpll_new_scope_at_nesting (depqbf, QDPLL_QTYPE_FORALL," << qid << "); \n";
        quant = "universal";
      }
      std::string::size_type n = 0;
      //n = line.find_first_not_of( " \t", n );
      n = line.find_first_of( " \t", n );
      line.erase( 0,  line.find_first_not_of( " \t", n ) );
      //std::cout << " The edited tail(line) = [ "  << line << "] \n";
      qid == 1 ? outquantvar = line : "true";
      ofs << "/* Add fresh variables to " + quant + " quantifier. \n"; 
      ofs << "\t \t" << line << " */ \n";
      add_vars(line, ofs);
      qid += 1;
    }
    else {
      // We don't care abt outtermost quant variabl...
      ofs << "// Add clause: " << line << "\n"; 
      add_vars(line, ofs);
    }
  }
  // Print "print formula"
 // ofs <<  "/* Print formula. */ \n" <<  "qdpll_print (depqbf, stdout);\n";
  ofs << "QDPLLResult res = qdpll_sat (depqbf);\n";
  ofs << "/* Expecting that the formula is satisfiable. */\n";
  ofs << "assert (res == QDPLL_RESULT_SAT);\n";
  ofs << "/* res == 10 means satisfiable, res == 20 means unsatisfiable. */\n";

  ofs << "\nprintf (\"Result is [10 SAT, 20 UNSAT] : %d\", res);\n";
  ofs << "printf (\"\\n\");\n";
  //std::cout << oquant << "\n";
  // Get a countermodel
  if ( oquant == 5 ) {
    // give the assignments to the variables.
    std::stringstream s(outquantvar);
    unsigned int word = 0;
    std::string var;

    ofs << "\n // Printing the assignments \n";
    
    for ( auto& var : edgeQuant ) {
      word = word + 1;
    //for (int i = 0; s >> word; i++) {
    //  if (word == "0") {
     //   break;
      //}
      //var = edgeQuant[i];
      //var = "a" + std::to_string(i);
      ofs << "QDPLLAssignment " << var << " = qdpll_get_value (depqbf," << word <<");\n";
      ofs << "if ( " << var << " == QDPLL_ASSIGNMENT_TRUE )  { \n" << "printf (\"Value of " << var << " == True \\n\"); \n" << "fprintf(fptr,\"%d \\n\", 1); \n  } \n";    
        //"interpretation[ index ] == 1;" << " \n index = index + 1; \n 

      ofs << "else if ( " << var << " == QDPLL_ASSIGNMENT_FALSE )  { \n" << "printf (\"Value of " << var << " == False \\n\"); \n" << "fprintf(fptr,\"%d \\n\", 0); \n } \n";    
        // "interpretation[ index ] == 0;" << " \n index = index + 1; \n 
      
      ofs << "else { printf(\"Undef \\n\"); \n" << "fprintf(fptr,\"%d \\n\", 0); }\n";
       // "interpretation[ index ] == 0;" << " \n index = index + 1;\n 

//      ofs << "printf (\"Value of " << var << " = %s\\n\", " << var << " == QDPLL_ASSIGNMENT_UNDEF ? \"undef\" : " << " (" << var << " == QDPLL_ASSIGNMENT_FALSE ? \"false\" : \"true\")); \n\n";
    }
    ofs << "printf(\"\\n\"); \n";
    ofs << "fclose(fptr);\n";
    std::cout << "\nRunning DepQbf wait ... \n\n"; 
   //std::cout << "First Level existential variables are: " << outquantvar << "\n";
  }
  else {
    std::cout << "Sorry! OuterMost Quantifier is Not Exists. No Assignments.\n";
  }

  // Delete Solver and end main
  ofs << "/* Delete solver instance. */\n";
  ofs << "qdpll_delete (depqbf);\n";
  ofs << "\n} //End main";

  ofs.close();
}

//void run_command () {
// eval $printf("Hi")
//}
