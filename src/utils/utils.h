#ifndef VTS_UTILS_H
#define VTS_UTILS_H

#include <string>
#include <stdexcept>
#include <cassert>
#include <iostream>
#include <ostream>
#include <sstream>
#include <memory>

#include <vector>
// #include <boost/regex.hpp>
#include <boost/algorithm/string.hpp>

#define toDigit(c) (c-'0')

inline std::vector<std::string> get_coordinates ( std::string text, bool splitOnUnder ) {
  std::vector<std::string> results;
  if ( splitOnUnder == false ) {
    boost::split(results, text, [](char c){ return c == '['; });
  } else {
    boost::split(results, text, [](char c){ return c == '_'; });
  }
  return results;
}

inline std::pair<int, int> getxy (std::string var) {
  std::pair<int, int> p2( toDigit(var[2]) , toDigit(var[4]) );
  return p2;
}

class vts_exception : public std::runtime_error
{
public:
  vts_exception(const char* what) : runtime_error(what) {}
  vts_exception(const std::string what) : runtime_error(what.c_str()) {}
};

// everntually remove EXTERNAL_VERSION
// should be NDEBUG == EXTERNAL_VERSION??
// #ifndef NDEBUG
#ifdef EXTERNAL_VERSION
#define triggered_at ""
#else
#define triggered_at " (triggered at " <<  __FILE__ << ", " << __LINE__ << ")"
#endif

#define issue_warning( ss )  { std::cerr << ss.str() << "\n";}

#ifndef NDEBUG
#define issue_error( ss )  { std::cerr << ss.str() << "\n"; assert( false );}
#else
#define issue_error( ss )  { throw vts_exception( ss.str() ); }
#endif


#define vts_error( M, S ) { std::stringstream ss;                   \
    ss << "# vts " << M << " Error - " << S                         \
       << triggered_at << std::endl;                                  \
    issue_error( ss ); }


#define vts_warning( M, S ) { std::stringstream ss;                   \
    ss << "# vts " << M << " Warning - " << S                         \
       << std::endl;                                                    \
    issue_warning( ss );  }


#define bio_synth_error( M, S ) { std::stringstream ss;     \
    ss << "# vts " << M << " Error - " << S                         \
       << triggered_at << std::endl;                                  \
    issue_error( ss ); }


#define bio_synth_warning( M, S ) { std::stringstream ss;                   \
    ss << "# vts " << M << " Warning - " << S                         \
       << std::endl;                                                    \
    issue_warning( ss );  }


#endif // VTS_UTILS_H
