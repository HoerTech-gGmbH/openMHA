function test_runexample09_feature_extraction()
  example_dir = mha_install_dirs('examples');
  work_dir = fullfile(example_dir, '09-localizer-steering-beamformer');
  cfg = 'Jack_live_resampling_doasvm_4Chan_16K_pool_acSteerMVDR_binaural.cfg';
  original_cfg = fullfile(work_dir, cfg);
  modified_cfg = fullfile(work_dir, '_feature__test.cfg');
  inwav = fullfile(work_dir, 'localizer-test.wav');
  outwav = fullfile(work_dir, 'Output', 'feature_extraction_localizer-test.wav');
  ac_out = fullfile(work_dir, 'Output', '4ch_doasvm_acsave_feature_extraction.mat');
  ac_ref = fullfile(pwd, 'example09_feature_extraction_ref.mat');

  % execute mha with the given config file in the example directory
  old_dir = chdir(work_dir);
  unittest_teardown(@chdir, old_dir);

  % make sure teardown errors don't distract from test errors
  fclose(fopen(outwav, 'w'));
  unittest_teardown(@delete, outwav);
  fclose(fopen(ac_out, 'w'));
  unittest_teardown(@delete, ac_out);

  % modify the original configuration to use file IO; this ensures to some
  % degree that original configuration file also works, and that this test uses
  % a known algorithm setup
  modify_cfg(original_cfg, modified_cfg);
  unittest_teardown(@delete, modified_cfg);

  mha = mha_start;
  unittest_teardown(@mha_set, mha, 'cmd', 'quit');

  mha_set(mha, 'srate', 16000);
  mha_query(mha, '', ['read:', modified_cfg]);
  mha_set(mha, 'iolib', 'MHAIOFile');
  mha_set(mha, 'io.in', inwav);
  mha_set(mha, 'io.out', outwav);
  mha_set(mha, 'cmd', 'start');
  mha_set(mha, 'cmd', 'release');

  ref_mat = load(ac_ref);
  cur_mat = load(ac_out);
  fn_ref = {'pool_max', 'pool'};

  % variable `pool` contains the probability data for each direction and instant
  % variable `pool_max` contains for each instant which direction has maximum
  % probability after some smoothing.

  % assert that all pool_max results are as expected
  assert_equal(ref_mat.pool_max, cur_mat.pool_max);
  
  % The noise floor is below pool == 0.08. 
  noise_level = 0.08;
  
  % The example file has no sound sources at 180°
  assert_all(cur_mat.pool(:,[1 end]) < noise_level);

  % prepare to identify sound sources: create another matrix from pool that
  % contains for each instant a value +1 at the direction where a sound source
  % begins spacially and a value -1 at the direction where a sound source ends
  % spacially when scanning from -180° to +180°.
  edges = diff(cur_mat.pool > 0.08, 1, 2); 
  
  % For each instant, count how many sound sources we see above the noise 
  % floor, compare with expectation.
  num_sources = sum(edges > 0, 2);
  assert_equal(ref_mat.num_sources, num_sources);
  
  % measure how wide each of the sound sources is, compare with expectation.
  [dummy,source_begin_directions] = find(edges == 1);
  [dummy,source_end_directions] = find(edges == -1);
  source_widths = source_end_directions - source_begin_directions;
  assert_equal(ref_mat.source_widths, source_widths);
  
  % Compare the noise floor. Median eliminates the sources somewhat.
  assert_difference_below(ref_mat.noise_floor, median(cur_mat.pool), 1e-6);
  
end

function modify_cfg(original_cfg, modified_cfg)
    cfg = fileread(original_cfg);
    fh = fopen(modified_cfg, 'w');
    % Since 'cfg' is a single string, one cannot match using '^' and '$', but
    % must match individual newlines instead.  This, however, avoids splitting
    % the string into a cell array and then joining it again for writing.
    edit = regexprep(cfg, '(\nsrate)|(\nio)|(\ncmd)', '\n#');
    edit = regexprep(edit, 'acsave.mat', 'acsave_feature_extraction.mat');
    fwrite(fh, edit);
    fclose(fh);
end
