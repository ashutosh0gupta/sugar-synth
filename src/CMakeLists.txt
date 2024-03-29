cmake_minimum_required(VERSION 3.5.1)

project(VTSSYNTH)

SET (LLVM_DEBUG FALSE CACHE BOOL "Use Locally compiled and debug enabled LLVM")
SET (Z3_DEBUG FALSE CACHE BOOL "Link debug verion of Z3")

IF(CMAKE_BUILD_TYPE MATCHES Debug)
    message("debug mode")
    SET (LLVM_DEBUG TRUE)
    SET (Z3_DEBUG TRUE)
ENDIF(CMAKE_BUILD_TYPE MATCHES Debug)

message(STATUS "DEBUG : LLVM - ${LLVM_DEBUG}, Z3 - ${Z3_DEBUG} ")

# IF(LLVM_DEBUG)
#   # SET ( CMAKE_MODULE_PATH "${PROJECT_BINARY_DIR}/../llvm/share/llvm/cmake" )
#   # message(STATUS "Module paths ${CMAKE_MODULE_PATH}")
#   find_package(LLVM REQUIRED CONFIG NO_DEFAULT_PATH HINTS "${PROJECT_BINARY_DIR}/../llvm-${LLVM_VERSION}/lib/cmake/llvm")
# ELSE(LLVM_DEBUG)
#   # STRING(REGEX REPLACE "^(.).*$" "\\1" LLVM_SHORT_VERSION "${LLVM_VERSION}")
#   # MESSAGE(STATUS "LLVM SHORT VERSION: ${LLVM_SHORT_VERSION}")
#   find_package(LLVM ${LLVM_VERSION} REQUIRED CONFIG)
# ENDIF(LLVM_DEBUG)


# message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
# message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

# ADD_DEFINITIONS( "-DLLVM_PACKAGE_VERSION=${LLVM_PACKAGE_VERSION}" )

IF(WIN32)
    SET(Boost_USE_STATIC_LIBS    ON)
    SET(Boost_USE_STATIC_RUNTIME ON)
    foreach(flag_var
        CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE
        CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO)
    if(${flag_var} MATCHES "/MD")
       string(REGEX REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
    endif(${flag_var} MATCHES "/MD")
    endforeach(flag_var)
ELSE(WIN32)
    SET(Boost_USE_STATIC_LIBS    OFF)
    SET(Boost_USE_STATIC_RUNTIME OFF)
ENDIF(WIN32)

set(Boost_USE_MULTITHREADED ON)
find_package(Boost 1.52.0 COMPONENTS regex program_options system filesystem REQUIRED) 

find_package (Threads)

SET (HB_UNICODE TRUE CACHE BOOL "Enable unicode output")

IF(Z3_DEBUG)
  LINK_DIRECTORIES("${PROJECT_BINARY_DIR}/../z3/buildd")
  SET( Z3_COMPILED_HEADERS "${PROJECT_BINARY_DIR}/../z3/buildd/src/")
ELSE(Z3_DEBUG)
  LINK_DIRECTORIES("${PROJECT_BINARY_DIR}/../z3/buildr")
  SET( Z3_COMPILED_HEADERS "${PROJECT_BINARY_DIR}/../z3/buildr/src/")
ENDIF(Z3_DEBUG)

IF(NOT WIN32)
    ADD_DEFINITIONS(-std=c++11 -Wall)
ENDIF(NOT WIN32)

if (HB_UNICODE AND NOT WIN32)
  ADD_DEFINITIONS(-DHB_UNICODE)
ENDIF(HB_UNICODE AND NOT WIN32)

include_directories (${CMAKE_CURRENT_BINARY_DIR} "${PROJECT_SOURCE_DIR}" "${PROJECT_SOURCE_DIR}/solver" "${PROJECT_SOURCE_DIR}/vts"  "${PROJECT_SOURCE_DIR}/library" "${PROJECT_BINARY_DIR}/../z3/src/api" "${PROJECT_BINARY_DIR}/../z3/src/" "${PROJECT_BINARY_DIR}/../z3/src/api/c++" ${Z3_COMPILED_HEADERS})
include_directories(${Boost_INCLUDE_DIRS}) 
LINK_DIRECTORIES(${Boost_LIBRARY_DIRS})

# add_definitions(${LLVM_DEFINITIONS})
# include_directories(${LLVM_INCLUDE_DIRS})

# llvm_map_components_to_libnames(llvm_libs aarch64 aarch64asmparser aarch64asmprinter aarch64codegen aarch64desc aarch64disassembler aarch64info aarch64utils analysis arm armasmparser armasmprinter armcodegen armdesc armdisassembler arminfo asmparser asmprinter bitreader bitwriter codegen core executionengine hexagon hexagoncodegen hexagondesc hexagondisassembler hexagoninfo instcombine instrumentation interpreter ipo irreader lineeditor linker lto mc mcdisassembler mcjit mcparser mips mipsasmparser mipsasmprinter mipscodegen mipsdesc mipsdisassembler mipsinfo msp430 msp430asmprinter msp430codegen msp430desc msp430info native nativecodegen nvptx nvptxasmprinter nvptxcodegen nvptxdesc nvptxinfo objcarcopts object option powerpc powerpcasmparser powerpcasmprinter powerpccodegen powerpcdesc powerpcdisassembler powerpcinfo profiledata runtimedyld scalaropts selectiondag sparc sparcasmparser sparcasmprinter sparccodegen sparcdesc sparcdisassembler sparcinfo support systemz systemzasmparser systemzasmprinter systemzcodegen systemzdesc systemzdisassembler systemzinfo tablegen target transformutils vectorize x86 x86asmparser x86asmprinter x86codegen x86desc x86disassembler x86info x86utils xcore xcoreasmprinter xcorecodegen xcoredesc xcoredisassembler xcoreinfo)
# engine 

# LINK_DIRECTORIES(${LLVM_LIBRARY_DIRS})
# add_subdirectory("library")

# add_executable(vts-synth main.cpp)
#add_executable(vts-synth main.cpp z3-hoister.cpp)
#add_executable( vts-synth main.cpp z3-hoister.cpp card-constr.cpp  normal-form.cpp qdimacs-printer.cpp depqbf-printer.cpp vts.cpp pp_vts.cpp qbf.cpp gates-encoding.cpp kcnf-encoding.cpp graph-connectivity.cpp vts-synthesis.cpp synth-func.cpp ) 

add_executable( vts-synth main.cpp solver/z3-hoister.cpp solver/z3-util.cpp solver/card-constr.cpp  solver/normal-form.cpp solver/qdimacs-printer.cpp solver/depqbf-printer.cpp sugar/sugar.cpp sugar/rule.cpp sugar/sugar-encode.cpp vts/vts.cpp vts/pp_vts.cpp solver/qbf.cpp solver/gates-encoding.cpp vts/kcnf-encoding.cpp vts/graph-connectivity.cpp vts/vts-synthesis.cpp vts/load-vts.cpp  vts/synth-func.cpp ) 

SET(LIBZ3 z3)

target_link_libraries (vts-synth ${LIBZ3} ${Boost_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
