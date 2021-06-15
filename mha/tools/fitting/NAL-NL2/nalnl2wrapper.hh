#ifndef NALNL2WRAPPER_HH
#define NALNL2WRAPPER_HH
#include <vector>

/// Interface of the NAL NL2 DLL, copied from the DevKit C++ Console Example Application
#define DllImport extern  "C" __declspec ( dllimport )
DllImport int __stdcall RealEarInsertionGain_NL2(double REIR[], double AC[], double BC[], double L, int limiting, int channels, int direction, int mic, double ACother[], int noOfAids);
DllImport int __stdcall setBWC(int channels, double crossOver[]);
DllImport int __stdcall CompressionThreshold_NL2(double CT[], int bandWidth, int selection, int WBCT, int aidType, int direction, int mic, int calcCh[]);
DllImport void __stdcall SetAdultChild(int adultChild, int dateOfBirth);
DllImport void __stdcall SetExperience(int experience);
DllImport void __stdcall SetCompSpeed(int compSpeed);
DllImport void __stdcall SetTonalLanguage(int tonal);
DllImport void __stdcall SetGender(int gender);
DllImport int __stdcall CrossOverFrequencies_NL2(double CFArray[], int channels, double AC[], double BC[], int FreqInCh[]);
DllImport int __stdcall CenterFrequencies(int centerF[], double CFArray[], int channels);

/// Data specific to the person for which the hearing aid is fitted
struct Client
{
public:
  /// Year_of_Birth*10000 + Month_of_Birth*100 + DayOfBirth.  E.g. if Date
  /// of birth is April 28 1973, this field contains the number 19730428.
  int dateOfBirth=19500101;
  /// 0 - adult; 1 - child; 2 - calculate for dateOfBirth
  int adultChild=0;
  /// 0 - unknown; 1 - male; 2 - female
  int gender=0;
  /// Lanugage of client is: 0 - non-tonal; 1 - tonal
  int tonal=0;
  /// Client is 0 - experienced hearing aid user; 1 - first time hearing aid user
  int experience=1;
  /// Air conduction thresholds in dB HL
  std::vector<double> ac = {0,0,0,0,0,0,0,0,0};
  // Bone conduction thresholds in dB HL
  std::vector<double> bc = {0,0,0,0,0,0,0,0,0};
  // Air conduction thresholds of contraleteral ear
  std::vector<double> acOther = {0,0,0,0,0,0,0,0,0};
};

/// Data specific to the hearing aid that is fitted
struct Fitmodel {
  /// Time constants of WDRC: 0 - very slow; 1 - very fast; 2 - dual
  int compSpeed=1;
  /// input level / dB to compute gains for
  std::vector<double> level={60};
  /// 0	Unilateral; 1	Bilateral
  bool bilateral=true;
  /// 0 - Undisturbed field; 1 - Head Surface
  int mic=1;
  /// 0 - 0° (from front); 1 - 45°
  int direction=0;
  /// number of dynamic compression bands
  int channels=18;
  /// 0 - off; 1 - wideband; 2 - multichannel
  int limiting=2;
  ///  Wide Band Comression Threshold 52 default NL2 value; 20 - 100
  int WBCT=52;
  /// Array of cross-over frequencies in Hz
  std::vector<double> CFArray={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
  /// Not sure what these frequencies are
  std::vector<int> freqInCh = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
  /// 0 - broadband; 1 - narrowband. Not sure what the effect is
  int bandwidth=1;
  /// 0 - REIG; 1 - REAG; 2 - 2ccCoupler; 3 - EarSimulator
  int selection=0;
  /// 0 - CIC; 1 - ITC; 2 - ITE; 3 - BTE
  int haType=3;
  /// Per-band flag indicates if gains should be computed for the respective band
  std::vector<int> calcCh={1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1};
  /// compression threshold per third-octave band
  std::vector<double> ct = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
  /// 0	REIG; 1	REAG
  int targetType=0;
  /// 0	Libby 4; 1	Libby 3; 2	#13; 3 Thin-tube; 4 RITC; 5 None (CIC,ITC,ITE)
  int tubing=2;
  /// 0	Tight; 1	Occluded; 2	Closed Dome; 3	1mm; 4	2mm; 5	3mm; 6	Open Dome
  int venting=0;
  /// RECD Measurement Method: 0	Predicted Values; 1	Measured
  int useRECDh=0;
  /// Center Frequencies
  std::vector<int> centerF={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
};
#endif
