/*
 * _coder_set_simple_compressor_parameters_mex.c
 *
 * Code generation for function 'set_simple_compressor_parameters'
 *
 */

/* Include files */
#include "mex.h"
#include "_coder_set_simple_compressor_parameters_api.h"

/* Function Declarations */
static void set_simple_compressor_parameters_mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]);
static void simple_compressor_mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]);

/* Variable Definitions */
emlrtContext emlrtContextGlobal = { true, false, EMLRT_VERSION_INFO, NULL, "set_simple_compressor_parameters", NULL, false, {2045744189U,2170104910U,2743257031U,4284093946U}, NULL };
void *emlrtRootTLSGlobal = NULL;
emlrtEntryPoint emlrtEntryPoints[2] = {
  { "set_simple_compressor_parameters", set_simple_compressor_parameters_mexFunction },
  { "simple_compressor", simple_compressor_mexFunction },
};

/* Function Definitions */
static void set_simple_compressor_parameters_mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
  const mxArray *inputs[7];
  int n = 0;
  int nInputs = nrhs;
  emlrtStack st = { NULL, NULL, NULL };
  /* Module initialization. */
  set_simple_compressor_parameters_initialize(&emlrtContextGlobal);
  st.tls = emlrtRootTLSGlobal;
  /* Check for proper number of arguments. */
  if (nrhs != 7) {
    emlrtErrMsgIdAndTxt(&st, "EMLRT:runTime:WrongNumberOfInputs", 5, mxINT32_CLASS, 7, mxCHAR_CLASS, 32, "set_simple_compressor_parameters");
  } else if (nlhs > 0) {
    emlrtErrMsgIdAndTxt(&st, "EMLRT:runTime:TooManyOutputArguments", 3, mxCHAR_CLASS, 32, "set_simple_compressor_parameters");
  }
  /* Temporary copy for mex inputs. */
  for (n = 0; n < nInputs; ++n) {
    inputs[n] = prhs[n];
  }
  /* Call the function. */
  set_simple_compressor_parameters_api(inputs);
  /* Module finalization. */
  set_simple_compressor_parameters_terminate();
}
static void simple_compressor_mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
  const mxArray *outputs[1];
  const mxArray *inputs[1];
  int n = 0;
  int nOutputs = (nlhs < 1 ? 1 : nlhs);
  int nInputs = nrhs;
  emlrtStack st = { NULL, NULL, NULL };
  /* Module initialization. */
  set_simple_compressor_parameters_initialize(&emlrtContextGlobal);
  st.tls = emlrtRootTLSGlobal;
  /* Check for proper number of arguments. */
  if (nrhs != 1) {
    emlrtErrMsgIdAndTxt(&st, "EMLRT:runTime:WrongNumberOfInputs", 5, mxINT32_CLASS, 1, mxCHAR_CLASS, 17, "simple_compressor");
  } else if (nlhs > 1) {
    emlrtErrMsgIdAndTxt(&st, "EMLRT:runTime:TooManyOutputArguments", 3, mxCHAR_CLASS, 17, "simple_compressor");
  }
  /* Temporary copy for mex inputs. */
  for (n = 0; n < nInputs; ++n) {
    inputs[n] = prhs[n];
  }
  /* Call the function. */
  simple_compressor_api(inputs, outputs);
  /* Copy over outputs to the caller. */
  for (n = 0; n < nOutputs; ++n) {
    plhs[n] = emlrtReturnArrayR2009a(outputs[n]);
  }
  /* Module finalization. */
  set_simple_compressor_parameters_terminate();
}

void set_simple_compressor_parameters_atexit_wrapper(void)
{
   set_simple_compressor_parameters_atexit();
}

void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
  emlrtMexFunction method;
  method = emlrtGetMethod(nrhs, prhs, emlrtEntryPoints, 2);
  /* Initialize the memory manager. */
  mexAtExit(set_simple_compressor_parameters_atexit_wrapper);
  /* Dispatch the entry-point. */
  method(nlhs, plhs, nrhs-1, prhs+1);
}
/* End of code generation (_coder_set_simple_compressor_parameters_mex.c) */
