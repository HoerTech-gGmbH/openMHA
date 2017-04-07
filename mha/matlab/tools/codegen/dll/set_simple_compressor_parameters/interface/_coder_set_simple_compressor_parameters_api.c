/*
 * _coder_set_simple_compressor_parameters_api.c
 *
 * Code generation for function 'set_simple_compressor_parameters'
 *
 */

/* Include files */
#include "_coder_set_simple_compressor_parameters_api.h"

/* Function Declarations */
static unsigned int emlrt_marshallIn(const emlrtStack *sp, const mxArray
  *channels, const char *identifier);
static unsigned int b_emlrt_marshallIn(const emlrtStack *sp, const mxArray *u,
  const emlrtMsgIdentifier *parentId);
static double c_emlrt_marshallIn(const emlrtStack *sp, const mxArray
  *sampling_rate, const char *identifier);
static double d_emlrt_marshallIn(const emlrtStack *sp, const mxArray *u, const
  emlrtMsgIdentifier *parentId);
static void e_emlrt_marshallIn(const emlrtStack *sp, const mxArray
  *gaintable_levels_, const char *identifier, emxArray_real_T *y);
static void f_emlrt_marshallIn(const emlrtStack *sp, const mxArray *u, const
  emlrtMsgIdentifier *parentId, emxArray_real_T *y);
static void g_emlrt_marshallIn(const emlrtStack *sp, const mxArray *gaintable_,
  const char *identifier, emxArray_real_T *y);
static void h_emlrt_marshallIn(const emlrtStack *sp, const mxArray *u, const
  emlrtMsgIdentifier *parentId, emxArray_real_T *y);
static const mxArray *emlrt_marshallOut(const emxArray_real_T *u);
static unsigned int i_emlrt_marshallIn(const emlrtStack *sp, const mxArray *src,
  const emlrtMsgIdentifier *msgId);
static double j_emlrt_marshallIn(const emlrtStack *sp, const mxArray *src, const
  emlrtMsgIdentifier *msgId);
static void k_emlrt_marshallIn(const emlrtStack *sp, const mxArray *src, const
  emlrtMsgIdentifier *msgId, emxArray_real_T *ret);
static void l_emlrt_marshallIn(const emlrtStack *sp, const mxArray *src, const
  emlrtMsgIdentifier *msgId, emxArray_real_T *ret);
static void emxInit_real_T(const emlrtStack *sp, emxArray_real_T **pEmxArray,
  int numDimensions, boolean_T doPush);
static void emxFree_real_T(emxArray_real_T **pEmxArray);

/* Function Definitions */
void set_simple_compressor_parameters_initialize(emlrtContext *aContext)
{
  emlrtStack st = { NULL, NULL, NULL };

  emlrtCreateRootTLS(&emlrtRootTLSGlobal, aContext, NULL, 1);
  st.tls = emlrtRootTLSGlobal;
  emlrtClearAllocCountR2012b(&st, false, 0U, 0);
  emlrtEnterRtStackR2012b(&st);
  emlrtFirstTimeR2012b(emlrtRootTLSGlobal);
}

void set_simple_compressor_parameters_terminate(void)
{
  emlrtStack st = { NULL, NULL, NULL };

  st.tls = emlrtRootTLSGlobal;
  emlrtLeaveRtStackR2012b(&st);
  emlrtDestroyRootTLS(&emlrtRootTLSGlobal);
}

void set_simple_compressor_parameters_atexit(void)
{
  emlrtStack st = { NULL, NULL, NULL };

  emlrtCreateRootTLS(&emlrtRootTLSGlobal, &emlrtContextGlobal, NULL, 1);
  st.tls = emlrtRootTLSGlobal;
  emlrtEnterRtStackR2012b(&st);
  emlrtLeaveRtStackR2012b(&st);
  emlrtDestroyRootTLS(&emlrtRootTLSGlobal);
  set_simple_compressor_parameters_xil_terminate();
}

void set_simple_compressor_parameters_api(const mxArray *prhs[7])
{
  emxArray_real_T *gaintable_levels_;
  emxArray_real_T *gaintable_;
  emxArray_real_T *attack_times;
  emxArray_real_T *release_times;
  unsigned int channels;
  double sampling_rate;
  unsigned int block_size;
  emlrtStack st = { NULL, NULL, NULL };

  st.tls = emlrtRootTLSGlobal;
  emlrtHeapReferenceStackEnterFcnR2012b(&st);
  emxInit_real_T(&st, &gaintable_levels_, 2, true);
  emxInit_real_T(&st, &gaintable_, 2, true);
  emxInit_real_T(&st, &attack_times, 2, true);
  emxInit_real_T(&st, &release_times, 2, true);
  prhs[3] = emlrtProtectR2012b(prhs[3], 3, false, -1);
  prhs[4] = emlrtProtectR2012b(prhs[4], 4, false, -1);
  prhs[5] = emlrtProtectR2012b(prhs[5], 5, false, -1);
  prhs[6] = emlrtProtectR2012b(prhs[6], 6, false, -1);

  /* Marshall function inputs */
  channels = emlrt_marshallIn(&st, emlrtAliasP(prhs[0]), "channels");
  sampling_rate = c_emlrt_marshallIn(&st, emlrtAliasP(prhs[1]), "sampling_rate");
  block_size = emlrt_marshallIn(&st, emlrtAliasP(prhs[2]), "block_size");
  e_emlrt_marshallIn(&st, emlrtAlias(prhs[3]), "gaintable_levels_",
                     gaintable_levels_);
  g_emlrt_marshallIn(&st, emlrtAlias(prhs[4]), "gaintable_", gaintable_);
  e_emlrt_marshallIn(&st, emlrtAlias(prhs[5]), "attack_times", attack_times);
  e_emlrt_marshallIn(&st, emlrtAlias(prhs[6]), "release_times", release_times);

  /* Invoke the target function */
  set_simple_compressor_parameters(channels, sampling_rate, block_size,
    gaintable_levels_, gaintable_, attack_times, release_times);
  release_times->canFreeData = false;
  emxFree_real_T(&release_times);
  attack_times->canFreeData = false;
  emxFree_real_T(&attack_times);
  gaintable_->canFreeData = false;
  emxFree_real_T(&gaintable_);
  gaintable_levels_->canFreeData = false;
  emxFree_real_T(&gaintable_levels_);
  emlrtHeapReferenceStackLeaveFcnR2012b(&st);
}

static unsigned int emlrt_marshallIn(const emlrtStack *sp, const mxArray
  *channels, const char *identifier)
{
  unsigned int y;
  emlrtMsgIdentifier thisId;
  thisId.fIdentifier = identifier;
  thisId.fParent = NULL;
  y = b_emlrt_marshallIn(sp, emlrtAlias(channels), &thisId);
  emlrtDestroyArray(&channels);
  return y;
}

static unsigned int b_emlrt_marshallIn(const emlrtStack *sp, const mxArray *u,
  const emlrtMsgIdentifier *parentId)
{
  unsigned int y;
  y = i_emlrt_marshallIn(sp, emlrtAlias(u), parentId);
  emlrtDestroyArray(&u);
  return y;
}

static double c_emlrt_marshallIn(const emlrtStack *sp, const mxArray
  *sampling_rate, const char *identifier)
{
  double y;
  emlrtMsgIdentifier thisId;
  thisId.fIdentifier = identifier;
  thisId.fParent = NULL;
  y = d_emlrt_marshallIn(sp, emlrtAlias(sampling_rate), &thisId);
  emlrtDestroyArray(&sampling_rate);
  return y;
}

static double d_emlrt_marshallIn(const emlrtStack *sp, const mxArray *u, const
  emlrtMsgIdentifier *parentId)
{
  double y;
  y = j_emlrt_marshallIn(sp, emlrtAlias(u), parentId);
  emlrtDestroyArray(&u);
  return y;
}

static void e_emlrt_marshallIn(const emlrtStack *sp, const mxArray
  *gaintable_levels_, const char *identifier, emxArray_real_T *y)
{
  emlrtMsgIdentifier thisId;
  thisId.fIdentifier = identifier;
  thisId.fParent = NULL;
  f_emlrt_marshallIn(sp, emlrtAlias(gaintable_levels_), &thisId, y);
  emlrtDestroyArray(&gaintable_levels_);
}

static void f_emlrt_marshallIn(const emlrtStack *sp, const mxArray *u, const
  emlrtMsgIdentifier *parentId, emxArray_real_T *y)
{
  k_emlrt_marshallIn(sp, emlrtAlias(u), parentId, y);
  emlrtDestroyArray(&u);
}

static void g_emlrt_marshallIn(const emlrtStack *sp, const mxArray *gaintable_,
  const char *identifier, emxArray_real_T *y)
{
  emlrtMsgIdentifier thisId;
  thisId.fIdentifier = identifier;
  thisId.fParent = NULL;
  h_emlrt_marshallIn(sp, emlrtAlias(gaintable_), &thisId, y);
  emlrtDestroyArray(&gaintable_);
}

static void h_emlrt_marshallIn(const emlrtStack *sp, const mxArray *u, const
  emlrtMsgIdentifier *parentId, emxArray_real_T *y)
{
  l_emlrt_marshallIn(sp, emlrtAlias(u), parentId, y);
  emlrtDestroyArray(&u);
}

void simple_compressor_api(const mxArray *prhs[1], const mxArray *plhs[1])
{
  emxArray_real_T *input_signal;
  emxArray_real_T *output_signal;
  emlrtStack st = { NULL, NULL, NULL };

  st.tls = emlrtRootTLSGlobal;
  emlrtHeapReferenceStackEnterFcnR2012b(&st);
  emxInit_real_T(&st, &input_signal, 2, true);
  emxInit_real_T(&st, &output_signal, 2, true);
  prhs[0] = emlrtProtectR2012b(prhs[0], 0, false, -1);

  /* Marshall function inputs */
  g_emlrt_marshallIn(&st, emlrtAlias(prhs[0]), "input_signal", input_signal);

  /* Invoke the target function */
  simple_compressor(input_signal, output_signal);

  /* Marshall function outputs */
  plhs[0] = emlrt_marshallOut(output_signal);
  output_signal->canFreeData = false;
  emxFree_real_T(&output_signal);
  input_signal->canFreeData = false;
  emxFree_real_T(&input_signal);
  emlrtHeapReferenceStackLeaveFcnR2012b(&st);
}

static const mxArray *emlrt_marshallOut(const emxArray_real_T *u)
{
  const mxArray *y;
  static const int iv0[2] = { 0, 0 };

  const mxArray *m0;
  y = NULL;
  m0 = emlrtCreateNumericArray(2, iv0, mxDOUBLE_CLASS, mxREAL);
  mxSetData((mxArray *)m0, (void *)u->data);
  emlrtSetDimensions((mxArray *)m0, u->size, 2);
  emlrtAssign(&y, m0);
  return y;
}

static unsigned int i_emlrt_marshallIn(const emlrtStack *sp, const mxArray *src,
  const emlrtMsgIdentifier *msgId)
{
  unsigned int ret;
  emlrtCheckBuiltInR2012b(sp, msgId, src, "uint32", false, 0U, 0);
  ret = *(unsigned int *)mxGetData(src);
  emlrtDestroyArray(&src);
  return ret;
}

static double j_emlrt_marshallIn(const emlrtStack *sp, const mxArray *src, const
  emlrtMsgIdentifier *msgId)
{
  double ret;
  emlrtCheckBuiltInR2012b(sp, msgId, src, "double", false, 0U, 0);
  ret = *(double *)mxGetData(src);
  emlrtDestroyArray(&src);
  return ret;
}

static void k_emlrt_marshallIn(const emlrtStack *sp, const mxArray *src, const
  emlrtMsgIdentifier *msgId, emxArray_real_T *ret)
{
  int iv1[2];
  boolean_T bv0[2];
  int i0;
  static const boolean_T bv1[2] = { false, true };

  int iv2[2];
  for (i0 = 0; i0 < 2; i0++) {
    iv1[i0] = 1 + -2 * i0;
    bv0[i0] = bv1[i0];
  }

  emlrtCheckVsBuiltInR2012b(sp, msgId, src, "double", false, 2U, iv1, bv0, iv2);
  ret->size[0] = iv2[0];
  ret->size[1] = iv2[1];
  ret->allocatedSize = ret->size[0] * ret->size[1];
  ret->data = (double *)mxGetData(src);
  ret->canFreeData = false;
  emlrtDestroyArray(&src);
}

static void l_emlrt_marshallIn(const emlrtStack *sp, const mxArray *src, const
  emlrtMsgIdentifier *msgId, emxArray_real_T *ret)
{
  int iv3[2];
  boolean_T bv2[2];
  int i;
  int iv4[2];
  for (i = 0; i < 2; i++) {
    iv3[i] = -1;
    bv2[i] = true;
  }

  emlrtCheckVsBuiltInR2012b(sp, msgId, src, "double", false, 2U, iv3, bv2, iv4);
  ret->size[0] = iv4[0];
  ret->size[1] = iv4[1];
  ret->allocatedSize = ret->size[0] * ret->size[1];
  ret->data = (double *)mxGetData(src);
  ret->canFreeData = false;
  emlrtDestroyArray(&src);
}

static void emxInit_real_T(const emlrtStack *sp, emxArray_real_T **pEmxArray,
  int numDimensions, boolean_T doPush)
{
  emxArray_real_T *emxArray;
  int i;
  *pEmxArray = (emxArray_real_T *)emlrtMallocMex(sizeof(emxArray_real_T));
  if (doPush) {
    emlrtPushHeapReferenceStackR2012b(sp, (void *)pEmxArray, (void (*)(void *))
      emxFree_real_T);
  }

  emxArray = *pEmxArray;
  emxArray->data = (double *)NULL;
  emxArray->numDimensions = numDimensions;
  emxArray->size = (int *)emlrtMallocMex((unsigned int)(sizeof(int) *
    numDimensions));
  emxArray->allocatedSize = 0;
  emxArray->canFreeData = true;
  for (i = 0; i < numDimensions; i++) {
    emxArray->size[i] = 0;
  }
}

static void emxFree_real_T(emxArray_real_T **pEmxArray)
{
  if (*pEmxArray != (emxArray_real_T *)NULL) {
    if (((*pEmxArray)->data != (double *)NULL) && (*pEmxArray)->canFreeData) {
      emlrtFreeMex((void *)(*pEmxArray)->data);
    }

    emlrtFreeMex((void *)(*pEmxArray)->size);
    emlrtFreeMex((void *)*pEmxArray);
    *pEmxArray = (emxArray_real_T *)NULL;
  }
}

/* End of code generation (_coder_set_simple_compressor_parameters_api.c) */
