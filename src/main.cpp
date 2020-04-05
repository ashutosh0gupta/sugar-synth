#include <stdio.h>
#include <stdlib.h>
#include <z3++.h>
#include <future>
#include <vector>

#include <unistd.h>
#include <chrono>
#include <ctime>
#include <fstream>
#include <iostream>
//#include <time.h>

#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/token_functions.hpp>
#include <boost/tokenizer.hpp>

using namespace boost;
using namespace boost::program_options;

#include <exception>

#include <load-vts.h>
#include <vts.h>
#include "sugar/sugar.h"
#include "z3-util.h"


int funcType = -1;
int synthVar = -1;
bool useZ3 = false;
bool displayGraph = false;
bool printModel = false;
bool inputFile = false;
std::vector<std::string> inputFilename;
std::string mode = "vts";

int parse_cmdline(int ac, char* av[]) {

  //-----------------------------
  // Setup options
  //-----------------------------

  options_description general("General options");
  general.add_options()("help", "produce a help message")
        //  ("help-module", value<std::string>(),
        //   "produce a help for a given module")
        //  ("version", "output the version number")
        ;

    /*
    options_description gui("GUI options");
    gui.add_options()
      ("display", value<std::string>(), "display to use")
      ;
   */

  options_description variation("Variation options");
  variation.add_options()
    ("func-type,f", value<int>(),"fix a function type for QBF")
    ("synth-var,s", value<int>(), "fix a synthesis variation for vts");

  options_description options("Available options");
  options.add_options()
    ("use-z3,z", "use z3 for QBF solving")
    ("print-model,p", "print vts model")
    ("display-graph,g","display the vts as graph")
    ("input-file,i", value<std::vector<std::string> >(), "input file.");

  options_description sugar_options("sugar synthesis");
  sugar_options.add_options()
    ("sugar-mode,r", "run sugar synthesis");

    // Declare an options description instance which will include
    // all the options
  options_description all("Allowed options");
  // all.add(general).add(gui).add(variation);
  all.add(general).add(variation).add(options).add(sugar_options);

  // Declare an options description instance which will be shown
  // to the user
  options_description visible(
    "VTS-Synth [version 0.0.1]. (C) Copyright 2017-2018 TIFR Mumbai. "
    "\nUsage: ./vts-synth [--options] [--variation arg] \n\nFunction "
    "types:\n  0. Arbitrary Boolean func: ackermannization. [default] \n  "
    "1. K-cnf with depth D. \n  2. Logic-gates AND OR. \n  3. Logic gate "
    "with unique arguments. \n\nSynthesis variation:\n  0. Default. \n  1. "
    "Edge synthesis.\n  2. Molecule synthesis.\n  3. K-Cnf.\n  4. Logic "
    "gates.\n  5. VTS repair.\n  6. Activate-deactivate");
  visible.add(general).add(variation).add(options).add(sugar_options);


    //-----------------------------
    // process commandline options
    //-----------------------------

    variables_map vm;
    store(parse_command_line(ac, av, all), vm);

    positional_options_description p;
    p.add("input-file", -1);

    if (vm.count("help")) {
      std::cout << visible;
      return 1;
    }
    if (vm.count("help-module")) {
      const std::string& s = vm["help-module"].as<std::string>();
      // if (s == "gui") {
      //  std::cout << gui;
      // } else
      if (s == "variation") {
        std::cout << variation;
      } else if (s == "options") {
        std::cout << options;
      } else {
        bio_synth_error( "command options",  "Unknown module '" << s
                  << "' in the --help-module option\n");
      }
      return 1;
    }

    if (vm.count("func-type")) {
      std::cout << "The 'function type' chosen is: "
                << vm["func-type"].as<int>() << "\n";
      funcType = vm["func-type"].as<int>();
    }

    if (vm.count("synth-var")) {
      std::cout << "The 'synthesis variation' chosen is: "
                << vm["synth-var"].as<int>() << "\n";
      synthVar = vm["synth-var"].as<int>();
    }
    if (vm.count("use-z3")) {
      // std::cout << "The 'use-Z3' options was set to "
      //  << vm["use-z3"].as<unsigned>() << "\n";
      useZ3 = true;
    }
    if (vm.count("display-graph")) {
      // std::cout << "The 'display' options was set to "
      //  << vm["display-graph"].as<unsigned>() << "\n";
      displayGraph = true;
    }
    if (vm.count("print-model")) {
      // std::cout << "The 'print model' options was set to "
      //  << vm["print-model"].as<unsigned>() << "\n";
      // printModel =  vm["print-model"].as<unsigned>();
      printModel = true;
    }
    if (vm.count("input-file")) {
      inputFilename = vm["input-file"].as<std::vector<std::string> >();
      std::cout << "Input file is = " << inputFilename[0] << "\n";
      inputFile = true;
    }
    if (vm.count("sugar-mode")) {
      mode = "sugar";
    }
    return 0;
}

int main(int ac, char* av[]) {
  try {

    if( parse_cmdline( ac, av ) ) return 0;

    if( mode == "sugar" ) {
      if( inputFilename.size() > 0 )
        synth_sugar( inputFilename[0] );
      else
        std::cout << "No input file given!\n";
      return 0;
    }

    z3::context c;

    // Record start time
    auto start = std::chrono::high_resolution_clock::now();

    // std::cout << useZ3;
    // std::cout << vm["func-model"].as<int>() << "\n";
    // exit(0);

    vts_ptr v = nullptr;

    if (synthVar != -1) {
      if (inputFile == true) {
        load_vts ld(c, inputFilename[0]);
        ld.load();
        v = ld.get_vts();
      } else {
        std::cout << "Using default file [t.vts] in current folder... \n";
        load_vts ld(c, "./examples/t.vts");
        ld.load();
        v = ld.get_vts();
      }
    } else {
      // vts: v [context, Molecule, Nodes, Edge_arity, Version, Connectivity,
      // Cnf_depth ]
      unsigned int N = 3;
      unsigned int M = 4;
      unsigned int Q = 2;
      // depth of cnf
      unsigned int D = 2;
      // unsigned L = 4;
      v = std::shared_ptr<vts>(new vts(c, M, N, Q, MODEL_4, 3, D));
    }

    // z3::model mdl = v->get_vts_for_prob1();
    // z3::model qbf_mdl = v->get_vts_for_qbf();

    // v->dump_dot("/tmp/vts.dot", mdl );
    // v->dump_dot("/tmp/vts.dot", qbf_mdl );

    // represent z3 vts formula
    z3::expr f(c);

    if (funcType != -1) {
      f = v->create_qbf_formula(funcType);
    } else if (synthVar != -1) {
      f = v->vts_synthesis(synthVar);
    } else {
      // Use SAT based VTS
      auto model_vts = v->get_vts_for_prob1();
      if (printModel == true) {
        std::cout << model_vts << '\n';
      }
      if (displayGraph == true) {
        v->dump_dot("/tmp/bio.dot", model_vts);
        auto retVal = system("xdot /tmp/dep_vts.dot");
        if (retVal == -1) std::cout << "SYSTEM ERROR !!!\n";
      }
      // f = v->create_qbf_formula( 0 );
    }
    // std::cout << f << "\n";

    /* Run Z3 home made QBF solver or DepQbf */
    if (useZ3 == true) {
      if (synthVar != -1) {
        std::cout << "Using z3...\n";
        z3::model m = v->get_vts_for_synth(f);
        if (printModel == true) {
          std::cout << m << '\n';
        }
        if (displayGraph == true) {
          if (synthVar == 1) {
            v->edge_dump_dot("/tmp/vts.dot", m);
          } else {
            v->dump_dot("/tmp/vts.dot", m);
          }
          auto retVal =
              system("dot -Tpng /tmp/vts.dot -o vts.png; open vts.png");

          // auto retVal = system("xdot /tmp/vts.dot");
          if (retVal == -1) std::cout << "SYTEM ERROR !!!\n";
        }
        auto finishZ3 = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsedZ3 = finishZ3 - start;
        std::cout << "entire run took " << elapsedZ3.count() << " secs\n";
        exit(0);
      } else {
        v->use_z3_qbf_solver(f);
      }
    }

    VecsExpr qs;
    std::cout << "Creating Prenexing of the Formula\n";
    auto prenex_f = prenex(f, qs);
    std::cout << "Done Prenexing the Formula\n";

    /* Print the formaula in pcnf  Avoid printing now ! */
    // std::cout << "Prenexed f : " << prenex_f << "\n";
    // std::cout << "Quants :\n";

    /* Avoid printing now !
       for(auto& q : qs ) {
       for( auto& e : q )
       std::cout << e << " ";
       std::cout << "\n";
       }
       */

    auto cnf_f = cnf_converter(prenex_f);

    /* Avoid printing for now!
       std::cout << "CNF body :\n";
       for( auto& cl : cnf_f )
       std::cout << cl << "\n";
       */

    // std::cout << "CNF f : " << cnf_f << "\n";
    std::cout << "Printing qdimacs at /tmp/myfile.qdimacs \n";
    qdimacs_printer(cnf_f, qs);
    // std::cout << "Creating depqbf input file at /tmp/depqbf.c \n";
    // std::cout << "Creating depqbf input file at
    // ./build/depqbf/examples/depqbf.c  \n"; depqbf_file_creator(edgeQuant,
    // equant_len);

    // bool timedout = false;
    std::future<int> future = std::async(std::launch::async, []() {
      // if ( flagA == false ) {
      auto retVal = system(
          "cd ./build/depqbf/examples;timeout 6000s ../depqbf --qdo "
          "--no-dynamic-nenofex  /tmp/myfile.qdimacs > /tmp/out.txt");
      // std::cout << retVal << "\n";
      //  } else {
      // auto retVal = system("cd ./build/depqbf/examples; gcc -o depqbf-file
      // depqbf-file.c -L.. -lqdpll; ./depqbf-file" );
      //    }
      return retVal;
      // system("./src/bash_script.sh");
    });

    // std::cout << "Running depqbf ... " << "\n";
    std::future_status status;

    status = future.wait_for(std::chrono::seconds(6000));

    if (status == std::future_status::timeout) {
      std::cout << "TimeOut! \n";
      // future.join();
      exit(0);
      std::terminate();
      return 1;
    }
    if (status == std::future_status::ready) {
      std::cout << "Program run was sucessful!\n";
      auto finish = std::chrono::high_resolution_clock::now();
      std::chrono::duration<double> elapsed = finish - start;
      std::cout << "entire run took " << elapsed.count() << " secs\n";
    }

    v->print_graph(c, "/tmp/dep_vts.dot", qs, printModel, displayGraph,
                   synthVar);
    if (displayGraph == true) {
      auto retVal = system("dot -Tpng /tmp/vts.dot -o vts.png; open vts.png");
      // auto retVal = system("xdot /tmp/dep_vts.dot");
      if (retVal == -1) std::cout << "SYTEM ERROR !!!\n";
    }

    //  }
    /*
       else {
       std::ifstream myfile ( "/tmp/out.txt" );
       std::string line;
       if ( myfile ) {
       std::string line;
       auto firstLine = std::getline( myfile, line );
    //firstLine[1]  == "1" ? std::cout << "THE FORMULA IS SAT" << "\n" :
    std::cout << "THE FORMULA IS UNSAT" << "\n"; std::cout << firstLine << "\n";
    }
    */

    // std::cout << "\nPrinting depqbf graph at /tmp/dep_vts.dot \n";
    // }

  }

  catch (z3::exception& ex) {
    std::cout << "\n Nuclear Missles are launched ----> " << ex << "\n";
  }
  return 0;
}

