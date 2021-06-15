#ifndef PARSER_HH
#define PARSER_HH
#include "nalnl2wrapper.hh"
#include <getopt.h>
#include <vector>
#include <string>
#include <sstream>
#include <iostream>

/// Command line options that this NAL NL2 wrapper understands
constexpr option long_options[] = {
    {"ac", required_argument, nullptr, 'a'},
    {"bc", required_argument, nullptr, 'b'},
    {"level", required_argument, nullptr, 'c'},
    {"date_of_birth", required_argument, nullptr, 'd'},
    {"gender", required_argument, nullptr, 'e'},
    {"tonal", required_argument, nullptr, 'f'},
    {"experience", required_argument, nullptr, 'g'},
    {"adultChild", required_argument, nullptr, 'h'},
    {"compSpeed", required_argument, nullptr, 'i'},
    {"bilateral", required_argument, nullptr, 'j'},
    {"mic", required_argument, nullptr, 'k'},
    {"direction", required_argument, nullptr, 'l'},
    {"limiting", required_argument, nullptr, 'm'},
    {"channels", required_argument, nullptr, 'n'},
    {"wbct", required_argument, nullptr, 'o'},
    {"bandwidth", required_argument, nullptr, 'p'},
    //{"selection", required_argument, nullptr, 'q'}, ! Disabled, we only support REIG
    {"hatype", required_argument, nullptr, 'r'},
    //{"targettype", required_argument, nullptr, 's'}, ! Disabled, we only support REIG
    {"tubing", required_argument, nullptr, 't'},
    {"venting", required_argument, nullptr, 'u'},
    {"userecd", required_argument, nullptr, 'v'},
    {"ac_other", required_argument, nullptr, 'w'},
    {"crossover", required_argument, nullptr, 'C'}
    };



template <typename T>
std::vector<T> parse_impl(const std::string& str){
  std::stringstream stream(str);
  std::vector<T> res;
  std::string token;
  while(std::getline(stream, token, ',')) {
    std::istringstream ss(token);
    T val;
    ss >> val;
    res.push_back(val);
  }
  return res;
}

template<typename T>
std::vector<T> parseVector(const std::string& str, int size, int option_index){
  auto res=parse_impl<T>(str);
  if(size != -1 && res.size()!=size)
    throw std::invalid_argument("Invalid number of elements in " +
                                std::string(long_options[option_index].name) +
                                ": Expected: "+
                                std::to_string(size)+
                                ". Got: "+
                                std::to_string(res.size()));
  return res;
}

template<typename T> T parseSingle(const std::string& str, int option_index){
  auto res=parse_impl<T>(str);
  if(res.size()!=1)
    throw std::invalid_argument("Invalid number of elements in " +
                                std::string(long_options[option_index].name) +
                                ": Expected: 1. Got: " +
                                std::to_string(res.size()));
  return res[0];
}

/// Fill Client and Fitmodel structs with data parsed from command line
std::pair<Client,Fitmodel> parser(int argc, char** argv);

#endif // PARSER_HH
