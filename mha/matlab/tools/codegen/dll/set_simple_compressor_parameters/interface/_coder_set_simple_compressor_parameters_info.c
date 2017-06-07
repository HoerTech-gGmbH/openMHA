/*
 * _coder_set_simple_compressor_parameters_info.c
 *
 * Code generation for function 'set_simple_compressor_parameters'
 *
 */

/* Include files */
#include "_coder_set_simple_compressor_parameters_info.h"

/* Function Declarations */
static void info_helper(const mxArray **info);
static const mxArray *emlrt_marshallOut(const char * u);
static const mxArray *b_emlrt_marshallOut(const uint32_T u);
static void b_info_helper(const mxArray **info);

/* Function Definitions */
const mxArray *emlrtMexFcnResolvedFunctionsInfo(void)
{
  const mxArray *nameCaptureInfo;
  nameCaptureInfo = NULL;
  emlrtAssign(&nameCaptureInfo, emlrtCreateStructMatrix(127, 1, 0, NULL));
  info_helper(&nameCaptureInfo);
  b_info_helper(&nameCaptureInfo);
  emlrtNameCapturePostProcessR2013b(&nameCaptureInfo);
  return nameCaptureInfo;
}

static void info_helper(const mxArray **info)
{
  const mxArray *rhs0 = NULL;
  const mxArray *lhs0 = NULL;
  const mxArray *rhs1 = NULL;
  const mxArray *lhs1 = NULL;
  const mxArray *rhs2 = NULL;
  const mxArray *lhs2 = NULL;
  const mxArray *rhs3 = NULL;
  const mxArray *lhs3 = NULL;
  const mxArray *rhs4 = NULL;
  const mxArray *lhs4 = NULL;
  const mxArray *rhs5 = NULL;
  const mxArray *lhs5 = NULL;
  const mxArray *rhs6 = NULL;
  const mxArray *lhs6 = NULL;
  const mxArray *rhs7 = NULL;
  const mxArray *lhs7 = NULL;
  const mxArray *rhs8 = NULL;
  const mxArray *lhs8 = NULL;
  const mxArray *rhs9 = NULL;
  const mxArray *lhs9 = NULL;
  const mxArray *rhs10 = NULL;
  const mxArray *lhs10 = NULL;
  const mxArray *rhs11 = NULL;
  const mxArray *lhs11 = NULL;
  const mxArray *rhs12 = NULL;
  const mxArray *lhs12 = NULL;
  const mxArray *rhs13 = NULL;
  const mxArray *lhs13 = NULL;
  const mxArray *rhs14 = NULL;
  const mxArray *lhs14 = NULL;
  const mxArray *rhs15 = NULL;
  const mxArray *lhs15 = NULL;
  const mxArray *rhs16 = NULL;
  const mxArray *lhs16 = NULL;
  const mxArray *rhs17 = NULL;
  const mxArray *lhs17 = NULL;
  const mxArray *rhs18 = NULL;
  const mxArray *lhs18 = NULL;
  const mxArray *rhs19 = NULL;
  const mxArray *lhs19 = NULL;
  const mxArray *rhs20 = NULL;
  const mxArray *lhs20 = NULL;
  const mxArray *rhs21 = NULL;
  const mxArray *lhs21 = NULL;
  const mxArray *rhs22 = NULL;
  const mxArray *lhs22 = NULL;
  const mxArray *rhs23 = NULL;
  const mxArray *lhs23 = NULL;
  const mxArray *rhs24 = NULL;
  const mxArray *lhs24 = NULL;
  const mxArray *rhs25 = NULL;
  const mxArray *lhs25 = NULL;
  const mxArray *rhs26 = NULL;
  const mxArray *lhs26 = NULL;
  const mxArray *rhs27 = NULL;
  const mxArray *lhs27 = NULL;
  const mxArray *rhs28 = NULL;
  const mxArray *lhs28 = NULL;
  const mxArray *rhs29 = NULL;
  const mxArray *lhs29 = NULL;
  const mxArray *rhs30 = NULL;
  const mxArray *lhs30 = NULL;
  const mxArray *rhs31 = NULL;
  const mxArray *lhs31 = NULL;
  const mxArray *rhs32 = NULL;
  const mxArray *lhs32 = NULL;
  const mxArray *rhs33 = NULL;
  const mxArray *lhs33 = NULL;
  const mxArray *rhs34 = NULL;
  const mxArray *lhs34 = NULL;
  const mxArray *rhs35 = NULL;
  const mxArray *lhs35 = NULL;
  const mxArray *rhs36 = NULL;
  const mxArray *lhs36 = NULL;
  const mxArray *rhs37 = NULL;
  const mxArray *lhs37 = NULL;
  const mxArray *rhs38 = NULL;
  const mxArray *lhs38 = NULL;
  const mxArray *rhs39 = NULL;
  const mxArray *lhs39 = NULL;
  const mxArray *rhs40 = NULL;
  const mxArray *lhs40 = NULL;
  const mxArray *rhs41 = NULL;
  const mxArray *lhs41 = NULL;
  const mxArray *rhs42 = NULL;
  const mxArray *lhs42 = NULL;
  const mxArray *rhs43 = NULL;
  const mxArray *lhs43 = NULL;
  const mxArray *rhs44 = NULL;
  const mxArray *lhs44 = NULL;
  const mxArray *rhs45 = NULL;
  const mxArray *lhs45 = NULL;
  const mxArray *rhs46 = NULL;
  const mxArray *lhs46 = NULL;
  const mxArray *rhs47 = NULL;
  const mxArray *lhs47 = NULL;
  const mxArray *rhs48 = NULL;
  const mxArray *lhs48 = NULL;
  const mxArray *rhs49 = NULL;
  const mxArray *lhs49 = NULL;
  const mxArray *rhs50 = NULL;
  const mxArray *lhs50 = NULL;
  const mxArray *rhs51 = NULL;
  const mxArray *lhs51 = NULL;
  const mxArray *rhs52 = NULL;
  const mxArray *lhs52 = NULL;
  const mxArray *rhs53 = NULL;
  const mxArray *lhs53 = NULL;
  const mxArray *rhs54 = NULL;
  const mxArray *lhs54 = NULL;
  const mxArray *rhs55 = NULL;
  const mxArray *lhs55 = NULL;
  const mxArray *rhs56 = NULL;
  const mxArray *lhs56 = NULL;
  const mxArray *rhs57 = NULL;
  const mxArray *lhs57 = NULL;
  const mxArray *rhs58 = NULL;
  const mxArray *lhs58 = NULL;
  const mxArray *rhs59 = NULL;
  const mxArray *lhs59 = NULL;
  const mxArray *rhs60 = NULL;
  const mxArray *lhs60 = NULL;
  const mxArray *rhs61 = NULL;
  const mxArray *lhs61 = NULL;
  const mxArray *rhs62 = NULL;
  const mxArray *lhs62 = NULL;
  const mxArray *rhs63 = NULL;
  const mxArray *lhs63 = NULL;
  emlrtAddField(*info, emlrt_marshallOut(
    "[E]/home/tobias/workspaces/mha/mha/matlab/tools/set_simple_compressor_parameters.m"),
                "context", 0);
  emlrtAddField(*info, emlrt_marshallOut("isequal"), "name", 0);
  emlrtAddField(*info, emlrt_marshallOut("double"), "dominantType", 0);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/elmat/isequal.m"), "resolved", 0);
  emlrtAddField(*info, b_emlrt_marshallOut(1286818758U), "fileTimeLo", 0);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "fileTimeHi", 0);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeLo", 0);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeHi", 0);
  emlrtAssign(&rhs0, emlrtCreateCellMatrix(0, 1));
  emlrtAssign(&lhs0, emlrtCreateCellMatrix(0, 1));
  emlrtAddField(*info, emlrtAliasP(rhs0), "rhs", 0);
  emlrtAddField(*info, emlrtAliasP(lhs0), "lhs", 0);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/elmat/isequal.m"), "context", 1);
  emlrtAddField(*info, emlrt_marshallOut("eml_isequal_core"), "name", 1);
  emlrtAddField(*info, emlrt_marshallOut("double"), "dominantType", 1);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/eml/eml_isequal_core.m"),
                "resolved", 1);
  emlrtAddField(*info, b_emlrt_marshallOut(1286818786U), "fileTimeLo", 1);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "fileTimeHi", 1);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeLo", 1);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeHi", 1);
  emlrtAssign(&rhs1, emlrtCreateCellMatrix(0, 1));
  emlrtAssign(&lhs1, emlrtCreateCellMatrix(0, 1));
  emlrtAddField(*info, emlrtAliasP(rhs1), "rhs", 1);
  emlrtAddField(*info, emlrtAliasP(lhs1), "lhs", 1);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/eml/eml_isequal_core.m!isequal_scalar"),
                "context", 2);
  emlrtAddField(*info, emlrt_marshallOut("isnan"), "name", 2);
  emlrtAddField(*info, emlrt_marshallOut("double"), "dominantType", 2);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/elmat/isnan.m"), "resolved", 2);
  emlrtAddField(*info, b_emlrt_marshallOut(1363713858U), "fileTimeLo", 2);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "fileTimeHi", 2);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeLo", 2);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeHi", 2);
  emlrtAssign(&rhs2, emlrtCreateCellMatrix(0, 1));
  emlrtAssign(&lhs2, emlrtCreateCellMatrix(0, 1));
  emlrtAddField(*info, emlrtAliasP(rhs2), "rhs", 2);
  emlrtAddField(*info, emlrtAliasP(lhs2), "lhs", 2);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/elmat/isnan.m"), "context", 3);
  emlrtAddField(*info, emlrt_marshallOut("coder.internal.isBuiltInNumeric"),
                "name", 3);
  emlrtAddField(*info, emlrt_marshallOut("double"), "dominantType", 3);
  emlrtAddField(*info, emlrt_marshallOut(
    "[IXE]$matlabroot$/toolbox/shared/coder/coder/+coder/+internal/isBuiltInNumeric.m"),
                "resolved", 3);
  emlrtAddField(*info, b_emlrt_marshallOut(1363714556U), "fileTimeLo", 3);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "fileTimeHi", 3);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeLo", 3);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeHi", 3);
  emlrtAssign(&rhs3, emlrtCreateCellMatrix(0, 1));
  emlrtAssign(&lhs3, emlrtCreateCellMatrix(0, 1));
  emlrtAddField(*info, emlrtAliasP(rhs3), "rhs", 3);
  emlrtAddField(*info, emlrtAliasP(lhs3), "lhs", 3);
  emlrtAddField(*info, emlrt_marshallOut(
    "[E]/home/tobias/workspaces/mha/mha/matlab/tools/set_simple_compressor_parameters.m"),
                "context", 4);
  emlrtAddField(*info, emlrt_marshallOut("mrdivide"), "name", 4);
  emlrtAddField(*info, emlrt_marshallOut("double"), "dominantType", 4);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/ops/mrdivide.p"), "resolved", 4);
  emlrtAddField(*info, b_emlrt_marshallOut(1388460096U), "fileTimeLo", 4);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "fileTimeHi", 4);
  emlrtAddField(*info, b_emlrt_marshallOut(1370009886U), "mFileTimeLo", 4);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeHi", 4);
  emlrtAssign(&rhs4, emlrtCreateCellMatrix(0, 1));
  emlrtAssign(&lhs4, emlrtCreateCellMatrix(0, 1));
  emlrtAddField(*info, emlrtAliasP(rhs4), "rhs", 4);
  emlrtAddField(*info, emlrtAliasP(lhs4), "lhs", 4);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/ops/mrdivide.p"), "context", 5);
  emlrtAddField(*info, emlrt_marshallOut("coder.internal.assert"), "name", 5);
  emlrtAddField(*info, emlrt_marshallOut("char"), "dominantType", 5);
  emlrtAddField(*info, emlrt_marshallOut(
    "[IXE]$matlabroot$/toolbox/shared/coder/coder/+coder/+internal/assert.m"),
                "resolved", 5);
  emlrtAddField(*info, b_emlrt_marshallOut(1363714556U), "fileTimeLo", 5);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "fileTimeHi", 5);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeLo", 5);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeHi", 5);
  emlrtAssign(&rhs5, emlrtCreateCellMatrix(0, 1));
  emlrtAssign(&lhs5, emlrtCreateCellMatrix(0, 1));
  emlrtAddField(*info, emlrtAliasP(rhs5), "rhs", 5);
  emlrtAddField(*info, emlrtAliasP(lhs5), "lhs", 5);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/ops/mrdivide.p"), "context", 6);
  emlrtAddField(*info, emlrt_marshallOut("rdivide"), "name", 6);
  emlrtAddField(*info, emlrt_marshallOut("double"), "dominantType", 6);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/ops/rdivide.m"), "resolved", 6);
  emlrtAddField(*info, b_emlrt_marshallOut(1363713880U), "fileTimeLo", 6);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "fileTimeHi", 6);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeLo", 6);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeHi", 6);
  emlrtAssign(&rhs6, emlrtCreateCellMatrix(0, 1));
  emlrtAssign(&lhs6, emlrtCreateCellMatrix(0, 1));
  emlrtAddField(*info, emlrtAliasP(rhs6), "rhs", 6);
  emlrtAddField(*info, emlrtAliasP(lhs6), "lhs", 6);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/ops/rdivide.m"), "context", 7);
  emlrtAddField(*info, emlrt_marshallOut("coder.internal.isBuiltInNumeric"),
                "name", 7);
  emlrtAddField(*info, emlrt_marshallOut("double"), "dominantType", 7);
  emlrtAddField(*info, emlrt_marshallOut(
    "[IXE]$matlabroot$/toolbox/shared/coder/coder/+coder/+internal/isBuiltInNumeric.m"),
                "resolved", 7);
  emlrtAddField(*info, b_emlrt_marshallOut(1363714556U), "fileTimeLo", 7);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "fileTimeHi", 7);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeLo", 7);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeHi", 7);
  emlrtAssign(&rhs7, emlrtCreateCellMatrix(0, 1));
  emlrtAssign(&lhs7, emlrtCreateCellMatrix(0, 1));
  emlrtAddField(*info, emlrtAliasP(rhs7), "rhs", 7);
  emlrtAddField(*info, emlrtAliasP(lhs7), "lhs", 7);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/ops/rdivide.m"), "context", 8);
  emlrtAddField(*info, emlrt_marshallOut("eml_scalexp_compatible"), "name", 8);
  emlrtAddField(*info, emlrt_marshallOut("double"), "dominantType", 8);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/eml/eml_scalexp_compatible.m"),
                "resolved", 8);
  emlrtAddField(*info, b_emlrt_marshallOut(1286818796U), "fileTimeLo", 8);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "fileTimeHi", 8);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeLo", 8);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeHi", 8);
  emlrtAssign(&rhs8, emlrtCreateCellMatrix(0, 1));
  emlrtAssign(&lhs8, emlrtCreateCellMatrix(0, 1));
  emlrtAddField(*info, emlrtAliasP(rhs8), "rhs", 8);
  emlrtAddField(*info, emlrtAliasP(lhs8), "lhs", 8);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/ops/rdivide.m"), "context", 9);
  emlrtAddField(*info, emlrt_marshallOut("eml_div"), "name", 9);
  emlrtAddField(*info, emlrt_marshallOut("double"), "dominantType", 9);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/eml/eml_div.m"), "resolved", 9);
  emlrtAddField(*info, b_emlrt_marshallOut(1375980688U), "fileTimeLo", 9);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "fileTimeHi", 9);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeLo", 9);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeHi", 9);
  emlrtAssign(&rhs9, emlrtCreateCellMatrix(0, 1));
  emlrtAssign(&lhs9, emlrtCreateCellMatrix(0, 1));
  emlrtAddField(*info, emlrtAliasP(rhs9), "rhs", 9);
  emlrtAddField(*info, emlrtAliasP(lhs9), "lhs", 9);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/eml/eml_div.m"), "context", 10);
  emlrtAddField(*info, emlrt_marshallOut("coder.internal.div"), "name", 10);
  emlrtAddField(*info, emlrt_marshallOut("double"), "dominantType", 10);
  emlrtAddField(*info, emlrt_marshallOut(
    "[IXE]$matlabroot$/toolbox/coder/coder/+coder/+internal/div.p"), "resolved",
                10);
  emlrtAddField(*info, b_emlrt_marshallOut(1389307920U), "fileTimeLo", 10);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "fileTimeHi", 10);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeLo", 10);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeHi", 10);
  emlrtAssign(&rhs10, emlrtCreateCellMatrix(0, 1));
  emlrtAssign(&lhs10, emlrtCreateCellMatrix(0, 1));
  emlrtAddField(*info, emlrtAliasP(rhs10), "rhs", 10);
  emlrtAddField(*info, emlrtAliasP(lhs10), "lhs", 10);
  emlrtAddField(*info, emlrt_marshallOut(
    "[E]/home/tobias/workspaces/mha/mha/matlab/tools/set_simple_compressor_parameters.m"),
                "context", 11);
  emlrtAddField(*info, emlrt_marshallOut("eml_mtimes_helper"), "name", 11);
  emlrtAddField(*info, emlrt_marshallOut(""), "dominantType", 11);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/ops/eml_mtimes_helper.m"),
                "resolved", 11);
  emlrtAddField(*info, b_emlrt_marshallOut(1383877294U), "fileTimeLo", 11);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "fileTimeHi", 11);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeLo", 11);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeHi", 11);
  emlrtAssign(&rhs11, emlrtCreateCellMatrix(0, 1));
  emlrtAssign(&lhs11, emlrtCreateCellMatrix(0, 1));
  emlrtAddField(*info, emlrtAliasP(rhs11), "rhs", 11);
  emlrtAddField(*info, emlrtAliasP(lhs11), "lhs", 11);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/ops/eml_mtimes_helper.m!common_checks"),
                "context", 12);
  emlrtAddField(*info, emlrt_marshallOut("coder.internal.isBuiltInNumeric"),
                "name", 12);
  emlrtAddField(*info, emlrt_marshallOut("double"), "dominantType", 12);
  emlrtAddField(*info, emlrt_marshallOut(
    "[IXE]$matlabroot$/toolbox/shared/coder/coder/+coder/+internal/isBuiltInNumeric.m"),
                "resolved", 12);
  emlrtAddField(*info, b_emlrt_marshallOut(1363714556U), "fileTimeLo", 12);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "fileTimeHi", 12);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeLo", 12);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeHi", 12);
  emlrtAssign(&rhs12, emlrtCreateCellMatrix(0, 1));
  emlrtAssign(&lhs12, emlrtCreateCellMatrix(0, 1));
  emlrtAddField(*info, emlrtAliasP(rhs12), "rhs", 12);
  emlrtAddField(*info, emlrtAliasP(lhs12), "lhs", 12);
  emlrtAddField(*info, emlrt_marshallOut(
    "[E]/home/tobias/workspaces/mha/mha/matlab/tools/set_simple_compressor_parameters.m"),
                "context", 13);
  emlrtAddField(*info, emlrt_marshallOut("rdivide"), "name", 13);
  emlrtAddField(*info, emlrt_marshallOut("double"), "dominantType", 13);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/ops/rdivide.m"), "resolved", 13);
  emlrtAddField(*info, b_emlrt_marshallOut(1363713880U), "fileTimeLo", 13);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "fileTimeHi", 13);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeLo", 13);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeHi", 13);
  emlrtAssign(&rhs13, emlrtCreateCellMatrix(0, 1));
  emlrtAssign(&lhs13, emlrtCreateCellMatrix(0, 1));
  emlrtAddField(*info, emlrtAliasP(rhs13), "rhs", 13);
  emlrtAddField(*info, emlrtAliasP(lhs13), "lhs", 13);
  emlrtAddField(*info, emlrt_marshallOut(
    "[E]/home/tobias/workspaces/mha/mha/matlab/tools/set_simple_compressor_parameters.m"),
                "context", 14);
  emlrtAddField(*info, emlrt_marshallOut("exp"), "name", 14);
  emlrtAddField(*info, emlrt_marshallOut("double"), "dominantType", 14);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/elfun/exp.m"), "resolved", 14);
  emlrtAddField(*info, b_emlrt_marshallOut(1343830380U), "fileTimeLo", 14);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "fileTimeHi", 14);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeLo", 14);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeHi", 14);
  emlrtAssign(&rhs14, emlrtCreateCellMatrix(0, 1));
  emlrtAssign(&lhs14, emlrtCreateCellMatrix(0, 1));
  emlrtAddField(*info, emlrtAliasP(rhs14), "rhs", 14);
  emlrtAddField(*info, emlrtAliasP(lhs14), "lhs", 14);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/elfun/exp.m"), "context", 15);
  emlrtAddField(*info, emlrt_marshallOut("eml_scalar_exp"), "name", 15);
  emlrtAddField(*info, emlrt_marshallOut("double"), "dominantType", 15);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/elfun/eml_scalar_exp.m"),
                "resolved", 15);
  emlrtAddField(*info, b_emlrt_marshallOut(1301328464U), "fileTimeLo", 15);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "fileTimeHi", 15);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeLo", 15);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeHi", 15);
  emlrtAssign(&rhs15, emlrtCreateCellMatrix(0, 1));
  emlrtAssign(&lhs15, emlrtCreateCellMatrix(0, 1));
  emlrtAddField(*info, emlrtAliasP(rhs15), "rhs", 15);
  emlrtAddField(*info, emlrtAliasP(lhs15), "lhs", 15);
  emlrtAddField(*info, emlrt_marshallOut(
    "[E]/home/tobias/workspaces/mha/mha/matlab/tools/simple_compressor.m"),
                "context", 16);
  emlrtAddField(*info, emlrt_marshallOut("eml_mtimes_helper"), "name", 16);
  emlrtAddField(*info, emlrt_marshallOut(""), "dominantType", 16);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/ops/eml_mtimes_helper.m"),
                "resolved", 16);
  emlrtAddField(*info, b_emlrt_marshallOut(1383877294U), "fileTimeLo", 16);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "fileTimeHi", 16);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeLo", 16);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeHi", 16);
  emlrtAssign(&rhs16, emlrtCreateCellMatrix(0, 1));
  emlrtAssign(&lhs16, emlrtCreateCellMatrix(0, 1));
  emlrtAddField(*info, emlrtAliasP(rhs16), "rhs", 16);
  emlrtAddField(*info, emlrtAliasP(lhs16), "lhs", 16);
  emlrtAddField(*info, emlrt_marshallOut(
    "[E]/home/tobias/workspaces/mha/mha/matlab/tools/simple_compressor.m"),
                "context", 17);
  emlrtAddField(*info, emlrt_marshallOut("isequal"), "name", 17);
  emlrtAddField(*info, emlrt_marshallOut("double"), "dominantType", 17);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/elmat/isequal.m"), "resolved", 17);
  emlrtAddField(*info, b_emlrt_marshallOut(1286818758U), "fileTimeLo", 17);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "fileTimeHi", 17);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeLo", 17);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeHi", 17);
  emlrtAssign(&rhs17, emlrtCreateCellMatrix(0, 1));
  emlrtAssign(&lhs17, emlrtCreateCellMatrix(0, 1));
  emlrtAddField(*info, emlrtAliasP(rhs17), "rhs", 17);
  emlrtAddField(*info, emlrtAliasP(lhs17), "lhs", 17);
  emlrtAddField(*info, emlrt_marshallOut(
    "[E]/home/tobias/workspaces/mha/mha/matlab/tools/simple_compressor.m"),
                "context", 18);
  emlrtAddField(*info, emlrt_marshallOut("power"), "name", 18);
  emlrtAddField(*info, emlrt_marshallOut("double"), "dominantType", 18);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/ops/power.m"), "resolved", 18);
  emlrtAddField(*info, b_emlrt_marshallOut(1363713880U), "fileTimeLo", 18);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "fileTimeHi", 18);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeLo", 18);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeHi", 18);
  emlrtAssign(&rhs18, emlrtCreateCellMatrix(0, 1));
  emlrtAssign(&lhs18, emlrtCreateCellMatrix(0, 1));
  emlrtAddField(*info, emlrtAliasP(rhs18), "rhs", 18);
  emlrtAddField(*info, emlrtAliasP(lhs18), "lhs", 18);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/ops/power.m"), "context", 19);
  emlrtAddField(*info, emlrt_marshallOut("coder.internal.isBuiltInNumeric"),
                "name", 19);
  emlrtAddField(*info, emlrt_marshallOut("double"), "dominantType", 19);
  emlrtAddField(*info, emlrt_marshallOut(
    "[IXE]$matlabroot$/toolbox/shared/coder/coder/+coder/+internal/isBuiltInNumeric.m"),
                "resolved", 19);
  emlrtAddField(*info, b_emlrt_marshallOut(1363714556U), "fileTimeLo", 19);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "fileTimeHi", 19);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeLo", 19);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeHi", 19);
  emlrtAssign(&rhs19, emlrtCreateCellMatrix(0, 1));
  emlrtAssign(&lhs19, emlrtCreateCellMatrix(0, 1));
  emlrtAddField(*info, emlrtAliasP(rhs19), "rhs", 19);
  emlrtAddField(*info, emlrtAliasP(lhs19), "lhs", 19);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/ops/power.m!fltpower"), "context",
                20);
  emlrtAddField(*info, emlrt_marshallOut("eml_scalar_eg"), "name", 20);
  emlrtAddField(*info, emlrt_marshallOut("double"), "dominantType", 20);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/eml/eml_scalar_eg.m"), "resolved",
                20);
  emlrtAddField(*info, b_emlrt_marshallOut(1375980688U), "fileTimeLo", 20);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "fileTimeHi", 20);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeLo", 20);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeHi", 20);
  emlrtAssign(&rhs20, emlrtCreateCellMatrix(0, 1));
  emlrtAssign(&lhs20, emlrtCreateCellMatrix(0, 1));
  emlrtAddField(*info, emlrtAliasP(rhs20), "rhs", 20);
  emlrtAddField(*info, emlrtAliasP(lhs20), "lhs", 20);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/eml/eml_scalar_eg.m"), "context",
                21);
  emlrtAddField(*info, emlrt_marshallOut("coder.internal.scalarEg"), "name", 21);
  emlrtAddField(*info, emlrt_marshallOut("double"), "dominantType", 21);
  emlrtAddField(*info, emlrt_marshallOut(
    "[IXE]$matlabroot$/toolbox/coder/coder/+coder/+internal/scalarEg.p"),
                "resolved", 21);
  emlrtAddField(*info, b_emlrt_marshallOut(1389307920U), "fileTimeLo", 21);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "fileTimeHi", 21);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeLo", 21);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeHi", 21);
  emlrtAssign(&rhs21, emlrtCreateCellMatrix(0, 1));
  emlrtAssign(&lhs21, emlrtCreateCellMatrix(0, 1));
  emlrtAddField(*info, emlrtAliasP(rhs21), "rhs", 21);
  emlrtAddField(*info, emlrtAliasP(lhs21), "lhs", 21);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/ops/power.m!fltpower"), "context",
                22);
  emlrtAddField(*info, emlrt_marshallOut("eml_scalexp_alloc"), "name", 22);
  emlrtAddField(*info, emlrt_marshallOut("double"), "dominantType", 22);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/eml/eml_scalexp_alloc.m"),
                "resolved", 22);
  emlrtAddField(*info, b_emlrt_marshallOut(1375980688U), "fileTimeLo", 22);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "fileTimeHi", 22);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeLo", 22);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeHi", 22);
  emlrtAssign(&rhs22, emlrtCreateCellMatrix(0, 1));
  emlrtAssign(&lhs22, emlrtCreateCellMatrix(0, 1));
  emlrtAddField(*info, emlrtAliasP(rhs22), "rhs", 22);
  emlrtAddField(*info, emlrtAliasP(lhs22), "lhs", 22);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/eml/eml_scalexp_alloc.m"),
                "context", 23);
  emlrtAddField(*info, emlrt_marshallOut("coder.internal.scalexpAlloc"), "name",
                23);
  emlrtAddField(*info, emlrt_marshallOut("double"), "dominantType", 23);
  emlrtAddField(*info, emlrt_marshallOut(
    "[IXE]$matlabroot$/toolbox/coder/coder/+coder/+internal/scalexpAlloc.p"),
                "resolved", 23);
  emlrtAddField(*info, b_emlrt_marshallOut(1389307920U), "fileTimeLo", 23);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "fileTimeHi", 23);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeLo", 23);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeHi", 23);
  emlrtAssign(&rhs23, emlrtCreateCellMatrix(0, 1));
  emlrtAssign(&lhs23, emlrtCreateCellMatrix(0, 1));
  emlrtAddField(*info, emlrtAliasP(rhs23), "rhs", 23);
  emlrtAddField(*info, emlrtAliasP(lhs23), "lhs", 23);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/ops/power.m!fltpower"), "context",
                24);
  emlrtAddField(*info, emlrt_marshallOut("floor"), "name", 24);
  emlrtAddField(*info, emlrt_marshallOut("double"), "dominantType", 24);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/elfun/floor.m"), "resolved", 24);
  emlrtAddField(*info, b_emlrt_marshallOut(1363713854U), "fileTimeLo", 24);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "fileTimeHi", 24);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeLo", 24);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeHi", 24);
  emlrtAssign(&rhs24, emlrtCreateCellMatrix(0, 1));
  emlrtAssign(&lhs24, emlrtCreateCellMatrix(0, 1));
  emlrtAddField(*info, emlrtAliasP(rhs24), "rhs", 24);
  emlrtAddField(*info, emlrtAliasP(lhs24), "lhs", 24);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/elfun/floor.m"), "context", 25);
  emlrtAddField(*info, emlrt_marshallOut("coder.internal.isBuiltInNumeric"),
                "name", 25);
  emlrtAddField(*info, emlrt_marshallOut("double"), "dominantType", 25);
  emlrtAddField(*info, emlrt_marshallOut(
    "[IXE]$matlabroot$/toolbox/shared/coder/coder/+coder/+internal/isBuiltInNumeric.m"),
                "resolved", 25);
  emlrtAddField(*info, b_emlrt_marshallOut(1363714556U), "fileTimeLo", 25);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "fileTimeHi", 25);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeLo", 25);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeHi", 25);
  emlrtAssign(&rhs25, emlrtCreateCellMatrix(0, 1));
  emlrtAssign(&lhs25, emlrtCreateCellMatrix(0, 1));
  emlrtAddField(*info, emlrtAliasP(rhs25), "rhs", 25);
  emlrtAddField(*info, emlrtAliasP(lhs25), "lhs", 25);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/elfun/floor.m"), "context", 26);
  emlrtAddField(*info, emlrt_marshallOut("eml_scalar_floor"), "name", 26);
  emlrtAddField(*info, emlrt_marshallOut("double"), "dominantType", 26);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/elfun/eml_scalar_floor.m"),
                "resolved", 26);
  emlrtAddField(*info, b_emlrt_marshallOut(1286818726U), "fileTimeLo", 26);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "fileTimeHi", 26);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeLo", 26);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeHi", 26);
  emlrtAssign(&rhs26, emlrtCreateCellMatrix(0, 1));
  emlrtAssign(&lhs26, emlrtCreateCellMatrix(0, 1));
  emlrtAddField(*info, emlrtAliasP(rhs26), "rhs", 26);
  emlrtAddField(*info, emlrtAliasP(lhs26), "lhs", 26);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/ops/power.m!scalar_float_power"),
                "context", 27);
  emlrtAddField(*info, emlrt_marshallOut("eml_scalar_eg"), "name", 27);
  emlrtAddField(*info, emlrt_marshallOut("double"), "dominantType", 27);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/eml/eml_scalar_eg.m"), "resolved",
                27);
  emlrtAddField(*info, b_emlrt_marshallOut(1375980688U), "fileTimeLo", 27);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "fileTimeHi", 27);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeLo", 27);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeHi", 27);
  emlrtAssign(&rhs27, emlrtCreateCellMatrix(0, 1));
  emlrtAssign(&lhs27, emlrtCreateCellMatrix(0, 1));
  emlrtAddField(*info, emlrtAliasP(rhs27), "rhs", 27);
  emlrtAddField(*info, emlrtAliasP(lhs27), "lhs", 27);
  emlrtAddField(*info, emlrt_marshallOut(
    "[E]/home/tobias/workspaces/mha/mha/matlab/tools/simple_compressor.m"),
                "context", 28);
  emlrtAddField(*info, emlrt_marshallOut("mean"), "name", 28);
  emlrtAddField(*info, emlrt_marshallOut("double"), "dominantType", 28);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/datafun/mean.m"), "resolved", 28);
  emlrtAddField(*info, b_emlrt_marshallOut(1383877284U), "fileTimeLo", 28);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "fileTimeHi", 28);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeLo", 28);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeHi", 28);
  emlrtAssign(&rhs28, emlrtCreateCellMatrix(0, 1));
  emlrtAssign(&lhs28, emlrtCreateCellMatrix(0, 1));
  emlrtAddField(*info, emlrtAliasP(rhs28), "rhs", 28);
  emlrtAddField(*info, emlrtAliasP(lhs28), "lhs", 28);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/datafun/mean.m"), "context", 29);
  emlrtAddField(*info, emlrt_marshallOut("coder.internal.assert"), "name", 29);
  emlrtAddField(*info, emlrt_marshallOut("char"), "dominantType", 29);
  emlrtAddField(*info, emlrt_marshallOut(
    "[IXE]$matlabroot$/toolbox/shared/coder/coder/+coder/+internal/assert.m"),
                "resolved", 29);
  emlrtAddField(*info, b_emlrt_marshallOut(1363714556U), "fileTimeLo", 29);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "fileTimeHi", 29);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeLo", 29);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeHi", 29);
  emlrtAssign(&rhs29, emlrtCreateCellMatrix(0, 1));
  emlrtAssign(&lhs29, emlrtCreateCellMatrix(0, 1));
  emlrtAddField(*info, emlrtAliasP(rhs29), "rhs", 29);
  emlrtAddField(*info, emlrtAliasP(lhs29), "lhs", 29);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/datafun/mean.m"), "context", 30);
  emlrtAddField(*info, emlrt_marshallOut("coder.internal.isBuiltInNumeric"),
                "name", 30);
  emlrtAddField(*info, emlrt_marshallOut("double"), "dominantType", 30);
  emlrtAddField(*info, emlrt_marshallOut(
    "[IXE]$matlabroot$/toolbox/shared/coder/coder/+coder/+internal/isBuiltInNumeric.m"),
                "resolved", 30);
  emlrtAddField(*info, b_emlrt_marshallOut(1363714556U), "fileTimeLo", 30);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "fileTimeHi", 30);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeLo", 30);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeHi", 30);
  emlrtAssign(&rhs30, emlrtCreateCellMatrix(0, 1));
  emlrtAssign(&lhs30, emlrtCreateCellMatrix(0, 1));
  emlrtAddField(*info, emlrtAliasP(rhs30), "rhs", 30);
  emlrtAddField(*info, emlrtAliasP(lhs30), "lhs", 30);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/datafun/mean.m"), "context", 31);
  emlrtAddField(*info, emlrt_marshallOut("isequal"), "name", 31);
  emlrtAddField(*info, emlrt_marshallOut("double"), "dominantType", 31);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/elmat/isequal.m"), "resolved", 31);
  emlrtAddField(*info, b_emlrt_marshallOut(1286818758U), "fileTimeLo", 31);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "fileTimeHi", 31);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeLo", 31);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeHi", 31);
  emlrtAssign(&rhs31, emlrtCreateCellMatrix(0, 1));
  emlrtAssign(&lhs31, emlrtCreateCellMatrix(0, 1));
  emlrtAddField(*info, emlrtAliasP(rhs31), "rhs", 31);
  emlrtAddField(*info, emlrtAliasP(lhs31), "lhs", 31);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/datafun/mean.m"), "context", 32);
  emlrtAddField(*info, emlrt_marshallOut("coder.internal.constNonSingletonDim"),
                "name", 32);
  emlrtAddField(*info, emlrt_marshallOut("double"), "dominantType", 32);
  emlrtAddField(*info, emlrt_marshallOut(
    "[IXE]$matlabroot$/toolbox/shared/coder/coder/+coder/+internal/constNonSingletonDim.m"),
                "resolved", 32);
  emlrtAddField(*info, b_emlrt_marshallOut(1372583160U), "fileTimeLo", 32);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "fileTimeHi", 32);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeLo", 32);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeHi", 32);
  emlrtAssign(&rhs32, emlrtCreateCellMatrix(0, 1));
  emlrtAssign(&lhs32, emlrtCreateCellMatrix(0, 1));
  emlrtAddField(*info, emlrtAliasP(rhs32), "rhs", 32);
  emlrtAddField(*info, emlrtAliasP(lhs32), "lhs", 32);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/datafun/mean.m"), "context", 33);
  emlrtAddField(*info, emlrt_marshallOut("sum"), "name", 33);
  emlrtAddField(*info, emlrt_marshallOut("double"), "dominantType", 33);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/datafun/sum.m"), "resolved", 33);
  emlrtAddField(*info, b_emlrt_marshallOut(1363713858U), "fileTimeLo", 33);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "fileTimeHi", 33);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeLo", 33);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeHi", 33);
  emlrtAssign(&rhs33, emlrtCreateCellMatrix(0, 1));
  emlrtAssign(&lhs33, emlrtCreateCellMatrix(0, 1));
  emlrtAddField(*info, emlrtAliasP(rhs33), "rhs", 33);
  emlrtAddField(*info, emlrtAliasP(lhs33), "lhs", 33);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/datafun/sum.m"), "context", 34);
  emlrtAddField(*info, emlrt_marshallOut("coder.internal.isBuiltInNumeric"),
                "name", 34);
  emlrtAddField(*info, emlrt_marshallOut("double"), "dominantType", 34);
  emlrtAddField(*info, emlrt_marshallOut(
    "[IXE]$matlabroot$/toolbox/shared/coder/coder/+coder/+internal/isBuiltInNumeric.m"),
                "resolved", 34);
  emlrtAddField(*info, b_emlrt_marshallOut(1363714556U), "fileTimeLo", 34);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "fileTimeHi", 34);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeLo", 34);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeHi", 34);
  emlrtAssign(&rhs34, emlrtCreateCellMatrix(0, 1));
  emlrtAssign(&lhs34, emlrtCreateCellMatrix(0, 1));
  emlrtAddField(*info, emlrtAliasP(rhs34), "rhs", 34);
  emlrtAddField(*info, emlrtAliasP(lhs34), "lhs", 34);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/datafun/sum.m"), "context", 35);
  emlrtAddField(*info, emlrt_marshallOut("eml_assert_valid_dim"), "name", 35);
  emlrtAddField(*info, emlrt_marshallOut("double"), "dominantType", 35);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/eml/eml_assert_valid_dim.m"),
                "resolved", 35);
  emlrtAddField(*info, b_emlrt_marshallOut(1372582416U), "fileTimeLo", 35);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "fileTimeHi", 35);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeLo", 35);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeHi", 35);
  emlrtAssign(&rhs35, emlrtCreateCellMatrix(0, 1));
  emlrtAssign(&lhs35, emlrtCreateCellMatrix(0, 1));
  emlrtAddField(*info, emlrtAliasP(rhs35), "rhs", 35);
  emlrtAddField(*info, emlrtAliasP(lhs35), "lhs", 35);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/eml/eml_assert_valid_dim.m"),
                "context", 36);
  emlrtAddField(*info, emlrt_marshallOut("coder.internal.assertValidDim"),
                "name", 36);
  emlrtAddField(*info, emlrt_marshallOut("double"), "dominantType", 36);
  emlrtAddField(*info, emlrt_marshallOut(
    "[IXE]$matlabroot$/toolbox/shared/coder/coder/+coder/+internal/assertValidDim.m"),
                "resolved", 36);
  emlrtAddField(*info, b_emlrt_marshallOut(1372583160U), "fileTimeLo", 36);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "fileTimeHi", 36);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeLo", 36);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeHi", 36);
  emlrtAssign(&rhs36, emlrtCreateCellMatrix(0, 1));
  emlrtAssign(&lhs36, emlrtCreateCellMatrix(0, 1));
  emlrtAddField(*info, emlrtAliasP(rhs36), "rhs", 36);
  emlrtAddField(*info, emlrtAliasP(lhs36), "lhs", 36);
  emlrtAddField(*info, emlrt_marshallOut(
    "[IXE]$matlabroot$/toolbox/shared/coder/coder/+coder/+internal/assertValidDim.m"),
                "context", 37);
  emlrtAddField(*info, emlrt_marshallOut("coder.internal.isBuiltInNumeric"),
                "name", 37);
  emlrtAddField(*info, emlrt_marshallOut("double"), "dominantType", 37);
  emlrtAddField(*info, emlrt_marshallOut(
    "[IXE]$matlabroot$/toolbox/shared/coder/coder/+coder/+internal/isBuiltInNumeric.m"),
                "resolved", 37);
  emlrtAddField(*info, b_emlrt_marshallOut(1363714556U), "fileTimeLo", 37);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "fileTimeHi", 37);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeLo", 37);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeHi", 37);
  emlrtAssign(&rhs37, emlrtCreateCellMatrix(0, 1));
  emlrtAssign(&lhs37, emlrtCreateCellMatrix(0, 1));
  emlrtAddField(*info, emlrtAliasP(rhs37), "rhs", 37);
  emlrtAddField(*info, emlrtAliasP(lhs37), "lhs", 37);
  emlrtAddField(*info, emlrt_marshallOut(
    "[IXE]$matlabroot$/toolbox/shared/coder/coder/+coder/+internal/assertValidDim.m"),
                "context", 38);
  emlrtAddField(*info, emlrt_marshallOut("floor"), "name", 38);
  emlrtAddField(*info, emlrt_marshallOut("double"), "dominantType", 38);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/elfun/floor.m"), "resolved", 38);
  emlrtAddField(*info, b_emlrt_marshallOut(1363713854U), "fileTimeLo", 38);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "fileTimeHi", 38);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeLo", 38);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeHi", 38);
  emlrtAssign(&rhs38, emlrtCreateCellMatrix(0, 1));
  emlrtAssign(&lhs38, emlrtCreateCellMatrix(0, 1));
  emlrtAddField(*info, emlrtAliasP(rhs38), "rhs", 38);
  emlrtAddField(*info, emlrtAliasP(lhs38), "lhs", 38);
  emlrtAddField(*info, emlrt_marshallOut(
    "[IXE]$matlabroot$/toolbox/shared/coder/coder/+coder/+internal/assertValidDim.m"),
                "context", 39);
  emlrtAddField(*info, emlrt_marshallOut("intmax"), "name", 39);
  emlrtAddField(*info, emlrt_marshallOut("char"), "dominantType", 39);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/elmat/intmax.m"), "resolved", 39);
  emlrtAddField(*info, b_emlrt_marshallOut(1362261882U), "fileTimeLo", 39);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "fileTimeHi", 39);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeLo", 39);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeHi", 39);
  emlrtAssign(&rhs39, emlrtCreateCellMatrix(0, 1));
  emlrtAssign(&lhs39, emlrtCreateCellMatrix(0, 1));
  emlrtAddField(*info, emlrtAliasP(rhs39), "rhs", 39);
  emlrtAddField(*info, emlrtAliasP(lhs39), "lhs", 39);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/elmat/intmax.m"), "context", 40);
  emlrtAddField(*info, emlrt_marshallOut("eml_switch_helper"), "name", 40);
  emlrtAddField(*info, emlrt_marshallOut(""), "dominantType", 40);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/eml/eml_switch_helper.m"),
                "resolved", 40);
  emlrtAddField(*info, b_emlrt_marshallOut(1381850300U), "fileTimeLo", 40);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "fileTimeHi", 40);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeLo", 40);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeHi", 40);
  emlrtAssign(&rhs40, emlrtCreateCellMatrix(0, 1));
  emlrtAssign(&lhs40, emlrtCreateCellMatrix(0, 1));
  emlrtAddField(*info, emlrtAliasP(rhs40), "rhs", 40);
  emlrtAddField(*info, emlrtAliasP(lhs40), "lhs", 40);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/datafun/sum.m"), "context", 41);
  emlrtAddField(*info, emlrt_marshallOut("eml_index_class"), "name", 41);
  emlrtAddField(*info, emlrt_marshallOut(""), "dominantType", 41);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/eml/eml_index_class.m"),
                "resolved", 41);
  emlrtAddField(*info, b_emlrt_marshallOut(1323170578U), "fileTimeLo", 41);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "fileTimeHi", 41);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeLo", 41);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeHi", 41);
  emlrtAssign(&rhs41, emlrtCreateCellMatrix(0, 1));
  emlrtAssign(&lhs41, emlrtCreateCellMatrix(0, 1));
  emlrtAddField(*info, emlrtAliasP(rhs41), "rhs", 41);
  emlrtAddField(*info, emlrtAliasP(lhs41), "lhs", 41);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/datafun/sum.m"), "context", 42);
  emlrtAddField(*info, emlrt_marshallOut("eml_scalar_eg"), "name", 42);
  emlrtAddField(*info, emlrt_marshallOut("double"), "dominantType", 42);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/eml/eml_scalar_eg.m"), "resolved",
                42);
  emlrtAddField(*info, b_emlrt_marshallOut(1375980688U), "fileTimeLo", 42);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "fileTimeHi", 42);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeLo", 42);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeHi", 42);
  emlrtAssign(&rhs42, emlrtCreateCellMatrix(0, 1));
  emlrtAssign(&lhs42, emlrtCreateCellMatrix(0, 1));
  emlrtAddField(*info, emlrtAliasP(rhs42), "rhs", 42);
  emlrtAddField(*info, emlrtAliasP(lhs42), "lhs", 42);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/datafun/sum.m"), "context", 43);
  emlrtAddField(*info, emlrt_marshallOut("eml_matrix_vstride"), "name", 43);
  emlrtAddField(*info, emlrt_marshallOut("double"), "dominantType", 43);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/eml/eml_matrix_vstride.m"),
                "resolved", 43);
  emlrtAddField(*info, b_emlrt_marshallOut(1360282350U), "fileTimeLo", 43);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "fileTimeHi", 43);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeLo", 43);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeHi", 43);
  emlrtAssign(&rhs43, emlrtCreateCellMatrix(0, 1));
  emlrtAssign(&lhs43, emlrtCreateCellMatrix(0, 1));
  emlrtAddField(*info, emlrtAliasP(rhs43), "rhs", 43);
  emlrtAddField(*info, emlrtAliasP(lhs43), "lhs", 43);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/eml/eml_matrix_vstride.m"),
                "context", 44);
  emlrtAddField(*info, emlrt_marshallOut("coder.internal.prodsize"), "name", 44);
  emlrtAddField(*info, emlrt_marshallOut("double"), "dominantType", 44);
  emlrtAddField(*info, emlrt_marshallOut(
    "[IXE]$matlabroot$/toolbox/shared/coder/coder/+coder/+internal/prodsize.m"),
                "resolved", 44);
  emlrtAddField(*info, b_emlrt_marshallOut(1360282588U), "fileTimeLo", 44);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "fileTimeHi", 44);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeLo", 44);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeHi", 44);
  emlrtAssign(&rhs44, emlrtCreateCellMatrix(0, 1));
  emlrtAssign(&lhs44, emlrtCreateCellMatrix(0, 1));
  emlrtAddField(*info, emlrtAliasP(rhs44), "rhs", 44);
  emlrtAddField(*info, emlrtAliasP(lhs44), "lhs", 44);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/datafun/sum.m"), "context", 45);
  emlrtAddField(*info, emlrt_marshallOut("eml_matrix_npages"), "name", 45);
  emlrtAddField(*info, emlrt_marshallOut("double"), "dominantType", 45);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/eml/eml_matrix_npages.m"),
                "resolved", 45);
  emlrtAddField(*info, b_emlrt_marshallOut(1360282350U), "fileTimeLo", 45);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "fileTimeHi", 45);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeLo", 45);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeHi", 45);
  emlrtAssign(&rhs45, emlrtCreateCellMatrix(0, 1));
  emlrtAssign(&lhs45, emlrtCreateCellMatrix(0, 1));
  emlrtAddField(*info, emlrtAliasP(rhs45), "rhs", 45);
  emlrtAddField(*info, emlrtAliasP(lhs45), "lhs", 45);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/eml/eml_matrix_npages.m"),
                "context", 46);
  emlrtAddField(*info, emlrt_marshallOut("coder.internal.prodsize"), "name", 46);
  emlrtAddField(*info, emlrt_marshallOut("double"), "dominantType", 46);
  emlrtAddField(*info, emlrt_marshallOut(
    "[IXE]$matlabroot$/toolbox/shared/coder/coder/+coder/+internal/prodsize.m"),
                "resolved", 46);
  emlrtAddField(*info, b_emlrt_marshallOut(1360282588U), "fileTimeLo", 46);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "fileTimeHi", 46);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeLo", 46);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeHi", 46);
  emlrtAssign(&rhs46, emlrtCreateCellMatrix(0, 1));
  emlrtAssign(&lhs46, emlrtCreateCellMatrix(0, 1));
  emlrtAddField(*info, emlrtAliasP(rhs46), "rhs", 46);
  emlrtAddField(*info, emlrtAliasP(lhs46), "lhs", 46);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/datafun/sum.m"), "context", 47);
  emlrtAddField(*info, emlrt_marshallOut("eml_int_forloop_overflow_check"),
                "name", 47);
  emlrtAddField(*info, emlrt_marshallOut(""), "dominantType", 47);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/eml/eml_int_forloop_overflow_check.m"),
                "resolved", 47);
  emlrtAddField(*info, b_emlrt_marshallOut(1375980688U), "fileTimeLo", 47);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "fileTimeHi", 47);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeLo", 47);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeHi", 47);
  emlrtAssign(&rhs47, emlrtCreateCellMatrix(0, 1));
  emlrtAssign(&lhs47, emlrtCreateCellMatrix(0, 1));
  emlrtAddField(*info, emlrtAliasP(rhs47), "rhs", 47);
  emlrtAddField(*info, emlrtAliasP(lhs47), "lhs", 47);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/eml/eml_int_forloop_overflow_check.m!eml_int_forloop_overflow_check_helper"),
                "context", 48);
  emlrtAddField(*info, emlrt_marshallOut("intmax"), "name", 48);
  emlrtAddField(*info, emlrt_marshallOut("char"), "dominantType", 48);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/elmat/intmax.m"), "resolved", 48);
  emlrtAddField(*info, b_emlrt_marshallOut(1362261882U), "fileTimeLo", 48);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "fileTimeHi", 48);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeLo", 48);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeHi", 48);
  emlrtAssign(&rhs48, emlrtCreateCellMatrix(0, 1));
  emlrtAssign(&lhs48, emlrtCreateCellMatrix(0, 1));
  emlrtAddField(*info, emlrtAliasP(rhs48), "rhs", 48);
  emlrtAddField(*info, emlrtAliasP(lhs48), "lhs", 48);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/datafun/sum.m"), "context", 49);
  emlrtAddField(*info, emlrt_marshallOut("eml_index_plus"), "name", 49);
  emlrtAddField(*info, emlrt_marshallOut("double"), "dominantType", 49);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/eml/eml_index_plus.m"),
                "resolved", 49);
  emlrtAddField(*info, b_emlrt_marshallOut(1372582416U), "fileTimeLo", 49);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "fileTimeHi", 49);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeLo", 49);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeHi", 49);
  emlrtAssign(&rhs49, emlrtCreateCellMatrix(0, 1));
  emlrtAssign(&lhs49, emlrtCreateCellMatrix(0, 1));
  emlrtAddField(*info, emlrtAliasP(rhs49), "rhs", 49);
  emlrtAddField(*info, emlrtAliasP(lhs49), "lhs", 49);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/eml/eml_index_plus.m"), "context",
                50);
  emlrtAddField(*info, emlrt_marshallOut("coder.internal.indexPlus"), "name", 50);
  emlrtAddField(*info, emlrt_marshallOut("double"), "dominantType", 50);
  emlrtAddField(*info, emlrt_marshallOut(
    "[IXE]$matlabroot$/toolbox/shared/coder/coder/+coder/+internal/indexPlus.m"),
                "resolved", 50);
  emlrtAddField(*info, b_emlrt_marshallOut(1372583160U), "fileTimeLo", 50);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "fileTimeHi", 50);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeLo", 50);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeHi", 50);
  emlrtAssign(&rhs50, emlrtCreateCellMatrix(0, 1));
  emlrtAssign(&lhs50, emlrtCreateCellMatrix(0, 1));
  emlrtAddField(*info, emlrtAliasP(rhs50), "rhs", 50);
  emlrtAddField(*info, emlrtAliasP(lhs50), "lhs", 50);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/datafun/sum.m"), "context", 51);
  emlrtAddField(*info, emlrt_marshallOut("eml_index_plus"), "name", 51);
  emlrtAddField(*info, emlrt_marshallOut("coder.internal.indexInt"),
                "dominantType", 51);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/eml/eml_index_plus.m"),
                "resolved", 51);
  emlrtAddField(*info, b_emlrt_marshallOut(1372582416U), "fileTimeLo", 51);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "fileTimeHi", 51);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeLo", 51);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeHi", 51);
  emlrtAssign(&rhs51, emlrtCreateCellMatrix(0, 1));
  emlrtAssign(&lhs51, emlrtCreateCellMatrix(0, 1));
  emlrtAddField(*info, emlrtAliasP(rhs51), "rhs", 51);
  emlrtAddField(*info, emlrtAliasP(lhs51), "lhs", 51);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/eml/eml_index_plus.m"), "context",
                52);
  emlrtAddField(*info, emlrt_marshallOut("coder.internal.indexPlus"), "name", 52);
  emlrtAddField(*info, emlrt_marshallOut("coder.internal.indexInt"),
                "dominantType", 52);
  emlrtAddField(*info, emlrt_marshallOut(
    "[IXE]$matlabroot$/toolbox/shared/coder/coder/+coder/+internal/indexPlus.m"),
                "resolved", 52);
  emlrtAddField(*info, b_emlrt_marshallOut(1372583160U), "fileTimeLo", 52);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "fileTimeHi", 52);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeLo", 52);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeHi", 52);
  emlrtAssign(&rhs52, emlrtCreateCellMatrix(0, 1));
  emlrtAssign(&lhs52, emlrtCreateCellMatrix(0, 1));
  emlrtAddField(*info, emlrtAliasP(rhs52), "rhs", 52);
  emlrtAddField(*info, emlrtAliasP(lhs52), "lhs", 52);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/datafun/mean.m"), "context", 53);
  emlrtAddField(*info, emlrt_marshallOut("rdivide"), "name", 53);
  emlrtAddField(*info, emlrt_marshallOut("double"), "dominantType", 53);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/ops/rdivide.m"), "resolved", 53);
  emlrtAddField(*info, b_emlrt_marshallOut(1363713880U), "fileTimeLo", 53);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "fileTimeHi", 53);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeLo", 53);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeHi", 53);
  emlrtAssign(&rhs53, emlrtCreateCellMatrix(0, 1));
  emlrtAssign(&lhs53, emlrtCreateCellMatrix(0, 1));
  emlrtAddField(*info, emlrtAliasP(rhs53), "rhs", 53);
  emlrtAddField(*info, emlrtAliasP(lhs53), "lhs", 53);
  emlrtAddField(*info, emlrt_marshallOut(
    "[E]/home/tobias/workspaces/mha/mha/matlab/tools/simple_compressor.m"),
                "context", 54);
  emlrtAddField(*info, emlrt_marshallOut("mrdivide"), "name", 54);
  emlrtAddField(*info, emlrt_marshallOut("double"), "dominantType", 54);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/ops/mrdivide.p"), "resolved", 54);
  emlrtAddField(*info, b_emlrt_marshallOut(1388460096U), "fileTimeLo", 54);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "fileTimeHi", 54);
  emlrtAddField(*info, b_emlrt_marshallOut(1370009886U), "mFileTimeLo", 54);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeHi", 54);
  emlrtAssign(&rhs54, emlrtCreateCellMatrix(0, 1));
  emlrtAssign(&lhs54, emlrtCreateCellMatrix(0, 1));
  emlrtAddField(*info, emlrtAliasP(rhs54), "rhs", 54);
  emlrtAddField(*info, emlrtAliasP(lhs54), "lhs", 54);
  emlrtAddField(*info, emlrt_marshallOut(
    "[E]/home/tobias/workspaces/mha/mha/matlab/tools/simple_compressor.m"),
                "context", 55);
  emlrtAddField(*info, emlrt_marshallOut("log10"), "name", 55);
  emlrtAddField(*info, emlrt_marshallOut("double"), "dominantType", 55);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/elfun/log10.m"), "resolved", 55);
  emlrtAddField(*info, b_emlrt_marshallOut(1343830382U), "fileTimeLo", 55);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "fileTimeHi", 55);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeLo", 55);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeHi", 55);
  emlrtAssign(&rhs55, emlrtCreateCellMatrix(0, 1));
  emlrtAssign(&lhs55, emlrtCreateCellMatrix(0, 1));
  emlrtAddField(*info, emlrtAliasP(rhs55), "rhs", 55);
  emlrtAddField(*info, emlrtAliasP(lhs55), "lhs", 55);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/elfun/log10.m"), "context", 56);
  emlrtAddField(*info, emlrt_marshallOut("eml_error"), "name", 56);
  emlrtAddField(*info, emlrt_marshallOut("char"), "dominantType", 56);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/eml/eml_error.m"), "resolved", 56);
  emlrtAddField(*info, b_emlrt_marshallOut(1343830358U), "fileTimeLo", 56);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "fileTimeHi", 56);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeLo", 56);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeHi", 56);
  emlrtAssign(&rhs56, emlrtCreateCellMatrix(0, 1));
  emlrtAssign(&lhs56, emlrtCreateCellMatrix(0, 1));
  emlrtAddField(*info, emlrtAliasP(rhs56), "rhs", 56);
  emlrtAddField(*info, emlrtAliasP(lhs56), "lhs", 56);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/elfun/log10.m"), "context", 57);
  emlrtAddField(*info, emlrt_marshallOut("eml_scalar_log10"), "name", 57);
  emlrtAddField(*info, emlrt_marshallOut("double"), "dominantType", 57);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/elfun/eml_scalar_log10.m"),
                "resolved", 57);
  emlrtAddField(*info, b_emlrt_marshallOut(1286818728U), "fileTimeLo", 57);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "fileTimeHi", 57);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeLo", 57);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeHi", 57);
  emlrtAssign(&rhs57, emlrtCreateCellMatrix(0, 1));
  emlrtAssign(&lhs57, emlrtCreateCellMatrix(0, 1));
  emlrtAddField(*info, emlrtAliasP(rhs57), "rhs", 57);
  emlrtAddField(*info, emlrtAliasP(lhs57), "lhs", 57);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/elfun/eml_scalar_log10.m"),
                "context", 58);
  emlrtAddField(*info, emlrt_marshallOut("realmax"), "name", 58);
  emlrtAddField(*info, emlrt_marshallOut("char"), "dominantType", 58);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/elmat/realmax.m"), "resolved", 58);
  emlrtAddField(*info, b_emlrt_marshallOut(1307651242U), "fileTimeLo", 58);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "fileTimeHi", 58);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeLo", 58);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeHi", 58);
  emlrtAssign(&rhs58, emlrtCreateCellMatrix(0, 1));
  emlrtAssign(&lhs58, emlrtCreateCellMatrix(0, 1));
  emlrtAddField(*info, emlrtAliasP(rhs58), "rhs", 58);
  emlrtAddField(*info, emlrtAliasP(lhs58), "lhs", 58);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/elmat/realmax.m"), "context", 59);
  emlrtAddField(*info, emlrt_marshallOut("eml_realmax"), "name", 59);
  emlrtAddField(*info, emlrt_marshallOut("char"), "dominantType", 59);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/eml/eml_realmax.m"), "resolved",
                59);
  emlrtAddField(*info, b_emlrt_marshallOut(1326727996U), "fileTimeLo", 59);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "fileTimeHi", 59);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeLo", 59);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeHi", 59);
  emlrtAssign(&rhs59, emlrtCreateCellMatrix(0, 1));
  emlrtAssign(&lhs59, emlrtCreateCellMatrix(0, 1));
  emlrtAddField(*info, emlrtAliasP(rhs59), "rhs", 59);
  emlrtAddField(*info, emlrtAliasP(lhs59), "lhs", 59);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/eml/eml_realmax.m"), "context",
                60);
  emlrtAddField(*info, emlrt_marshallOut("eml_float_model"), "name", 60);
  emlrtAddField(*info, emlrt_marshallOut("char"), "dominantType", 60);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/eml/eml_float_model.m"),
                "resolved", 60);
  emlrtAddField(*info, b_emlrt_marshallOut(1326727996U), "fileTimeLo", 60);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "fileTimeHi", 60);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeLo", 60);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeHi", 60);
  emlrtAssign(&rhs60, emlrtCreateCellMatrix(0, 1));
  emlrtAssign(&lhs60, emlrtCreateCellMatrix(0, 1));
  emlrtAddField(*info, emlrtAliasP(rhs60), "rhs", 60);
  emlrtAddField(*info, emlrtAliasP(lhs60), "lhs", 60);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/elfun/eml_scalar_log10.m"),
                "context", 61);
  emlrtAddField(*info, emlrt_marshallOut("mrdivide"), "name", 61);
  emlrtAddField(*info, emlrt_marshallOut("double"), "dominantType", 61);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/ops/mrdivide.p"), "resolved", 61);
  emlrtAddField(*info, b_emlrt_marshallOut(1388460096U), "fileTimeLo", 61);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "fileTimeHi", 61);
  emlrtAddField(*info, b_emlrt_marshallOut(1370009886U), "mFileTimeLo", 61);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeHi", 61);
  emlrtAssign(&rhs61, emlrtCreateCellMatrix(0, 1));
  emlrtAssign(&lhs61, emlrtCreateCellMatrix(0, 1));
  emlrtAddField(*info, emlrtAliasP(rhs61), "rhs", 61);
  emlrtAddField(*info, emlrtAliasP(lhs61), "lhs", 61);
  emlrtAddField(*info, emlrt_marshallOut(
    "[E]/home/tobias/workspaces/mha/mha/matlab/tools/simple_compressor.m"),
                "context", 62);
  emlrtAddField(*info, emlrt_marshallOut("interp1"), "name", 62);
  emlrtAddField(*info, emlrt_marshallOut("double"), "dominantType", 62);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/polyfun/interp1.m"), "resolved",
                62);
  emlrtAddField(*info, b_emlrt_marshallOut(1381850306U), "fileTimeLo", 62);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "fileTimeHi", 62);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeLo", 62);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeHi", 62);
  emlrtAssign(&rhs62, emlrtCreateCellMatrix(0, 1));
  emlrtAssign(&lhs62, emlrtCreateCellMatrix(0, 1));
  emlrtAddField(*info, emlrtAliasP(rhs62), "rhs", 62);
  emlrtAddField(*info, emlrtAliasP(lhs62), "lhs", 62);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/polyfun/interp1.m"), "context",
                63);
  emlrtAddField(*info, emlrt_marshallOut("coder.internal.assert"), "name", 63);
  emlrtAddField(*info, emlrt_marshallOut("char"), "dominantType", 63);
  emlrtAddField(*info, emlrt_marshallOut(
    "[IXE]$matlabroot$/toolbox/shared/coder/coder/+coder/+internal/assert.m"),
                "resolved", 63);
  emlrtAddField(*info, b_emlrt_marshallOut(1363714556U), "fileTimeLo", 63);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "fileTimeHi", 63);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeLo", 63);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeHi", 63);
  emlrtAssign(&rhs63, emlrtCreateCellMatrix(0, 1));
  emlrtAssign(&lhs63, emlrtCreateCellMatrix(0, 1));
  emlrtAddField(*info, emlrtAliasP(rhs63), "rhs", 63);
  emlrtAddField(*info, emlrtAliasP(lhs63), "lhs", 63);
  emlrtDestroyArray(&rhs0);
  emlrtDestroyArray(&lhs0);
  emlrtDestroyArray(&rhs1);
  emlrtDestroyArray(&lhs1);
  emlrtDestroyArray(&rhs2);
  emlrtDestroyArray(&lhs2);
  emlrtDestroyArray(&rhs3);
  emlrtDestroyArray(&lhs3);
  emlrtDestroyArray(&rhs4);
  emlrtDestroyArray(&lhs4);
  emlrtDestroyArray(&rhs5);
  emlrtDestroyArray(&lhs5);
  emlrtDestroyArray(&rhs6);
  emlrtDestroyArray(&lhs6);
  emlrtDestroyArray(&rhs7);
  emlrtDestroyArray(&lhs7);
  emlrtDestroyArray(&rhs8);
  emlrtDestroyArray(&lhs8);
  emlrtDestroyArray(&rhs9);
  emlrtDestroyArray(&lhs9);
  emlrtDestroyArray(&rhs10);
  emlrtDestroyArray(&lhs10);
  emlrtDestroyArray(&rhs11);
  emlrtDestroyArray(&lhs11);
  emlrtDestroyArray(&rhs12);
  emlrtDestroyArray(&lhs12);
  emlrtDestroyArray(&rhs13);
  emlrtDestroyArray(&lhs13);
  emlrtDestroyArray(&rhs14);
  emlrtDestroyArray(&lhs14);
  emlrtDestroyArray(&rhs15);
  emlrtDestroyArray(&lhs15);
  emlrtDestroyArray(&rhs16);
  emlrtDestroyArray(&lhs16);
  emlrtDestroyArray(&rhs17);
  emlrtDestroyArray(&lhs17);
  emlrtDestroyArray(&rhs18);
  emlrtDestroyArray(&lhs18);
  emlrtDestroyArray(&rhs19);
  emlrtDestroyArray(&lhs19);
  emlrtDestroyArray(&rhs20);
  emlrtDestroyArray(&lhs20);
  emlrtDestroyArray(&rhs21);
  emlrtDestroyArray(&lhs21);
  emlrtDestroyArray(&rhs22);
  emlrtDestroyArray(&lhs22);
  emlrtDestroyArray(&rhs23);
  emlrtDestroyArray(&lhs23);
  emlrtDestroyArray(&rhs24);
  emlrtDestroyArray(&lhs24);
  emlrtDestroyArray(&rhs25);
  emlrtDestroyArray(&lhs25);
  emlrtDestroyArray(&rhs26);
  emlrtDestroyArray(&lhs26);
  emlrtDestroyArray(&rhs27);
  emlrtDestroyArray(&lhs27);
  emlrtDestroyArray(&rhs28);
  emlrtDestroyArray(&lhs28);
  emlrtDestroyArray(&rhs29);
  emlrtDestroyArray(&lhs29);
  emlrtDestroyArray(&rhs30);
  emlrtDestroyArray(&lhs30);
  emlrtDestroyArray(&rhs31);
  emlrtDestroyArray(&lhs31);
  emlrtDestroyArray(&rhs32);
  emlrtDestroyArray(&lhs32);
  emlrtDestroyArray(&rhs33);
  emlrtDestroyArray(&lhs33);
  emlrtDestroyArray(&rhs34);
  emlrtDestroyArray(&lhs34);
  emlrtDestroyArray(&rhs35);
  emlrtDestroyArray(&lhs35);
  emlrtDestroyArray(&rhs36);
  emlrtDestroyArray(&lhs36);
  emlrtDestroyArray(&rhs37);
  emlrtDestroyArray(&lhs37);
  emlrtDestroyArray(&rhs38);
  emlrtDestroyArray(&lhs38);
  emlrtDestroyArray(&rhs39);
  emlrtDestroyArray(&lhs39);
  emlrtDestroyArray(&rhs40);
  emlrtDestroyArray(&lhs40);
  emlrtDestroyArray(&rhs41);
  emlrtDestroyArray(&lhs41);
  emlrtDestroyArray(&rhs42);
  emlrtDestroyArray(&lhs42);
  emlrtDestroyArray(&rhs43);
  emlrtDestroyArray(&lhs43);
  emlrtDestroyArray(&rhs44);
  emlrtDestroyArray(&lhs44);
  emlrtDestroyArray(&rhs45);
  emlrtDestroyArray(&lhs45);
  emlrtDestroyArray(&rhs46);
  emlrtDestroyArray(&lhs46);
  emlrtDestroyArray(&rhs47);
  emlrtDestroyArray(&lhs47);
  emlrtDestroyArray(&rhs48);
  emlrtDestroyArray(&lhs48);
  emlrtDestroyArray(&rhs49);
  emlrtDestroyArray(&lhs49);
  emlrtDestroyArray(&rhs50);
  emlrtDestroyArray(&lhs50);
  emlrtDestroyArray(&rhs51);
  emlrtDestroyArray(&lhs51);
  emlrtDestroyArray(&rhs52);
  emlrtDestroyArray(&lhs52);
  emlrtDestroyArray(&rhs53);
  emlrtDestroyArray(&lhs53);
  emlrtDestroyArray(&rhs54);
  emlrtDestroyArray(&lhs54);
  emlrtDestroyArray(&rhs55);
  emlrtDestroyArray(&lhs55);
  emlrtDestroyArray(&rhs56);
  emlrtDestroyArray(&lhs56);
  emlrtDestroyArray(&rhs57);
  emlrtDestroyArray(&lhs57);
  emlrtDestroyArray(&rhs58);
  emlrtDestroyArray(&lhs58);
  emlrtDestroyArray(&rhs59);
  emlrtDestroyArray(&lhs59);
  emlrtDestroyArray(&rhs60);
  emlrtDestroyArray(&lhs60);
  emlrtDestroyArray(&rhs61);
  emlrtDestroyArray(&lhs61);
  emlrtDestroyArray(&rhs62);
  emlrtDestroyArray(&lhs62);
  emlrtDestroyArray(&rhs63);
  emlrtDestroyArray(&lhs63);
}

static const mxArray *emlrt_marshallOut(const char * u)
{
  const mxArray *y;
  const mxArray *m0;
  y = NULL;
  m0 = emlrtCreateString(u);
  emlrtAssign(&y, m0);
  return y;
}

static const mxArray *b_emlrt_marshallOut(const uint32_T u)
{
  const mxArray *y;
  const mxArray *m1;
  y = NULL;
  m1 = emlrtCreateNumericMatrix(1, 1, mxUINT32_CLASS, mxREAL);
  *(uint32_T *)mxGetData(m1) = u;
  emlrtAssign(&y, m1);
  return y;
}

static void b_info_helper(const mxArray **info)
{
  const mxArray *rhs64 = NULL;
  const mxArray *lhs64 = NULL;
  const mxArray *rhs65 = NULL;
  const mxArray *lhs65 = NULL;
  const mxArray *rhs66 = NULL;
  const mxArray *lhs66 = NULL;
  const mxArray *rhs67 = NULL;
  const mxArray *lhs67 = NULL;
  const mxArray *rhs68 = NULL;
  const mxArray *lhs68 = NULL;
  const mxArray *rhs69 = NULL;
  const mxArray *lhs69 = NULL;
  const mxArray *rhs70 = NULL;
  const mxArray *lhs70 = NULL;
  const mxArray *rhs71 = NULL;
  const mxArray *lhs71 = NULL;
  const mxArray *rhs72 = NULL;
  const mxArray *lhs72 = NULL;
  const mxArray *rhs73 = NULL;
  const mxArray *lhs73 = NULL;
  const mxArray *rhs74 = NULL;
  const mxArray *lhs74 = NULL;
  const mxArray *rhs75 = NULL;
  const mxArray *lhs75 = NULL;
  const mxArray *rhs76 = NULL;
  const mxArray *lhs76 = NULL;
  const mxArray *rhs77 = NULL;
  const mxArray *lhs77 = NULL;
  const mxArray *rhs78 = NULL;
  const mxArray *lhs78 = NULL;
  const mxArray *rhs79 = NULL;
  const mxArray *lhs79 = NULL;
  const mxArray *rhs80 = NULL;
  const mxArray *lhs80 = NULL;
  const mxArray *rhs81 = NULL;
  const mxArray *lhs81 = NULL;
  const mxArray *rhs82 = NULL;
  const mxArray *lhs82 = NULL;
  const mxArray *rhs83 = NULL;
  const mxArray *lhs83 = NULL;
  const mxArray *rhs84 = NULL;
  const mxArray *lhs84 = NULL;
  const mxArray *rhs85 = NULL;
  const mxArray *lhs85 = NULL;
  const mxArray *rhs86 = NULL;
  const mxArray *lhs86 = NULL;
  const mxArray *rhs87 = NULL;
  const mxArray *lhs87 = NULL;
  const mxArray *rhs88 = NULL;
  const mxArray *lhs88 = NULL;
  const mxArray *rhs89 = NULL;
  const mxArray *lhs89 = NULL;
  const mxArray *rhs90 = NULL;
  const mxArray *lhs90 = NULL;
  const mxArray *rhs91 = NULL;
  const mxArray *lhs91 = NULL;
  const mxArray *rhs92 = NULL;
  const mxArray *lhs92 = NULL;
  const mxArray *rhs93 = NULL;
  const mxArray *lhs93 = NULL;
  const mxArray *rhs94 = NULL;
  const mxArray *lhs94 = NULL;
  const mxArray *rhs95 = NULL;
  const mxArray *lhs95 = NULL;
  const mxArray *rhs96 = NULL;
  const mxArray *lhs96 = NULL;
  const mxArray *rhs97 = NULL;
  const mxArray *lhs97 = NULL;
  const mxArray *rhs98 = NULL;
  const mxArray *lhs98 = NULL;
  const mxArray *rhs99 = NULL;
  const mxArray *lhs99 = NULL;
  const mxArray *rhs100 = NULL;
  const mxArray *lhs100 = NULL;
  const mxArray *rhs101 = NULL;
  const mxArray *lhs101 = NULL;
  const mxArray *rhs102 = NULL;
  const mxArray *lhs102 = NULL;
  const mxArray *rhs103 = NULL;
  const mxArray *lhs103 = NULL;
  const mxArray *rhs104 = NULL;
  const mxArray *lhs104 = NULL;
  const mxArray *rhs105 = NULL;
  const mxArray *lhs105 = NULL;
  const mxArray *rhs106 = NULL;
  const mxArray *lhs106 = NULL;
  const mxArray *rhs107 = NULL;
  const mxArray *lhs107 = NULL;
  const mxArray *rhs108 = NULL;
  const mxArray *lhs108 = NULL;
  const mxArray *rhs109 = NULL;
  const mxArray *lhs109 = NULL;
  const mxArray *rhs110 = NULL;
  const mxArray *lhs110 = NULL;
  const mxArray *rhs111 = NULL;
  const mxArray *lhs111 = NULL;
  const mxArray *rhs112 = NULL;
  const mxArray *lhs112 = NULL;
  const mxArray *rhs113 = NULL;
  const mxArray *lhs113 = NULL;
  const mxArray *rhs114 = NULL;
  const mxArray *lhs114 = NULL;
  const mxArray *rhs115 = NULL;
  const mxArray *lhs115 = NULL;
  const mxArray *rhs116 = NULL;
  const mxArray *lhs116 = NULL;
  const mxArray *rhs117 = NULL;
  const mxArray *lhs117 = NULL;
  const mxArray *rhs118 = NULL;
  const mxArray *lhs118 = NULL;
  const mxArray *rhs119 = NULL;
  const mxArray *lhs119 = NULL;
  const mxArray *rhs120 = NULL;
  const mxArray *lhs120 = NULL;
  const mxArray *rhs121 = NULL;
  const mxArray *lhs121 = NULL;
  const mxArray *rhs122 = NULL;
  const mxArray *lhs122 = NULL;
  const mxArray *rhs123 = NULL;
  const mxArray *lhs123 = NULL;
  const mxArray *rhs124 = NULL;
  const mxArray *lhs124 = NULL;
  const mxArray *rhs125 = NULL;
  const mxArray *lhs125 = NULL;
  const mxArray *rhs126 = NULL;
  const mxArray *lhs126 = NULL;
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/polyfun/interp1.m!interp1_work"),
                "context", 64);
  emlrtAddField(*info, emlrt_marshallOut("coder.internal.assert"), "name", 64);
  emlrtAddField(*info, emlrt_marshallOut("char"), "dominantType", 64);
  emlrtAddField(*info, emlrt_marshallOut(
    "[IXE]$matlabroot$/toolbox/shared/coder/coder/+coder/+internal/assert.m"),
                "resolved", 64);
  emlrtAddField(*info, b_emlrt_marshallOut(1363714556U), "fileTimeLo", 64);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "fileTimeHi", 64);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeLo", 64);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeHi", 64);
  emlrtAssign(&rhs64, emlrtCreateCellMatrix(0, 1));
  emlrtAssign(&lhs64, emlrtCreateCellMatrix(0, 1));
  emlrtAddField(*info, emlrtAliasP(rhs64), "rhs", 64);
  emlrtAddField(*info, emlrtAliasP(lhs64), "lhs", 64);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/polyfun/interp1.m!interp1_work"),
                "context", 65);
  emlrtAddField(*info, emlrt_marshallOut("eml_scalar_eg"), "name", 65);
  emlrtAddField(*info, emlrt_marshallOut("double"), "dominantType", 65);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/eml/eml_scalar_eg.m"), "resolved",
                65);
  emlrtAddField(*info, b_emlrt_marshallOut(1375980688U), "fileTimeLo", 65);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "fileTimeHi", 65);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeLo", 65);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeHi", 65);
  emlrtAssign(&rhs65, emlrtCreateCellMatrix(0, 1));
  emlrtAssign(&lhs65, emlrtCreateCellMatrix(0, 1));
  emlrtAddField(*info, emlrtAliasP(rhs65), "rhs", 65);
  emlrtAddField(*info, emlrtAliasP(lhs65), "lhs", 65);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/polyfun/interp1.m!StringToMethodID"),
                "context", 66);
  emlrtAddField(*info, emlrt_marshallOut("coder.internal.assert"), "name", 66);
  emlrtAddField(*info, emlrt_marshallOut("char"), "dominantType", 66);
  emlrtAddField(*info, emlrt_marshallOut(
    "[IXE]$matlabroot$/toolbox/shared/coder/coder/+coder/+internal/assert.m"),
                "resolved", 66);
  emlrtAddField(*info, b_emlrt_marshallOut(1363714556U), "fileTimeLo", 66);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "fileTimeHi", 66);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeLo", 66);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeHi", 66);
  emlrtAssign(&rhs66, emlrtCreateCellMatrix(0, 1));
  emlrtAssign(&lhs66, emlrtCreateCellMatrix(0, 1));
  emlrtAddField(*info, emlrtAliasP(rhs66), "rhs", 66);
  emlrtAddField(*info, emlrtAliasP(lhs66), "lhs", 66);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/polyfun/interp1.m!StringToMethodID"),
                "context", 67);
  emlrtAddField(*info, emlrt_marshallOut("length"), "name", 67);
  emlrtAddField(*info, emlrt_marshallOut("char"), "dominantType", 67);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/elmat/length.m"), "resolved", 67);
  emlrtAddField(*info, b_emlrt_marshallOut(1303146206U), "fileTimeLo", 67);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "fileTimeHi", 67);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeLo", 67);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeHi", 67);
  emlrtAssign(&rhs67, emlrtCreateCellMatrix(0, 1));
  emlrtAssign(&lhs67, emlrtCreateCellMatrix(0, 1));
  emlrtAddField(*info, emlrtAliasP(rhs67), "rhs", 67);
  emlrtAddField(*info, emlrtAliasP(lhs67), "lhs", 67);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/polyfun/interp1.m!StringToMethodID"),
                "context", 68);
  emlrtAddField(*info, emlrt_marshallOut("max"), "name", 68);
  emlrtAddField(*info, emlrt_marshallOut("double"), "dominantType", 68);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/datafun/max.m"), "resolved", 68);
  emlrtAddField(*info, b_emlrt_marshallOut(1311255316U), "fileTimeLo", 68);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "fileTimeHi", 68);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeLo", 68);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeHi", 68);
  emlrtAssign(&rhs68, emlrtCreateCellMatrix(0, 1));
  emlrtAssign(&lhs68, emlrtCreateCellMatrix(0, 1));
  emlrtAddField(*info, emlrtAliasP(rhs68), "rhs", 68);
  emlrtAddField(*info, emlrtAliasP(lhs68), "lhs", 68);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/datafun/max.m"), "context", 69);
  emlrtAddField(*info, emlrt_marshallOut("eml_min_or_max"), "name", 69);
  emlrtAddField(*info, emlrt_marshallOut("char"), "dominantType", 69);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/eml/eml_min_or_max.m"),
                "resolved", 69);
  emlrtAddField(*info, b_emlrt_marshallOut(1378295984U), "fileTimeLo", 69);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "fileTimeHi", 69);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeLo", 69);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeHi", 69);
  emlrtAssign(&rhs69, emlrtCreateCellMatrix(0, 1));
  emlrtAssign(&lhs69, emlrtCreateCellMatrix(0, 1));
  emlrtAddField(*info, emlrtAliasP(rhs69), "rhs", 69);
  emlrtAddField(*info, emlrtAliasP(lhs69), "lhs", 69);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/eml/eml_min_or_max.m!eml_bin_extremum"),
                "context", 70);
  emlrtAddField(*info, emlrt_marshallOut("eml_scalar_eg"), "name", 70);
  emlrtAddField(*info, emlrt_marshallOut("double"), "dominantType", 70);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/eml/eml_scalar_eg.m"), "resolved",
                70);
  emlrtAddField(*info, b_emlrt_marshallOut(1375980688U), "fileTimeLo", 70);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "fileTimeHi", 70);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeLo", 70);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeHi", 70);
  emlrtAssign(&rhs70, emlrtCreateCellMatrix(0, 1));
  emlrtAssign(&lhs70, emlrtCreateCellMatrix(0, 1));
  emlrtAddField(*info, emlrtAliasP(rhs70), "rhs", 70);
  emlrtAddField(*info, emlrtAliasP(lhs70), "lhs", 70);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/eml/eml_min_or_max.m!eml_bin_extremum"),
                "context", 71);
  emlrtAddField(*info, emlrt_marshallOut("eml_scalexp_alloc"), "name", 71);
  emlrtAddField(*info, emlrt_marshallOut("double"), "dominantType", 71);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/eml/eml_scalexp_alloc.m"),
                "resolved", 71);
  emlrtAddField(*info, b_emlrt_marshallOut(1375980688U), "fileTimeLo", 71);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "fileTimeHi", 71);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeLo", 71);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeHi", 71);
  emlrtAssign(&rhs71, emlrtCreateCellMatrix(0, 1));
  emlrtAssign(&lhs71, emlrtCreateCellMatrix(0, 1));
  emlrtAddField(*info, emlrtAliasP(rhs71), "rhs", 71);
  emlrtAddField(*info, emlrtAliasP(lhs71), "lhs", 71);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/eml/eml_min_or_max.m!eml_bin_extremum"),
                "context", 72);
  emlrtAddField(*info, emlrt_marshallOut("eml_index_class"), "name", 72);
  emlrtAddField(*info, emlrt_marshallOut(""), "dominantType", 72);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/eml/eml_index_class.m"),
                "resolved", 72);
  emlrtAddField(*info, b_emlrt_marshallOut(1323170578U), "fileTimeLo", 72);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "fileTimeHi", 72);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeLo", 72);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeHi", 72);
  emlrtAssign(&rhs72, emlrtCreateCellMatrix(0, 1));
  emlrtAssign(&lhs72, emlrtCreateCellMatrix(0, 1));
  emlrtAddField(*info, emlrtAliasP(rhs72), "rhs", 72);
  emlrtAddField(*info, emlrtAliasP(lhs72), "lhs", 72);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/eml/eml_min_or_max.m!eml_scalar_bin_extremum"),
                "context", 73);
  emlrtAddField(*info, emlrt_marshallOut("eml_scalar_eg"), "name", 73);
  emlrtAddField(*info, emlrt_marshallOut("double"), "dominantType", 73);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/eml/eml_scalar_eg.m"), "resolved",
                73);
  emlrtAddField(*info, b_emlrt_marshallOut(1375980688U), "fileTimeLo", 73);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "fileTimeHi", 73);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeLo", 73);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeHi", 73);
  emlrtAssign(&rhs73, emlrtCreateCellMatrix(0, 1));
  emlrtAssign(&lhs73, emlrtCreateCellMatrix(0, 1));
  emlrtAddField(*info, emlrtAliasP(rhs73), "rhs", 73);
  emlrtAddField(*info, emlrtAliasP(lhs73), "lhs", 73);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/eml/eml_min_or_max.m!eml_scalar_bin_extremum"),
                "context", 74);
  emlrtAddField(*info, emlrt_marshallOut("coder.internal.isBuiltInNumeric"),
                "name", 74);
  emlrtAddField(*info, emlrt_marshallOut("double"), "dominantType", 74);
  emlrtAddField(*info, emlrt_marshallOut(
    "[IXE]$matlabroot$/toolbox/shared/coder/coder/+coder/+internal/isBuiltInNumeric.m"),
                "resolved", 74);
  emlrtAddField(*info, b_emlrt_marshallOut(1363714556U), "fileTimeLo", 74);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "fileTimeHi", 74);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeLo", 74);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeHi", 74);
  emlrtAssign(&rhs74, emlrtCreateCellMatrix(0, 1));
  emlrtAssign(&lhs74, emlrtCreateCellMatrix(0, 1));
  emlrtAddField(*info, emlrtAliasP(rhs74), "rhs", 74);
  emlrtAddField(*info, emlrtAliasP(lhs74), "lhs", 74);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/polyfun/interp1.m!StringToMethodID"),
                "context", 75);
  emlrtAddField(*info, emlrt_marshallOut("strncmpi"), "name", 75);
  emlrtAddField(*info, emlrt_marshallOut("char"), "dominantType", 75);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/strfun/strncmpi.m"), "resolved",
                75);
  emlrtAddField(*info, b_emlrt_marshallOut(1328857922U), "fileTimeLo", 75);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "fileTimeHi", 75);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeLo", 75);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeHi", 75);
  emlrtAssign(&rhs75, emlrtCreateCellMatrix(0, 1));
  emlrtAssign(&lhs75, emlrtCreateCellMatrix(0, 1));
  emlrtAddField(*info, emlrtAliasP(rhs75), "rhs", 75);
  emlrtAddField(*info, emlrtAliasP(lhs75), "lhs", 75);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/strfun/strncmpi.m"), "context",
                76);
  emlrtAddField(*info, emlrt_marshallOut("eml_assert_supported_string"), "name",
                76);
  emlrtAddField(*info, emlrt_marshallOut("char"), "dominantType", 76);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/strfun/eml_assert_supported_string.m"),
                "resolved", 76);
  emlrtAddField(*info, b_emlrt_marshallOut(1327419110U), "fileTimeLo", 76);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "fileTimeHi", 76);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeLo", 76);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeHi", 76);
  emlrtAssign(&rhs76, emlrtCreateCellMatrix(0, 1));
  emlrtAssign(&lhs76, emlrtCreateCellMatrix(0, 1));
  emlrtAddField(*info, emlrtAliasP(rhs76), "rhs", 76);
  emlrtAddField(*info, emlrtAliasP(lhs76), "lhs", 76);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/strfun/eml_assert_supported_string.m!inrange"),
                "context", 77);
  emlrtAddField(*info, emlrt_marshallOut("eml_charmax"), "name", 77);
  emlrtAddField(*info, emlrt_marshallOut(""), "dominantType", 77);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/strfun/eml_charmax.m"),
                "resolved", 77);
  emlrtAddField(*info, b_emlrt_marshallOut(1327419110U), "fileTimeLo", 77);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "fileTimeHi", 77);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeLo", 77);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeHi", 77);
  emlrtAssign(&rhs77, emlrtCreateCellMatrix(0, 1));
  emlrtAssign(&lhs77, emlrtCreateCellMatrix(0, 1));
  emlrtAddField(*info, emlrtAliasP(rhs77), "rhs", 77);
  emlrtAddField(*info, emlrtAliasP(lhs77), "lhs", 77);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/strfun/eml_charmax.m"), "context",
                78);
  emlrtAddField(*info, emlrt_marshallOut("intmax"), "name", 78);
  emlrtAddField(*info, emlrt_marshallOut("char"), "dominantType", 78);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/elmat/intmax.m"), "resolved", 78);
  emlrtAddField(*info, b_emlrt_marshallOut(1362261882U), "fileTimeLo", 78);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "fileTimeHi", 78);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeLo", 78);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeHi", 78);
  emlrtAssign(&rhs78, emlrtCreateCellMatrix(0, 1));
  emlrtAssign(&lhs78, emlrtCreateCellMatrix(0, 1));
  emlrtAddField(*info, emlrtAliasP(rhs78), "rhs", 78);
  emlrtAddField(*info, emlrtAliasP(lhs78), "lhs", 78);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/strfun/strncmpi.m"), "context",
                79);
  emlrtAddField(*info, emlrt_marshallOut("min"), "name", 79);
  emlrtAddField(*info, emlrt_marshallOut("double"), "dominantType", 79);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/datafun/min.m"), "resolved", 79);
  emlrtAddField(*info, b_emlrt_marshallOut(1311255318U), "fileTimeLo", 79);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "fileTimeHi", 79);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeLo", 79);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeHi", 79);
  emlrtAssign(&rhs79, emlrtCreateCellMatrix(0, 1));
  emlrtAssign(&lhs79, emlrtCreateCellMatrix(0, 1));
  emlrtAddField(*info, emlrtAliasP(rhs79), "rhs", 79);
  emlrtAddField(*info, emlrtAliasP(lhs79), "lhs", 79);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/datafun/min.m"), "context", 80);
  emlrtAddField(*info, emlrt_marshallOut("eml_min_or_max"), "name", 80);
  emlrtAddField(*info, emlrt_marshallOut("char"), "dominantType", 80);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/eml/eml_min_or_max.m"),
                "resolved", 80);
  emlrtAddField(*info, b_emlrt_marshallOut(1378295984U), "fileTimeLo", 80);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "fileTimeHi", 80);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeLo", 80);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeHi", 80);
  emlrtAssign(&rhs80, emlrtCreateCellMatrix(0, 1));
  emlrtAssign(&lhs80, emlrtCreateCellMatrix(0, 1));
  emlrtAddField(*info, emlrtAliasP(rhs80), "rhs", 80);
  emlrtAddField(*info, emlrtAliasP(lhs80), "lhs", 80);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/strfun/strncmpi.m"), "context",
                81);
  emlrtAddField(*info, emlrt_marshallOut("lower"), "name", 81);
  emlrtAddField(*info, emlrt_marshallOut("char"), "dominantType", 81);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/strfun/lower.m"), "resolved", 81);
  emlrtAddField(*info, b_emlrt_marshallOut(1327419110U), "fileTimeLo", 81);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "fileTimeHi", 81);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeLo", 81);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeHi", 81);
  emlrtAssign(&rhs81, emlrtCreateCellMatrix(0, 1));
  emlrtAssign(&lhs81, emlrtCreateCellMatrix(0, 1));
  emlrtAddField(*info, emlrtAliasP(rhs81), "rhs", 81);
  emlrtAddField(*info, emlrtAliasP(lhs81), "lhs", 81);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/strfun/lower.m"), "context", 82);
  emlrtAddField(*info, emlrt_marshallOut("eml_string_transform"), "name", 82);
  emlrtAddField(*info, emlrt_marshallOut("char"), "dominantType", 82);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/strfun/eml_string_transform.m"),
                "resolved", 82);
  emlrtAddField(*info, b_emlrt_marshallOut(1327419110U), "fileTimeLo", 82);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "fileTimeHi", 82);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeLo", 82);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeHi", 82);
  emlrtAssign(&rhs82, emlrtCreateCellMatrix(0, 1));
  emlrtAssign(&lhs82, emlrtCreateCellMatrix(0, 1));
  emlrtAddField(*info, emlrtAliasP(rhs82), "rhs", 82);
  emlrtAddField(*info, emlrtAliasP(lhs82), "lhs", 82);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/strfun/eml_string_transform.m"),
                "context", 83);
  emlrtAddField(*info, emlrt_marshallOut("eml_assert_supported_string"), "name",
                83);
  emlrtAddField(*info, emlrt_marshallOut("char"), "dominantType", 83);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/strfun/eml_assert_supported_string.m"),
                "resolved", 83);
  emlrtAddField(*info, b_emlrt_marshallOut(1327419110U), "fileTimeLo", 83);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "fileTimeHi", 83);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeLo", 83);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeHi", 83);
  emlrtAssign(&rhs83, emlrtCreateCellMatrix(0, 1));
  emlrtAssign(&lhs83, emlrtCreateCellMatrix(0, 1));
  emlrtAddField(*info, emlrtAliasP(rhs83), "rhs", 83);
  emlrtAddField(*info, emlrtAliasP(lhs83), "lhs", 83);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/strfun/eml_assert_supported_string.m"),
                "context", 84);
  emlrtAddField(*info, emlrt_marshallOut("eml_charmax"), "name", 84);
  emlrtAddField(*info, emlrt_marshallOut(""), "dominantType", 84);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/strfun/eml_charmax.m"),
                "resolved", 84);
  emlrtAddField(*info, b_emlrt_marshallOut(1327419110U), "fileTimeLo", 84);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "fileTimeHi", 84);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeLo", 84);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeHi", 84);
  emlrtAssign(&rhs84, emlrtCreateCellMatrix(0, 1));
  emlrtAssign(&lhs84, emlrtCreateCellMatrix(0, 1));
  emlrtAddField(*info, emlrtAliasP(rhs84), "rhs", 84);
  emlrtAddField(*info, emlrtAliasP(lhs84), "lhs", 84);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/strfun/eml_string_transform.m"),
                "context", 85);
  emlrtAddField(*info, emlrt_marshallOut("eml_charmax"), "name", 85);
  emlrtAddField(*info, emlrt_marshallOut(""), "dominantType", 85);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/strfun/eml_charmax.m"),
                "resolved", 85);
  emlrtAddField(*info, b_emlrt_marshallOut(1327419110U), "fileTimeLo", 85);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "fileTimeHi", 85);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeLo", 85);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeHi", 85);
  emlrtAssign(&rhs85, emlrtCreateCellMatrix(0, 1));
  emlrtAssign(&lhs85, emlrtCreateCellMatrix(0, 1));
  emlrtAddField(*info, emlrtAliasP(rhs85), "rhs", 85);
  emlrtAddField(*info, emlrtAliasP(lhs85), "lhs", 85);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/strfun/eml_string_transform.m"),
                "context", 86);
  emlrtAddField(*info, emlrt_marshallOut("colon"), "name", 86);
  emlrtAddField(*info, emlrt_marshallOut("int8"), "dominantType", 86);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/ops/colon.m"), "resolved", 86);
  emlrtAddField(*info, b_emlrt_marshallOut(1378295988U), "fileTimeLo", 86);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "fileTimeHi", 86);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeLo", 86);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeHi", 86);
  emlrtAssign(&rhs86, emlrtCreateCellMatrix(0, 1));
  emlrtAssign(&lhs86, emlrtCreateCellMatrix(0, 1));
  emlrtAddField(*info, emlrtAliasP(rhs86), "rhs", 86);
  emlrtAddField(*info, emlrtAliasP(lhs86), "lhs", 86);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/ops/colon.m"), "context", 87);
  emlrtAddField(*info, emlrt_marshallOut("colon"), "name", 87);
  emlrtAddField(*info, emlrt_marshallOut("int8"), "dominantType", 87);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/ops/colon.m"), "resolved", 87);
  emlrtAddField(*info, b_emlrt_marshallOut(1378295988U), "fileTimeLo", 87);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "fileTimeHi", 87);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeLo", 87);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeHi", 87);
  emlrtAssign(&rhs87, emlrtCreateCellMatrix(0, 1));
  emlrtAssign(&lhs87, emlrtCreateCellMatrix(0, 1));
  emlrtAddField(*info, emlrtAliasP(rhs87), "rhs", 87);
  emlrtAddField(*info, emlrtAliasP(lhs87), "lhs", 87);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/ops/colon.m"), "context", 88);
  emlrtAddField(*info, emlrt_marshallOut("coder.internal.isBuiltInNumeric"),
                "name", 88);
  emlrtAddField(*info, emlrt_marshallOut("double"), "dominantType", 88);
  emlrtAddField(*info, emlrt_marshallOut(
    "[IXE]$matlabroot$/toolbox/shared/coder/coder/+coder/+internal/isBuiltInNumeric.m"),
                "resolved", 88);
  emlrtAddField(*info, b_emlrt_marshallOut(1363714556U), "fileTimeLo", 88);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "fileTimeHi", 88);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeLo", 88);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeHi", 88);
  emlrtAssign(&rhs88, emlrtCreateCellMatrix(0, 1));
  emlrtAssign(&lhs88, emlrtCreateCellMatrix(0, 1));
  emlrtAddField(*info, emlrtAliasP(rhs88), "rhs", 88);
  emlrtAddField(*info, emlrtAliasP(lhs88), "lhs", 88);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/ops/colon.m"), "context", 89);
  emlrtAddField(*info, emlrt_marshallOut("coder.internal.isBuiltInNumeric"),
                "name", 89);
  emlrtAddField(*info, emlrt_marshallOut("int8"), "dominantType", 89);
  emlrtAddField(*info, emlrt_marshallOut(
    "[IXE]$matlabroot$/toolbox/shared/coder/coder/+coder/+internal/isBuiltInNumeric.m"),
                "resolved", 89);
  emlrtAddField(*info, b_emlrt_marshallOut(1363714556U), "fileTimeLo", 89);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "fileTimeHi", 89);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeLo", 89);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeHi", 89);
  emlrtAssign(&rhs89, emlrtCreateCellMatrix(0, 1));
  emlrtAssign(&lhs89, emlrtCreateCellMatrix(0, 1));
  emlrtAddField(*info, emlrtAliasP(rhs89), "rhs", 89);
  emlrtAddField(*info, emlrtAliasP(lhs89), "lhs", 89);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/ops/colon.m"), "context", 90);
  emlrtAddField(*info, emlrt_marshallOut("floor"), "name", 90);
  emlrtAddField(*info, emlrt_marshallOut("double"), "dominantType", 90);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/elfun/floor.m"), "resolved", 90);
  emlrtAddField(*info, b_emlrt_marshallOut(1363713854U), "fileTimeLo", 90);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "fileTimeHi", 90);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeLo", 90);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeHi", 90);
  emlrtAssign(&rhs90, emlrtCreateCellMatrix(0, 1));
  emlrtAssign(&lhs90, emlrtCreateCellMatrix(0, 1));
  emlrtAddField(*info, emlrtAliasP(rhs90), "rhs", 90);
  emlrtAddField(*info, emlrtAliasP(lhs90), "lhs", 90);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/ops/colon.m!checkrange"),
                "context", 91);
  emlrtAddField(*info, emlrt_marshallOut("intmin"), "name", 91);
  emlrtAddField(*info, emlrt_marshallOut("char"), "dominantType", 91);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/elmat/intmin.m"), "resolved", 91);
  emlrtAddField(*info, b_emlrt_marshallOut(1362261882U), "fileTimeLo", 91);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "fileTimeHi", 91);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeLo", 91);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeHi", 91);
  emlrtAssign(&rhs91, emlrtCreateCellMatrix(0, 1));
  emlrtAssign(&lhs91, emlrtCreateCellMatrix(0, 1));
  emlrtAddField(*info, emlrtAliasP(rhs91), "rhs", 91);
  emlrtAddField(*info, emlrtAliasP(lhs91), "lhs", 91);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/elmat/intmin.m"), "context", 92);
  emlrtAddField(*info, emlrt_marshallOut("eml_switch_helper"), "name", 92);
  emlrtAddField(*info, emlrt_marshallOut(""), "dominantType", 92);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/eml/eml_switch_helper.m"),
                "resolved", 92);
  emlrtAddField(*info, b_emlrt_marshallOut(1381850300U), "fileTimeLo", 92);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "fileTimeHi", 92);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeLo", 92);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeHi", 92);
  emlrtAssign(&rhs92, emlrtCreateCellMatrix(0, 1));
  emlrtAssign(&lhs92, emlrtCreateCellMatrix(0, 1));
  emlrtAddField(*info, emlrtAliasP(rhs92), "rhs", 92);
  emlrtAddField(*info, emlrtAliasP(lhs92), "lhs", 92);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/ops/colon.m!checkrange"),
                "context", 93);
  emlrtAddField(*info, emlrt_marshallOut("intmax"), "name", 93);
  emlrtAddField(*info, emlrt_marshallOut("char"), "dominantType", 93);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/elmat/intmax.m"), "resolved", 93);
  emlrtAddField(*info, b_emlrt_marshallOut(1362261882U), "fileTimeLo", 93);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "fileTimeHi", 93);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeLo", 93);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeHi", 93);
  emlrtAssign(&rhs93, emlrtCreateCellMatrix(0, 1));
  emlrtAssign(&lhs93, emlrtCreateCellMatrix(0, 1));
  emlrtAddField(*info, emlrtAliasP(rhs93), "rhs", 93);
  emlrtAddField(*info, emlrtAliasP(lhs93), "lhs", 93);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/ops/colon.m!eml_integer_colon_dispatcher"),
                "context", 94);
  emlrtAddField(*info, emlrt_marshallOut("intmin"), "name", 94);
  emlrtAddField(*info, emlrt_marshallOut("char"), "dominantType", 94);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/elmat/intmin.m"), "resolved", 94);
  emlrtAddField(*info, b_emlrt_marshallOut(1362261882U), "fileTimeLo", 94);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "fileTimeHi", 94);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeLo", 94);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeHi", 94);
  emlrtAssign(&rhs94, emlrtCreateCellMatrix(0, 1));
  emlrtAssign(&lhs94, emlrtCreateCellMatrix(0, 1));
  emlrtAddField(*info, emlrtAliasP(rhs94), "rhs", 94);
  emlrtAddField(*info, emlrtAliasP(lhs94), "lhs", 94);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/ops/colon.m!eml_integer_colon_dispatcher"),
                "context", 95);
  emlrtAddField(*info, emlrt_marshallOut("intmax"), "name", 95);
  emlrtAddField(*info, emlrt_marshallOut("char"), "dominantType", 95);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/elmat/intmax.m"), "resolved", 95);
  emlrtAddField(*info, b_emlrt_marshallOut(1362261882U), "fileTimeLo", 95);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "fileTimeHi", 95);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeLo", 95);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeHi", 95);
  emlrtAssign(&rhs95, emlrtCreateCellMatrix(0, 1));
  emlrtAssign(&lhs95, emlrtCreateCellMatrix(0, 1));
  emlrtAddField(*info, emlrtAliasP(rhs95), "rhs", 95);
  emlrtAddField(*info, emlrtAliasP(lhs95), "lhs", 95);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/ops/colon.m!eml_integer_colon_dispatcher"),
                "context", 96);
  emlrtAddField(*info, emlrt_marshallOut("eml_isa_uint"), "name", 96);
  emlrtAddField(*info, emlrt_marshallOut("int8"), "dominantType", 96);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/eml/eml_isa_uint.m"), "resolved",
                96);
  emlrtAddField(*info, b_emlrt_marshallOut(1375980688U), "fileTimeLo", 96);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "fileTimeHi", 96);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeLo", 96);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeHi", 96);
  emlrtAssign(&rhs96, emlrtCreateCellMatrix(0, 1));
  emlrtAssign(&lhs96, emlrtCreateCellMatrix(0, 1));
  emlrtAddField(*info, emlrtAliasP(rhs96), "rhs", 96);
  emlrtAddField(*info, emlrtAliasP(lhs96), "lhs", 96);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/eml/eml_isa_uint.m"), "context",
                97);
  emlrtAddField(*info, emlrt_marshallOut("coder.internal.isaUint"), "name", 97);
  emlrtAddField(*info, emlrt_marshallOut("int8"), "dominantType", 97);
  emlrtAddField(*info, emlrt_marshallOut(
    "[IXE]$matlabroot$/toolbox/coder/coder/+coder/+internal/isaUint.p"),
                "resolved", 97);
  emlrtAddField(*info, b_emlrt_marshallOut(1389307920U), "fileTimeLo", 97);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "fileTimeHi", 97);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeLo", 97);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeHi", 97);
  emlrtAssign(&rhs97, emlrtCreateCellMatrix(0, 1));
  emlrtAssign(&lhs97, emlrtCreateCellMatrix(0, 1));
  emlrtAddField(*info, emlrtAliasP(rhs97), "rhs", 97);
  emlrtAddField(*info, emlrtAliasP(lhs97), "lhs", 97);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/ops/colon.m!integer_colon_length_nonnegd"),
                "context", 98);
  emlrtAddField(*info, emlrt_marshallOut("eml_unsigned_class"), "name", 98);
  emlrtAddField(*info, emlrt_marshallOut("char"), "dominantType", 98);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/eml/eml_unsigned_class.m"),
                "resolved", 98);
  emlrtAddField(*info, b_emlrt_marshallOut(1375980688U), "fileTimeLo", 98);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "fileTimeHi", 98);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeLo", 98);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeHi", 98);
  emlrtAssign(&rhs98, emlrtCreateCellMatrix(0, 1));
  emlrtAssign(&lhs98, emlrtCreateCellMatrix(0, 1));
  emlrtAddField(*info, emlrtAliasP(rhs98), "rhs", 98);
  emlrtAddField(*info, emlrtAliasP(lhs98), "lhs", 98);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/eml/eml_unsigned_class.m"),
                "context", 99);
  emlrtAddField(*info, emlrt_marshallOut("coder.internal.unsignedClass"), "name",
                99);
  emlrtAddField(*info, emlrt_marshallOut("char"), "dominantType", 99);
  emlrtAddField(*info, emlrt_marshallOut(
    "[IXE]$matlabroot$/toolbox/coder/coder/+coder/+internal/unsignedClass.p"),
                "resolved", 99);
  emlrtAddField(*info, b_emlrt_marshallOut(1389307922U), "fileTimeLo", 99);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "fileTimeHi", 99);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeLo", 99);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeHi", 99);
  emlrtAssign(&rhs99, emlrtCreateCellMatrix(0, 1));
  emlrtAssign(&lhs99, emlrtCreateCellMatrix(0, 1));
  emlrtAddField(*info, emlrtAliasP(rhs99), "rhs", 99);
  emlrtAddField(*info, emlrtAliasP(lhs99), "lhs", 99);
  emlrtAddField(*info, emlrt_marshallOut(
    "[IXE]$matlabroot$/toolbox/coder/coder/+coder/+internal/unsignedClass.p"),
                "context", 100);
  emlrtAddField(*info, emlrt_marshallOut("eml_switch_helper"), "name", 100);
  emlrtAddField(*info, emlrt_marshallOut(""), "dominantType", 100);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/eml/eml_switch_helper.m"),
                "resolved", 100);
  emlrtAddField(*info, b_emlrt_marshallOut(1381850300U), "fileTimeLo", 100);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "fileTimeHi", 100);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeLo", 100);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeHi", 100);
  emlrtAssign(&rhs100, emlrtCreateCellMatrix(0, 1));
  emlrtAssign(&lhs100, emlrtCreateCellMatrix(0, 1));
  emlrtAddField(*info, emlrtAliasP(rhs100), "rhs", 100);
  emlrtAddField(*info, emlrtAliasP(lhs100), "lhs", 100);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/ops/colon.m!integer_colon_length_nonnegd"),
                "context", 101);
  emlrtAddField(*info, emlrt_marshallOut("eml_index_class"), "name", 101);
  emlrtAddField(*info, emlrt_marshallOut(""), "dominantType", 101);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/eml/eml_index_class.m"),
                "resolved", 101);
  emlrtAddField(*info, b_emlrt_marshallOut(1323170578U), "fileTimeLo", 101);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "fileTimeHi", 101);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeLo", 101);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeHi", 101);
  emlrtAssign(&rhs101, emlrtCreateCellMatrix(0, 1));
  emlrtAssign(&lhs101, emlrtCreateCellMatrix(0, 1));
  emlrtAddField(*info, emlrtAliasP(rhs101), "rhs", 101);
  emlrtAddField(*info, emlrtAliasP(lhs101), "lhs", 101);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/ops/colon.m!integer_colon_length_nonnegd"),
                "context", 102);
  emlrtAddField(*info, emlrt_marshallOut("intmax"), "name", 102);
  emlrtAddField(*info, emlrt_marshallOut("char"), "dominantType", 102);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/elmat/intmax.m"), "resolved", 102);
  emlrtAddField(*info, b_emlrt_marshallOut(1362261882U), "fileTimeLo", 102);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "fileTimeHi", 102);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeLo", 102);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeHi", 102);
  emlrtAssign(&rhs102, emlrtCreateCellMatrix(0, 1));
  emlrtAssign(&lhs102, emlrtCreateCellMatrix(0, 1));
  emlrtAddField(*info, emlrtAliasP(rhs102), "rhs", 102);
  emlrtAddField(*info, emlrtAliasP(lhs102), "lhs", 102);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/ops/colon.m!integer_colon_length_nonnegd"),
                "context", 103);
  emlrtAddField(*info, emlrt_marshallOut("eml_isa_uint"), "name", 103);
  emlrtAddField(*info, emlrt_marshallOut("int8"), "dominantType", 103);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/eml/eml_isa_uint.m"), "resolved",
                103);
  emlrtAddField(*info, b_emlrt_marshallOut(1375980688U), "fileTimeLo", 103);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "fileTimeHi", 103);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeLo", 103);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeHi", 103);
  emlrtAssign(&rhs103, emlrtCreateCellMatrix(0, 1));
  emlrtAssign(&lhs103, emlrtCreateCellMatrix(0, 1));
  emlrtAddField(*info, emlrtAliasP(rhs103), "rhs", 103);
  emlrtAddField(*info, emlrtAliasP(lhs103), "lhs", 103);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/ops/colon.m!integer_colon_length_nonnegd"),
                "context", 104);
  emlrtAddField(*info, emlrt_marshallOut("eml_index_plus"), "name", 104);
  emlrtAddField(*info, emlrt_marshallOut("double"), "dominantType", 104);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/eml/eml_index_plus.m"),
                "resolved", 104);
  emlrtAddField(*info, b_emlrt_marshallOut(1372582416U), "fileTimeLo", 104);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "fileTimeHi", 104);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeLo", 104);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeHi", 104);
  emlrtAssign(&rhs104, emlrtCreateCellMatrix(0, 1));
  emlrtAssign(&lhs104, emlrtCreateCellMatrix(0, 1));
  emlrtAddField(*info, emlrtAliasP(rhs104), "rhs", 104);
  emlrtAddField(*info, emlrtAliasP(lhs104), "lhs", 104);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/ops/colon.m!eml_signed_integer_colon"),
                "context", 105);
  emlrtAddField(*info, emlrt_marshallOut("eml_int_forloop_overflow_check"),
                "name", 105);
  emlrtAddField(*info, emlrt_marshallOut(""), "dominantType", 105);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/eml/eml_int_forloop_overflow_check.m"),
                "resolved", 105);
  emlrtAddField(*info, b_emlrt_marshallOut(1375980688U), "fileTimeLo", 105);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "fileTimeHi", 105);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeLo", 105);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeHi", 105);
  emlrtAssign(&rhs105, emlrtCreateCellMatrix(0, 1));
  emlrtAssign(&lhs105, emlrtCreateCellMatrix(0, 1));
  emlrtAddField(*info, emlrtAliasP(rhs105), "rhs", 105);
  emlrtAddField(*info, emlrtAliasP(lhs105), "lhs", 105);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/strfun/eml_string_transform.m"),
                "context", 106);
  emlrtAddField(*info, emlrt_marshallOut("char"), "name", 106);
  emlrtAddField(*info, emlrt_marshallOut("int8"), "dominantType", 106);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/strfun/char.m"), "resolved", 106);
  emlrtAddField(*info, b_emlrt_marshallOut(1319729968U), "fileTimeLo", 106);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "fileTimeHi", 106);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeLo", 106);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeHi", 106);
  emlrtAssign(&rhs106, emlrtCreateCellMatrix(0, 1));
  emlrtAssign(&lhs106, emlrtCreateCellMatrix(0, 1));
  emlrtAddField(*info, emlrtAliasP(rhs106), "rhs", 106);
  emlrtAddField(*info, emlrtAliasP(lhs106), "lhs", 106);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/polyfun/interp1.m!interp1_work"),
                "context", 107);
  emlrtAddField(*info, emlrt_marshallOut("coder.internal.nan"), "name", 107);
  emlrtAddField(*info, emlrt_marshallOut("char"), "dominantType", 107);
  emlrtAddField(*info, emlrt_marshallOut(
    "[IXE]$matlabroot$/toolbox/shared/coder/coder/+coder/+internal/nan.m"),
                "resolved", 107);
  emlrtAddField(*info, b_emlrt_marshallOut(1362262032U), "fileTimeLo", 107);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "fileTimeHi", 107);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeLo", 107);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeHi", 107);
  emlrtAssign(&rhs107, emlrtCreateCellMatrix(0, 1));
  emlrtAssign(&lhs107, emlrtCreateCellMatrix(0, 1));
  emlrtAddField(*info, emlrtAliasP(rhs107), "rhs", 107);
  emlrtAddField(*info, emlrtAliasP(lhs107), "lhs", 107);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/polyfun/interp1.m!interp1_work"),
                "context", 108);
  emlrtAddField(*info, emlrt_marshallOut("eml_int_forloop_overflow_check"),
                "name", 108);
  emlrtAddField(*info, emlrt_marshallOut(""), "dominantType", 108);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/eml/eml_int_forloop_overflow_check.m"),
                "resolved", 108);
  emlrtAddField(*info, b_emlrt_marshallOut(1375980688U), "fileTimeLo", 108);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "fileTimeHi", 108);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeLo", 108);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeHi", 108);
  emlrtAssign(&rhs108, emlrtCreateCellMatrix(0, 1));
  emlrtAssign(&lhs108, emlrtCreateCellMatrix(0, 1));
  emlrtAddField(*info, emlrtAliasP(rhs108), "rhs", 108);
  emlrtAddField(*info, emlrtAliasP(lhs108), "lhs", 108);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/polyfun/interp1.m!interp1_work"),
                "context", 109);
  emlrtAddField(*info, emlrt_marshallOut("isnan"), "name", 109);
  emlrtAddField(*info, emlrt_marshallOut("double"), "dominantType", 109);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/elmat/isnan.m"), "resolved", 109);
  emlrtAddField(*info, b_emlrt_marshallOut(1363713858U), "fileTimeLo", 109);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "fileTimeHi", 109);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeLo", 109);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeHi", 109);
  emlrtAssign(&rhs109, emlrtCreateCellMatrix(0, 1));
  emlrtAssign(&lhs109, emlrtCreateCellMatrix(0, 1));
  emlrtAddField(*info, emlrtAliasP(rhs109), "rhs", 109);
  emlrtAddField(*info, emlrtAliasP(lhs109), "lhs", 109);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/polyfun/interp1.m!interp1_work"),
                "context", 110);
  emlrtAddField(*info, emlrt_marshallOut("eml_error"), "name", 110);
  emlrtAddField(*info, emlrt_marshallOut("char"), "dominantType", 110);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/eml/eml_error.m"), "resolved",
                110);
  emlrtAddField(*info, b_emlrt_marshallOut(1343830358U), "fileTimeLo", 110);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "fileTimeHi", 110);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeLo", 110);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeHi", 110);
  emlrtAssign(&rhs110, emlrtCreateCellMatrix(0, 1));
  emlrtAssign(&lhs110, emlrtCreateCellMatrix(0, 1));
  emlrtAddField(*info, emlrtAliasP(rhs110), "rhs", 110);
  emlrtAddField(*info, emlrtAliasP(lhs110), "lhs", 110);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/polyfun/interp1.m!interp1_work"),
                "context", 111);
  emlrtAddField(*info, emlrt_marshallOut("fliplr"), "name", 111);
  emlrtAddField(*info, emlrt_marshallOut("double"), "dominantType", 111);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/elmat/fliplr.m"), "resolved", 111);
  emlrtAddField(*info, b_emlrt_marshallOut(1286818688U), "fileTimeLo", 111);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "fileTimeHi", 111);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeLo", 111);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeHi", 111);
  emlrtAssign(&rhs111, emlrtCreateCellMatrix(0, 1));
  emlrtAssign(&lhs111, emlrtCreateCellMatrix(0, 1));
  emlrtAddField(*info, emlrtAliasP(rhs111), "rhs", 111);
  emlrtAddField(*info, emlrtAliasP(lhs111), "lhs", 111);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/elmat/fliplr.m"), "context", 112);
  emlrtAddField(*info, emlrt_marshallOut("eml_index_class"), "name", 112);
  emlrtAddField(*info, emlrt_marshallOut(""), "dominantType", 112);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/eml/eml_index_class.m"),
                "resolved", 112);
  emlrtAddField(*info, b_emlrt_marshallOut(1323170578U), "fileTimeLo", 112);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "fileTimeHi", 112);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeLo", 112);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeHi", 112);
  emlrtAssign(&rhs112, emlrtCreateCellMatrix(0, 1));
  emlrtAssign(&lhs112, emlrtCreateCellMatrix(0, 1));
  emlrtAddField(*info, emlrtAliasP(rhs112), "rhs", 112);
  emlrtAddField(*info, emlrtAliasP(lhs112), "lhs", 112);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/elmat/fliplr.m"), "context", 113);
  emlrtAddField(*info, emlrt_marshallOut("eml_index_rdivide"), "name", 113);
  emlrtAddField(*info, emlrt_marshallOut("double"), "dominantType", 113);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/eml/eml_index_rdivide.m"),
                "resolved", 113);
  emlrtAddField(*info, b_emlrt_marshallOut(1372582416U), "fileTimeLo", 113);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "fileTimeHi", 113);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeLo", 113);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeHi", 113);
  emlrtAssign(&rhs113, emlrtCreateCellMatrix(0, 1));
  emlrtAssign(&lhs113, emlrtCreateCellMatrix(0, 1));
  emlrtAddField(*info, emlrtAliasP(rhs113), "rhs", 113);
  emlrtAddField(*info, emlrtAliasP(lhs113), "lhs", 113);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/eml/eml_index_rdivide.m"),
                "context", 114);
  emlrtAddField(*info, emlrt_marshallOut("coder.internal.indexDivide"), "name",
                114);
  emlrtAddField(*info, emlrt_marshallOut("double"), "dominantType", 114);
  emlrtAddField(*info, emlrt_marshallOut(
    "[IXE]$matlabroot$/toolbox/shared/coder/coder/+coder/+internal/indexDivide.m"),
                "resolved", 114);
  emlrtAddField(*info, b_emlrt_marshallOut(1372583160U), "fileTimeLo", 114);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "fileTimeHi", 114);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeLo", 114);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeHi", 114);
  emlrtAssign(&rhs114, emlrtCreateCellMatrix(0, 1));
  emlrtAssign(&lhs114, emlrtCreateCellMatrix(0, 1));
  emlrtAddField(*info, emlrtAliasP(rhs114), "rhs", 114);
  emlrtAddField(*info, emlrtAliasP(lhs114), "lhs", 114);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/elmat/fliplr.m"), "context", 115);
  emlrtAddField(*info, emlrt_marshallOut("eml_int_forloop_overflow_check"),
                "name", 115);
  emlrtAddField(*info, emlrt_marshallOut(""), "dominantType", 115);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/eml/eml_int_forloop_overflow_check.m"),
                "resolved", 115);
  emlrtAddField(*info, b_emlrt_marshallOut(1375980688U), "fileTimeLo", 115);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "fileTimeHi", 115);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeLo", 115);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeHi", 115);
  emlrtAssign(&rhs115, emlrtCreateCellMatrix(0, 1));
  emlrtAssign(&lhs115, emlrtCreateCellMatrix(0, 1));
  emlrtAddField(*info, emlrtAliasP(rhs115), "rhs", 115);
  emlrtAddField(*info, emlrtAliasP(lhs115), "lhs", 115);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/elmat/fliplr.m"), "context", 116);
  emlrtAddField(*info, emlrt_marshallOut("eml_index_minus"), "name", 116);
  emlrtAddField(*info, emlrt_marshallOut("coder.internal.indexInt"),
                "dominantType", 116);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/eml/eml_index_minus.m"),
                "resolved", 116);
  emlrtAddField(*info, b_emlrt_marshallOut(1372582416U), "fileTimeLo", 116);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "fileTimeHi", 116);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeLo", 116);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeHi", 116);
  emlrtAssign(&rhs116, emlrtCreateCellMatrix(0, 1));
  emlrtAssign(&lhs116, emlrtCreateCellMatrix(0, 1));
  emlrtAddField(*info, emlrtAliasP(rhs116), "rhs", 116);
  emlrtAddField(*info, emlrtAliasP(lhs116), "lhs", 116);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/eml/eml_index_minus.m"),
                "context", 117);
  emlrtAddField(*info, emlrt_marshallOut("coder.internal.indexMinus"), "name",
                117);
  emlrtAddField(*info, emlrt_marshallOut("coder.internal.indexInt"),
                "dominantType", 117);
  emlrtAddField(*info, emlrt_marshallOut(
    "[IXE]$matlabroot$/toolbox/shared/coder/coder/+coder/+internal/indexMinus.m"),
                "resolved", 117);
  emlrtAddField(*info, b_emlrt_marshallOut(1372583160U), "fileTimeLo", 117);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "fileTimeHi", 117);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeLo", 117);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeHi", 117);
  emlrtAssign(&rhs117, emlrtCreateCellMatrix(0, 1));
  emlrtAssign(&lhs117, emlrtCreateCellMatrix(0, 1));
  emlrtAddField(*info, emlrtAliasP(rhs117), "rhs", 117);
  emlrtAddField(*info, emlrtAliasP(lhs117), "lhs", 117);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/elmat/fliplr.m"), "context", 118);
  emlrtAddField(*info, emlrt_marshallOut("eml_index_plus"), "name", 118);
  emlrtAddField(*info, emlrt_marshallOut("double"), "dominantType", 118);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/eml/eml_index_plus.m"),
                "resolved", 118);
  emlrtAddField(*info, b_emlrt_marshallOut(1372582416U), "fileTimeLo", 118);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "fileTimeHi", 118);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeLo", 118);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeHi", 118);
  emlrtAssign(&rhs118, emlrtCreateCellMatrix(0, 1));
  emlrtAssign(&lhs118, emlrtCreateCellMatrix(0, 1));
  emlrtAddField(*info, emlrtAliasP(rhs118), "rhs", 118);
  emlrtAddField(*info, emlrtAliasP(lhs118), "lhs", 118);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/polyfun/interp1.m!interp1Linear"),
                "context", 119);
  emlrtAddField(*info, emlrt_marshallOut("isnan"), "name", 119);
  emlrtAddField(*info, emlrt_marshallOut("double"), "dominantType", 119);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/elmat/isnan.m"), "resolved", 119);
  emlrtAddField(*info, b_emlrt_marshallOut(1363713858U), "fileTimeLo", 119);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "fileTimeHi", 119);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeLo", 119);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeHi", 119);
  emlrtAssign(&rhs119, emlrtCreateCellMatrix(0, 1));
  emlrtAssign(&lhs119, emlrtCreateCellMatrix(0, 1));
  emlrtAddField(*info, emlrtAliasP(rhs119), "rhs", 119);
  emlrtAddField(*info, emlrtAliasP(lhs119), "lhs", 119);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/polyfun/interp1.m!interp1Linear"),
                "context", 120);
  emlrtAddField(*info, emlrt_marshallOut("coder.internal.nan"), "name", 120);
  emlrtAddField(*info, emlrt_marshallOut("char"), "dominantType", 120);
  emlrtAddField(*info, emlrt_marshallOut(
    "[IXE]$matlabroot$/toolbox/shared/coder/coder/+coder/+internal/nan.m"),
                "resolved", 120);
  emlrtAddField(*info, b_emlrt_marshallOut(1362262032U), "fileTimeLo", 120);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "fileTimeHi", 120);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeLo", 120);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeHi", 120);
  emlrtAssign(&rhs120, emlrtCreateCellMatrix(0, 1));
  emlrtAssign(&lhs120, emlrtCreateCellMatrix(0, 1));
  emlrtAddField(*info, emlrtAliasP(rhs120), "rhs", 120);
  emlrtAddField(*info, emlrtAliasP(lhs120), "lhs", 120);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/polyfun/interp1.m!interp1Linear"),
                "context", 121);
  emlrtAddField(*info, emlrt_marshallOut("coder.internal.bsearch"), "name", 121);
  emlrtAddField(*info, emlrt_marshallOut("double"), "dominantType", 121);
  emlrtAddField(*info, emlrt_marshallOut(
    "[IXE]$matlabroot$/toolbox/coder/coder/+coder/+internal/bsearch.p"),
                "resolved", 121);
  emlrtAddField(*info, b_emlrt_marshallOut(1389307920U), "fileTimeLo", 121);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "fileTimeHi", 121);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeLo", 121);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeHi", 121);
  emlrtAssign(&rhs121, emlrtCreateCellMatrix(0, 1));
  emlrtAssign(&lhs121, emlrtCreateCellMatrix(0, 1));
  emlrtAddField(*info, emlrtAliasP(rhs121), "rhs", 121);
  emlrtAddField(*info, emlrtAliasP(lhs121), "lhs", 121);
  emlrtAddField(*info, emlrt_marshallOut(
    "[IXE]$matlabroot$/toolbox/coder/coder/+coder/+internal/bsearch.p"),
                "context", 122);
  emlrtAddField(*info, emlrt_marshallOut("coder.internal.narginchk"), "name",
                122);
  emlrtAddField(*info, emlrt_marshallOut(""), "dominantType", 122);
  emlrtAddField(*info, emlrt_marshallOut(
    "[IXE]$matlabroot$/toolbox/shared/coder/coder/+coder/+internal/narginchk.m"),
                "resolved", 122);
  emlrtAddField(*info, b_emlrt_marshallOut(1363714558U), "fileTimeLo", 122);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "fileTimeHi", 122);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeLo", 122);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeHi", 122);
  emlrtAssign(&rhs122, emlrtCreateCellMatrix(0, 1));
  emlrtAssign(&lhs122, emlrtCreateCellMatrix(0, 1));
  emlrtAddField(*info, emlrtAliasP(rhs122), "rhs", 122);
  emlrtAddField(*info, emlrtAliasP(lhs122), "lhs", 122);
  emlrtAddField(*info, emlrt_marshallOut(
    "[IXE]$matlabroot$/toolbox/shared/coder/coder/+coder/+internal/narginchk.m"),
                "context", 123);
  emlrtAddField(*info, emlrt_marshallOut("floor"), "name", 123);
  emlrtAddField(*info, emlrt_marshallOut("double"), "dominantType", 123);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/elfun/floor.m"), "resolved", 123);
  emlrtAddField(*info, b_emlrt_marshallOut(1363713854U), "fileTimeLo", 123);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "fileTimeHi", 123);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeLo", 123);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeHi", 123);
  emlrtAssign(&rhs123, emlrtCreateCellMatrix(0, 1));
  emlrtAssign(&lhs123, emlrtCreateCellMatrix(0, 1));
  emlrtAddField(*info, emlrtAliasP(rhs123), "rhs", 123);
  emlrtAddField(*info, emlrtAliasP(lhs123), "lhs", 123);
  emlrtAddField(*info, emlrt_marshallOut(
    "[IXE]$matlabroot$/toolbox/shared/coder/coder/+coder/+internal/narginchk.m"),
                "context", 124);
  emlrtAddField(*info, emlrt_marshallOut("coder.internal.assert"), "name", 124);
  emlrtAddField(*info, emlrt_marshallOut("char"), "dominantType", 124);
  emlrtAddField(*info, emlrt_marshallOut(
    "[IXE]$matlabroot$/toolbox/shared/coder/coder/+coder/+internal/assert.m"),
                "resolved", 124);
  emlrtAddField(*info, b_emlrt_marshallOut(1363714556U), "fileTimeLo", 124);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "fileTimeHi", 124);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeLo", 124);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeHi", 124);
  emlrtAssign(&rhs124, emlrtCreateCellMatrix(0, 1));
  emlrtAssign(&lhs124, emlrtCreateCellMatrix(0, 1));
  emlrtAddField(*info, emlrtAliasP(rhs124), "rhs", 124);
  emlrtAddField(*info, emlrtAliasP(lhs124), "lhs", 124);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/polyfun/interp1.m!interp1Linear"),
                "context", 125);
  emlrtAddField(*info, emlrt_marshallOut("mrdivide"), "name", 125);
  emlrtAddField(*info, emlrt_marshallOut("double"), "dominantType", 125);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/ops/mrdivide.p"), "resolved", 125);
  emlrtAddField(*info, b_emlrt_marshallOut(1388460096U), "fileTimeLo", 125);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "fileTimeHi", 125);
  emlrtAddField(*info, b_emlrt_marshallOut(1370009886U), "mFileTimeLo", 125);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeHi", 125);
  emlrtAssign(&rhs125, emlrtCreateCellMatrix(0, 1));
  emlrtAssign(&lhs125, emlrtCreateCellMatrix(0, 1));
  emlrtAddField(*info, emlrtAliasP(rhs125), "rhs", 125);
  emlrtAddField(*info, emlrtAliasP(lhs125), "lhs", 125);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/ops/power.m!fltpower"), "context",
                126);
  emlrtAddField(*info, emlrt_marshallOut("eml_error"), "name", 126);
  emlrtAddField(*info, emlrt_marshallOut("char"), "dominantType", 126);
  emlrtAddField(*info, emlrt_marshallOut(
    "[ILXE]$matlabroot$/toolbox/eml/lib/matlab/eml/eml_error.m"), "resolved",
                126);
  emlrtAddField(*info, b_emlrt_marshallOut(1343830358U), "fileTimeLo", 126);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "fileTimeHi", 126);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeLo", 126);
  emlrtAddField(*info, b_emlrt_marshallOut(0U), "mFileTimeHi", 126);
  emlrtAssign(&rhs126, emlrtCreateCellMatrix(0, 1));
  emlrtAssign(&lhs126, emlrtCreateCellMatrix(0, 1));
  emlrtAddField(*info, emlrtAliasP(rhs126), "rhs", 126);
  emlrtAddField(*info, emlrtAliasP(lhs126), "lhs", 126);
  emlrtDestroyArray(&rhs64);
  emlrtDestroyArray(&lhs64);
  emlrtDestroyArray(&rhs65);
  emlrtDestroyArray(&lhs65);
  emlrtDestroyArray(&rhs66);
  emlrtDestroyArray(&lhs66);
  emlrtDestroyArray(&rhs67);
  emlrtDestroyArray(&lhs67);
  emlrtDestroyArray(&rhs68);
  emlrtDestroyArray(&lhs68);
  emlrtDestroyArray(&rhs69);
  emlrtDestroyArray(&lhs69);
  emlrtDestroyArray(&rhs70);
  emlrtDestroyArray(&lhs70);
  emlrtDestroyArray(&rhs71);
  emlrtDestroyArray(&lhs71);
  emlrtDestroyArray(&rhs72);
  emlrtDestroyArray(&lhs72);
  emlrtDestroyArray(&rhs73);
  emlrtDestroyArray(&lhs73);
  emlrtDestroyArray(&rhs74);
  emlrtDestroyArray(&lhs74);
  emlrtDestroyArray(&rhs75);
  emlrtDestroyArray(&lhs75);
  emlrtDestroyArray(&rhs76);
  emlrtDestroyArray(&lhs76);
  emlrtDestroyArray(&rhs77);
  emlrtDestroyArray(&lhs77);
  emlrtDestroyArray(&rhs78);
  emlrtDestroyArray(&lhs78);
  emlrtDestroyArray(&rhs79);
  emlrtDestroyArray(&lhs79);
  emlrtDestroyArray(&rhs80);
  emlrtDestroyArray(&lhs80);
  emlrtDestroyArray(&rhs81);
  emlrtDestroyArray(&lhs81);
  emlrtDestroyArray(&rhs82);
  emlrtDestroyArray(&lhs82);
  emlrtDestroyArray(&rhs83);
  emlrtDestroyArray(&lhs83);
  emlrtDestroyArray(&rhs84);
  emlrtDestroyArray(&lhs84);
  emlrtDestroyArray(&rhs85);
  emlrtDestroyArray(&lhs85);
  emlrtDestroyArray(&rhs86);
  emlrtDestroyArray(&lhs86);
  emlrtDestroyArray(&rhs87);
  emlrtDestroyArray(&lhs87);
  emlrtDestroyArray(&rhs88);
  emlrtDestroyArray(&lhs88);
  emlrtDestroyArray(&rhs89);
  emlrtDestroyArray(&lhs89);
  emlrtDestroyArray(&rhs90);
  emlrtDestroyArray(&lhs90);
  emlrtDestroyArray(&rhs91);
  emlrtDestroyArray(&lhs91);
  emlrtDestroyArray(&rhs92);
  emlrtDestroyArray(&lhs92);
  emlrtDestroyArray(&rhs93);
  emlrtDestroyArray(&lhs93);
  emlrtDestroyArray(&rhs94);
  emlrtDestroyArray(&lhs94);
  emlrtDestroyArray(&rhs95);
  emlrtDestroyArray(&lhs95);
  emlrtDestroyArray(&rhs96);
  emlrtDestroyArray(&lhs96);
  emlrtDestroyArray(&rhs97);
  emlrtDestroyArray(&lhs97);
  emlrtDestroyArray(&rhs98);
  emlrtDestroyArray(&lhs98);
  emlrtDestroyArray(&rhs99);
  emlrtDestroyArray(&lhs99);
  emlrtDestroyArray(&rhs100);
  emlrtDestroyArray(&lhs100);
  emlrtDestroyArray(&rhs101);
  emlrtDestroyArray(&lhs101);
  emlrtDestroyArray(&rhs102);
  emlrtDestroyArray(&lhs102);
  emlrtDestroyArray(&rhs103);
  emlrtDestroyArray(&lhs103);
  emlrtDestroyArray(&rhs104);
  emlrtDestroyArray(&lhs104);
  emlrtDestroyArray(&rhs105);
  emlrtDestroyArray(&lhs105);
  emlrtDestroyArray(&rhs106);
  emlrtDestroyArray(&lhs106);
  emlrtDestroyArray(&rhs107);
  emlrtDestroyArray(&lhs107);
  emlrtDestroyArray(&rhs108);
  emlrtDestroyArray(&lhs108);
  emlrtDestroyArray(&rhs109);
  emlrtDestroyArray(&lhs109);
  emlrtDestroyArray(&rhs110);
  emlrtDestroyArray(&lhs110);
  emlrtDestroyArray(&rhs111);
  emlrtDestroyArray(&lhs111);
  emlrtDestroyArray(&rhs112);
  emlrtDestroyArray(&lhs112);
  emlrtDestroyArray(&rhs113);
  emlrtDestroyArray(&lhs113);
  emlrtDestroyArray(&rhs114);
  emlrtDestroyArray(&lhs114);
  emlrtDestroyArray(&rhs115);
  emlrtDestroyArray(&lhs115);
  emlrtDestroyArray(&rhs116);
  emlrtDestroyArray(&lhs116);
  emlrtDestroyArray(&rhs117);
  emlrtDestroyArray(&lhs117);
  emlrtDestroyArray(&rhs118);
  emlrtDestroyArray(&lhs118);
  emlrtDestroyArray(&rhs119);
  emlrtDestroyArray(&lhs119);
  emlrtDestroyArray(&rhs120);
  emlrtDestroyArray(&lhs120);
  emlrtDestroyArray(&rhs121);
  emlrtDestroyArray(&lhs121);
  emlrtDestroyArray(&rhs122);
  emlrtDestroyArray(&lhs122);
  emlrtDestroyArray(&rhs123);
  emlrtDestroyArray(&lhs123);
  emlrtDestroyArray(&rhs124);
  emlrtDestroyArray(&lhs124);
  emlrtDestroyArray(&rhs125);
  emlrtDestroyArray(&lhs125);
  emlrtDestroyArray(&rhs126);
  emlrtDestroyArray(&lhs126);
}

MEXFUNCTION_LINKAGE mxArray *emlrtMexFcnProperties(void);
mxArray *emlrtMexFcnProperties()
{
  const char *mexProperties[] = {
    "Version",
    "ResolvedFunctions",
    "EntryPoints",
    "CoverageInfo",
    NULL };

  const char *epProperties[] = {
    "Name",
    "NumberOfInputs",
    "NumberOfOutputs",
    "ConstantInputs" };

  mxArray *xResult = mxCreateStructMatrix(1,1,4,mexProperties);
  mxArray *xEntryPoints = mxCreateStructMatrix(1,2,4,epProperties);
  mxArray *xInputs = NULL;
  xInputs = mxCreateLogicalMatrix(1, 7);
  mxSetFieldByNumber(xEntryPoints, 0, 0, mxCreateString(
    "set_simple_compressor_parameters"));
  mxSetFieldByNumber(xEntryPoints, 0, 1, mxCreateDoubleScalar(7));
  mxSetFieldByNumber(xEntryPoints, 0, 2, mxCreateDoubleScalar(0));
  mxSetFieldByNumber(xEntryPoints, 0, 3, xInputs);
  xInputs = mxCreateLogicalMatrix(1, 1);
  mxSetFieldByNumber(xEntryPoints, 1, 0, mxCreateString("simple_compressor"));
  mxSetFieldByNumber(xEntryPoints, 1, 1, mxCreateDoubleScalar(1));
  mxSetFieldByNumber(xEntryPoints, 1, 2, mxCreateDoubleScalar(1));
  mxSetFieldByNumber(xEntryPoints, 1, 3, xInputs);
  mxSetFieldByNumber(xResult, 0, 0, mxCreateString("8.3.0.532 (R2014a)"));
  mxSetFieldByNumber(xResult, 0, 1, (mxArray*)emlrtMexFcnResolvedFunctionsInfo());
  mxSetFieldByNumber(xResult, 0, 2, xEntryPoints);
  return xResult;
}

/* End of code generation (_coder_set_simple_compressor_parameters_info.c) */
