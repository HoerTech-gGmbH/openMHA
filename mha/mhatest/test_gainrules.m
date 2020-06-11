function test_gainrules

  test_01();
  test_02();

function test_01
% This test creates a first fit for the "slope" demo audiogramm with three
% fitting rules and compares if the first fit is equal to the value obtained
% with the first fit before the fitting rule functions were refactored to
% remove code duplication.
  sAud.id = 'slope';
  sAud.client_id = 'TT123456';
  freqs = [125, 250, 500, 750, 1000, 1500, 2000, 3000, 4000, 6000, 8000];
  hl_l =  [ 10,  15,  30,  40,   45,   50,   65,   65,   75,   65,   65];
  hl_r =  [ 20,  15,  20,  30,   35,   35,   40,   55,   70,   65,   85];
  for idx = 1:length(freqs)
    sAud.l.htl_ac.data(idx).f = freqs(idx);
    sAud.l.htl_ac.data(idx).hl = hl_l(idx);
    sAud.r.htl_ac.data(idx).f = freqs(idx);
    sAud.r.htl_ac.data(idx).hl = hl_r(idx);
  end

  sFitmodel.frequencies = [177 297 500 841 1414 2378 3999.99976 6726.99951 11314];
  sFitmodel.edge_frequencies = [9.99999994e-09 229.279297 385.356964 ...
                                               648.459717 1090.49243 1833.70972 3084.15283 5187.29199 8724.0625 10000001];
  sFitmodel.levels = [-10:110];
  sFitmodel.channels = 2;
  sFitmodel.side = 'lr';

  sGt_expected = load('test_gainrules_data.mat');
  sGt_actual = gainrule_NALRP(sAud, sFitmodel);
  assert_equal(sGt_expected.sGt_nalrp_01, sGt_actual);

  compression_ratio = 2;
  sGt_actual = gainrule_CRvar_NALRP(sAud, sFitmodel, compression_ratio);
  assert_almost_equal_gaintable_structs(sGt_expected.sGt_cr2nalrp_01, sGt_actual,1e-12);

  compression_ratio = 3;
  sGt_actual = gainrule_CRvar_NALRP(sAud, sFitmodel, compression_ratio);
  assert_almost_equal_gaintable_structs(sGt_expected.sGt_cr3nalrp_01, sGt_actual,1e-12);

  sGt_actual = gainrule_camfit_compr(sAud, sFitmodel);
  assert_almost(sGt_expected.sGt_camfit_compr_01.l,sGt_actual.l,1e-12);
  assert_almost(sGt_expected.sGt_camfit_compr_01.r,sGt_actual.r,1e-12);

function test_02
  sAud.id = 'htl1';
  sAud.client_id = 'TT000002';
  freqs = [125, 250, 500, 750, 1000, 1500, 2000, 3000, 4000, 6000, 8000];
  hl_l =  [ 50,  50,  50,  50,   55,   55,   60,   65,   80,   85,   90];
  hl_r =  [ 40,  40,  40,  45,   50,   55,   65,   70,   70,   70,   70];
  for idx = 1:length(freqs)
    sAud.l.htl_ac.data(idx).f = freqs(idx);
    sAud.l.htl_ac.data(idx).hl = hl_l(idx);
    sAud.r.htl_ac.data(idx).f = freqs(idx);
    sAud.r.htl_ac.data(idx).hl = hl_r(idx);
  end

  sFitmodel.frequencies = [276 1029 2632 9391];
  sFitmodel.edge_frequencies = [9.99999994000000e-09, 5.32920227000000e+02, ...
                                1.64569995000000e+03, 4.97163135000000e+03, ...
                                1.00000010000000e+07];
  sFitmodel.levels = [-10:110];
  sFitmodel.channels = 2;
  sFitmodel.side = 'lr';

  sGt_expected = load('test_gainrules_data.mat');

  sGt_actual = gainrule_NALRP(sAud, sFitmodel);
  assert_equal(sGt_expected.sGt_nalrp_02, sGt_actual);

  compression_ratio = 2;
  sGt_actual = gainrule_CRvar_NALRP(sAud, sFitmodel, compression_ratio);
  assert_almost_equal_gaintable_structs(sGt_expected.sGt_cr2nalrp_02, sGt_actual,1e-6);

  compression_ratio = 3;
  sGt_actual = gainrule_CRvar_NALRP(sAud, sFitmodel, compression_ratio);
  assert_almost_equal_gaintable_structs(sGt_expected.sGt_cr3nalrp_02, sGt_actual,1e-6);

  sGt_actual = gainrule_camfit_compr(sAud, sFitmodel);
  assert_equal(sGt_expected.sGt_camfit_compr_02, sGt_actual);

function assert_almost_equal_gaintable_structs(sExpected,sActual,epsilon)
  assert_almost(sExpected.compression.l.gain,sActual.compression.l.gain,epsilon);
  assert_almost(sExpected.compression.l.l_kneepoint,sActual.compression.l.l_kneepoint,epsilon);
  assert_almost(sExpected.compression.l.c_slope,sActual.compression.l.c_slope,epsilon);

  assert_almost(sExpected.compression.r.gain,sActual.compression.r.gain,epsilon);
  assert_almost(sExpected.compression.r.l_kneepoint,sActual.compression.r.l_kneepoint,epsilon);
  assert_almost(sExpected.compression.r.c_slope,sActual.compression.r.c_slope,epsilon);

  assert_almost(sExpected.l,sActual.l,epsilon);
  assert_almost(sExpected.r,sActual.r,epsilon);
