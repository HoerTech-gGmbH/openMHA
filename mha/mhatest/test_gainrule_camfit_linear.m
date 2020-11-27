function test_gainrule_camfit_linear

% This test creates a first fit for the "slope" demo audiogramm and for the
% normal hearing audiogram (0dB HL at all frequencies) and checks the fitting
% done by gainrule_camfit_linear.
  slope.id = 'slope';
  slope.client_id = 'TT123456';
  freqs = [125, 250, 500, 750, 1000, 1500, 2000, 3000, 4000, 6000, 8000];
  hl_l =  [ 10,  15,  30,  40,   45,   50,   65,   65,   75,   65,   65];
  hl_r =  [ 20,  15,  20,  30,   35,   35,   40,   55,   70,   65,   85];

  % easy access to some selected thresholds
  index125 = 1;
  index2k = 7;

  for idx = 1:length(freqs)
    slope.l.htl_ac.data(idx).f = freqs(idx);
    slope.l.htl_ac.data(idx).hl = hl_l(idx);
    slope.r.htl_ac.data(idx).f = freqs(idx);
    slope.r.htl_ac.data(idx).hl = hl_r(idx);
  end

  sFitmodel.frequencies = freqs;
  sFitmodel.edge_frequencies = [0.1 sqrt(freqs(1:end-1).*freqs(2:end)) 10000001];
  sFitmodel.levels = [-10:110];
  sFitmodel.channels = 2;
  sFitmodel.side = 'lr';

  sGt_actual = gainrule_camfit_linear(slope, sFitmodel);

  % Check that gainrule_camfit_linear return a field "insertion_gains"
  assert_all(isfield(sGt_actual, 'insertion_gains'));
  
  % Check the fittings done at 125 Hz, and 2000 Hz:
  intercept_125 = -11;
  intercept_2000 = 1;
  expected_gain_125l = 0.48 * hl_l(index125) + intercept_125;
  expected_gain_125r = 0.48 * hl_r(index125) + intercept_125;
  expected_gain_2000l = 0.48 * hl_l(index2k) + intercept_2000;
  expected_gain_2000r = 0.48 * hl_r(index2k) + intercept_2000;

  actual1_gain_125l = sGt_actual.l(1,index125);
  actual1_gain_125r = sGt_actual.r(1,index125);
  actual1_gain_2000l = sGt_actual.l(1,index2k);
  actual1_gain_2000r = sGt_actual.r(1,index2k);
  actual2_gain_125l = sGt_actual.insertion_gains.l(index125);
  actual2_gain_125r = sGt_actual.insertion_gains.r(index125);
  actual2_gain_2000l = sGt_actual.insertion_gains.l(index2k);
  actual2_gain_2000r = sGt_actual.insertion_gains.r(index2k);

  % first check that both actual values are the same
  assert_equal([actual1_gain_125l,actual1_gain_125r,actual1_gain_2000l,actual1_gain_2000r],...
                        [actual2_gain_125l,actual2_gain_125r,actual2_gain_2000l,actual2_gain_2000r]);

  % Then check that expected and actual values are the same except if the expected value would be negative
  if expected_gain_125l < 0
    assert_equal(0, actual1_gain_125l);
  else
    assert_equal(expected_gain_125l, actual1_gain_125l);
  end

  if expected_gain_125r < 0
    assert_equal(0, actual1_gain_125r);
  else
    assert_equal(expected_gain_125r, actual1_gain_125r);
  end

  if expected_gain_2000l < 0
    assert_equal(0, actual1_gain_2000l);
  else
    assert_equal(expected_gain_2000l, actual1_gain_2000l);
  end

  if expected_gain_2000r < 0
    assert_equal(0, actual1_gain_2000r);
  else
    assert_equal(expected_gain_2000r, actual1_gain_2000r);
  end
