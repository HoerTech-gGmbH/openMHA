#include <string.h>
#include <unistd.h>
#include "mex.h"
#include "mhajack.h"
#include "mexhelperfunctions.h"

#define DATA_IN prhs[0]
#define PORTS_IN prhs[1]
#define PORTS_OUT prhs[2]
#define DATA_OUT plhs[0]
#define RATE_OUT plhs[1]
#define SIZE_OUT plhs[2]

void mexJackIO(const mxArray* mxout,
	       mxArray** mxin,
	       const mxArray* mx_in_ports,
	       const mxArray* mx_out_ports,
	       mxArray** rtout,
	       mxArray** sizeout,
	       bool use_jack_transport)
{
    MHASignal::waveform_t* s_out;
    MHAMex::mx_convert( mxout, s_out );
    MHASignal::waveform_t s_in(s_out->num_frames,s_out->num_channels);
    float srate;
    unsigned int fragsize;
    std::vector<std::string> p_in;
    std::vector<std::string> p_out;
    MHAMex::mx_convert( mx_in_ports, p_in );
    MHAMex::mx_convert( mx_out_ports, p_out );
    MHAJack::io(s_out, &s_in, "jackiomex",
		p_in, p_out, 
		&srate, &fragsize,
		use_jack_transport);
    delete s_out;
    *mxin = MHAMex::mx_create(s_in);
    *rtout = double2mxarray( srate );
    *sizeout = double2mxarray( fragsize );
}

void mexFunction( int nlhs, mxArray *plhs[],
                  int nrhs, const mxArray *prhs[] )
{
    try{
	bool use_jack_transport = false;
        unsigned int k;
        if( nrhs < 3 )
            mexErrMsgTxt(
                "Usage: [y, r, s] = jackiomex( x, csInPorts, csOutPorts, bJackTransport )\n"
                "parameters:\n"
                "   x             : input matrix (one column for each channel)\n"
                "   csInPorts     : cell string array with input port names\n"
                "   csOutPorts    : cell string array with output port names\n"
		"   bJackTransport: flag, if JACK transport is to be used\n"
                "return values:\n"
                "   y : output signal (same dimension as input signal)\n"
                "   r : JACK sample rate / Hz\n"
                "   s : JACK buffer size / samples\n"
                "\n(c) 2005 by HoerTech gGmbH, Marie-Curie-Str. 2, D-26129 Oldenburg"

                );
        if( mxGetClassID( DATA_IN ) != mxDOUBLE_CLASS )
            throw "Input data is not double array.";
        if( mxGetClassID( PORTS_IN ) != mxCELL_CLASS )
            throw "Input ports are not a cell array";
        for( k=0; k<mxGetNumberOfElements(PORTS_IN); k++ ){
            if( mxGetClassID(mxGetCell(PORTS_IN,k)) != mxCHAR_CLASS )
                throw "Input port is not a string";
        }
        if( mxGetClassID( PORTS_OUT ) != mxCELL_CLASS )
            throw "Output ports are not a cell array";
        for( k=0; k<mxGetNumberOfElements(PORTS_OUT); k++ ){
            if( mxGetClassID(mxGetCell(PORTS_OUT,k)) != mxCHAR_CLASS )
               throw "Output port is not a string";
        }
	if( nrhs > 3 ){
	    int jack_tp;
	    MHAMex::mx_convert( prhs[3], jack_tp );
	    use_jack_transport = (jack_tp > 0);
	}
	mexJackIO(DATA_IN,&DATA_OUT,PORTS_IN,PORTS_OUT,&RATE_OUT,&SIZE_OUT,use_jack_transport);
    }
    catch(const char* e){
        mexErrMsgTxt(e);
    }
    catch(std::exception& e){
        mexErrMsgTxt(e.what());
    }
}


/*
 * Local variables:
 * compile-command: "make -C ../.. matlab"
 * End:
 */
