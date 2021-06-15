#include <iostream>
#include <stdlib.h>
#include "nalnl2wrapper.hh"
#include "parser.hh"

static double thirdOctaveFreqs[19] = {
     125,  160,  200,  250,  315,  400,  500,  630,  800, 1000,
	 1250, 1600, 2000, 2500, 3150, 4000, 5000, 6300, 8000
 };

int main(int argc, char* argv[])
{
	double data[19];

    // Parse command line
    auto [client, fitmodel]=parser(argc,argv);
    
    SetAdultChild(client.adultChild, client.dateOfBirth);
    SetExperience(client.experience);
    SetCompSpeed(fitmodel.compSpeed);
    SetTonalLanguage(client.tonal);
    SetGender(client.gender);
      
    CrossOverFrequencies_NL2(fitmodel.CFArray.data(), fitmodel.channels, client.ac.data(), client.bc.data(), fitmodel.freqInCh.data());
    setBWC(fitmodel.channels, fitmodel.CFArray.data() /*thirdOctaveFreqs*/);
    CompressionThreshold_NL2(fitmodel.ct.data(), fitmodel.bandwidth, fitmodel.selection, fitmodel.WBCT, fitmodel.haType, fitmodel.direction, fitmodel.mic, fitmodel.calcCh.data());
    CenterFrequencies(fitmodel.centerF.data(), fitmodel.CFArray.data(), fitmodel.channels);

    for(const auto lvl : fitmodel.level){
        for( int i = 0; i < 19; i++ ){
		    data[i] = -99;
		}
        RealEarInsertionGain_NL2(data, client.ac.data(), client.bc.data(), lvl, fitmodel.limiting, fitmodel.channels, fitmodel.direction, fitmodel.mic, client.acOther.data(), fitmodel.bilateral);
          
          
        // output to console
		for( int i = 0; i < 19; i++ ){
		    std::cout << data[i] << " ";
	  	}
        std::cout << "\n";
    }
    return 0;
}
