#include "parser.hh"
#include "nalnl2wrapper.hh"
#include <stdio.h>
#include <stdlib.h>
#include <stdexcept>
#include <utility>

std::pair<Client,Fitmodel> parser(int argc, char **argv) {

  Client client;
  Fitmodel fitmodel;

  int option_index = 0;
  int c;
  while(1){
    c = getopt_long (argc, argv, "",
                     long_options, &option_index);

    /* Detect the end of the options. */
    if (c == -1)
      break;

    switch (c)
      {
      case 'a': // --ac=...
        {
          auto ac=parseVector<double>(optarg,9,option_index);
          client.ac=ac;
        }
        break;
      case 'b': // --bc=...
        {
          auto bc=parseVector<double>(optarg,9, option_index);
          if(bc.size()!=9U)
            throw std::invalid_argument("Invalid number of elements in bc: Expected: 9. Got: "+std::to_string(bc.size()));
          client.bc=bc;
        }
        break;
      case 'c': // --level=...
        {
          auto lvl=parseVector<double>(optarg, -1, option_index);
          fitmodel.level=lvl;
        }
        break;
      case 'd': // --date_of_birth=...
        {
          auto date_of_birth=client.dateOfBirth=parseSingle<int>(optarg,option_index);
        }
        break;
      case 'e': // --gender=...
        {
          auto gender=parseSingle<int>(optarg,option_index);
          if(gender!=0 and gender!=1 and gender!=2)
            throw std::invalid_argument("Invalid gender: Expected: 0 (unknown) or 1 (male) or 2 (female). Got: "+std::to_string(gender));
        client.gender=gender;
        }
        break;
      case 'f': // --tonal=...
        {
          auto tonal=parseSingle<int>(optarg, option_index);
          if(tonal!=0 and tonal!=1)
            throw std::invalid_argument("Invalid tonal: Expected: 0 (non-tonal) or 1 (tonal). Got: "+std::to_string(tonal));
          client.tonal=tonal;
        }
        break;
      case 'g': // --experience=...
        {
          auto exp=parseSingle<int>(optarg,option_index);
          if(exp!=0 and exp!=1)
            throw std::invalid_argument("Invalid experience: Expected: 0 (non-experienced) or 1 (experienced). Got: "+std::to_string(exp));
          client.experience=exp;
        }
        break;
      case 'h': // adultChild=...
        {
          auto adultchild=parseSingle<int>(optarg,option_index);
          if(adultchild!=0 and adultchild!=1 and adultchild!=2)
            throw std::invalid_argument("Invalid adult child: Expected: 0 (calculate from dob) or 1 (adult) or 2 (child). Got: "+std::to_string(adultchild));
          client.adultChild=adultchild;
        }
        break;
      case 'i': // --compSpeed=...
        {
          auto arg=parseSingle<int>(optarg,option_index);
          if(arg!=0 and arg!=1)
            throw std::invalid_argument("Invalid compSpeed: Expected: 0, 1, or 2. Got: "+std::to_string(arg));
          fitmodel.compSpeed=arg;
        }
        break;
      case 'j': // --bilateral=...
        {
          auto arg=parseSingle<int>(optarg,option_index);
          if(arg!=0 and arg!=1)
            throw std::invalid_argument("Invalid bilateral: Expected: 0 or 1. Got: "+std::to_string(arg));
          fitmodel.bilateral=arg;
        }
        break;
      case 'k': // --mic=...
        {
          auto arg=parseSingle<int>(optarg,option_index);
          if(arg!=0 and arg!=1)
            throw std::invalid_argument("Invalid mic: Expected: 0 or 1. Got: "+std::to_string(arg));
          fitmodel.mic=arg;
        }
        break;
      case 'l': // --direction=...
        {
          auto arg=parseSingle<int>(optarg,option_index);
          if(arg!=0 and arg!=1)
            throw std::invalid_argument("Invalid direction: Expected: 0 or 1. Got: "+std::to_string(arg));
          fitmodel.direction=arg;
        }
        break;
      case 'm': // --limitimg=...
        {
          auto arg=parseSingle<int>(optarg,option_index);
          if(arg!=0 and arg!=1 and arg!=2)
            throw std::invalid_argument("Invalid limiting: Expected: 0-2. Got: "+std::to_string(arg));
          fitmodel.limiting=arg;
        }
        break;
      case 'n': // --channels=...
        {
          auto arg=parseSingle<int>(optarg,option_index);
          if(arg>18 or arg<1)
            throw std::invalid_argument("Invalid channels: Expected: 1-18. Got: "+std::to_string(arg));
          fitmodel.channels=arg;
        }
        break;
      case 'o': // --wbct=...
        {
          auto arg=parseSingle<int>(optarg,option_index);
          if(arg<20 or arg>100)
            throw std::invalid_argument("Invalid bilateral: Expected: 20-100. Got: "+std::to_string(arg));
          fitmodel.WBCT=arg;
        }
        break;
      case 'p': // --bandwidth=
        {
          auto arg=parseSingle<int>(optarg,option_index);
          if(arg!=0 and arg!=1)
            throw std::invalid_argument("Invalid bilateral: Expected: 0 or 1. Got: "+std::to_string(arg));
          fitmodel.bandwidth=arg;
        }
        break;
      case 'q': // Disabled!
        {
          auto arg=parseSingle<int>(optarg,option_index);
          if(arg<0 or arg>3 )
            throw std::invalid_argument("Invalid" + std::string(long_options[option_index].name)+ ": Expected: 0-3. Got: "+std::to_string(arg));
          fitmodel.selection=arg;
        }
        break;
      case 'r': // --hatype=
        {
          auto arg=parseSingle<int>(optarg,option_index);
          if(arg<0 or arg>3 )
            throw std::invalid_argument("Invalid" + std::string(long_options[option_index].name)+ ": Expected: 0-3. Got: "+std::to_string(arg));
          fitmodel.haType=arg;
        }
        break;
      case 's': // Disabled!
        {
          auto arg=parseSingle<int>(optarg,option_index);
          if(arg<0 or arg>1 )
            throw std::invalid_argument("Invalid" + std::string(long_options[option_index].name)+ ": Expected: 0-1. Got: "+std::to_string(arg));
          fitmodel.targetType=arg;
        }
        break;
      case 't': // --tubing=...
        {
          auto arg=parseSingle<int>(optarg,option_index);
          if(arg<0 or arg>5 )
            throw std::invalid_argument("Invalid" + std::string(long_options[option_index].name)+ ": Expected: 0-5. Got: "+std::to_string(arg));
          fitmodel.tubing=arg;
        }
        break;
      case 'u': // --venting=...
        {
          auto arg=parseSingle<int>(optarg,option_index);
          if(arg<0 or arg>6 )
            throw std::invalid_argument("Invalid" + std::string(long_options[option_index].name)+ ": Expected: 0-6. Got: "+std::to_string(arg));
          fitmodel.venting=arg;
        }
        break;
      case 'v': // --userecd=...
        {
          auto arg=parseSingle<int>(optarg,option_index);
          if(arg<0 or arg>1 )
            throw std::invalid_argument("Invalid" + std::string(long_options[option_index].name)+ ": Expected: 0. Got: "+std::to_string(arg));
          fitmodel.useRECDh=arg;
        }
        break;
      case 'w': // --ac_other=...
        {
          auto arg=parseVector<double>(optarg,9,option_index);
          client.acOther=arg;
        }
        break;
      case 'C': // --crossover=...
        {
          auto arg=parseVector<double>(optarg,-1,option_index);
          fitmodel.CFArray=arg;
        }
        break;
      case '?':
        /* getopt_long already printed an error message. */
        break;
      default:
        abort ();
      }
  }
  return std::make_pair(client,fitmodel);
}
