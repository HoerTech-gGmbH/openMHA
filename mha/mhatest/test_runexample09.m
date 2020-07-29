% Steps to recreate the reference MAT file:
%
% - Navigate to <example_dir>/09-localizer-steering-beamformer/.
%
% - Edit Jack_live_resampling_doasvm_4Chan_16K_pool_acSteerMVDR_binaural.cfg to
%   match the code below, i.e., the end result should be a configuration based
%   on MHAIOFile that processes localizer-test.wav.  Be sure to uncomment
%   "cmd=quit" at the end of the file.
%
% - Execute start_demo_live_binauralSteerBF.sh.
%
% - Launch Octave and navigate to the example's directory.
%
% - Execute the following code:
%
%   load('Output/4ch_doasvm_acsave.mat');
%   pool = pool(1:10:end, :);
%   save('-hdf5', 'example09_acref.mat', 'pool', 'pool_max');
%
% - Move the new MAT file to this directory.

function test_runexample09()
  example_dir = mha_install_dirs('examples');
  work_dir = fullfile(example_dir, '09-localizer-steering-beamformer');
  cfg = 'Jack_live_resampling_doasvm_4Chan_16K_pool_acSteerMVDR_binaural.cfg';
  original_cfg = fullfile(work_dir, cfg);
  modified_cfg = fullfile(work_dir, '__test.cfg');
  inwav = fullfile(work_dir, 'localizer-test.wav');
  outwav = fullfile(work_dir, 'Output', 'Enhanced_localizer-test.wav');
  ac_out = fullfile(work_dir, 'Output', '4ch_doasvm_acsave.mat');
  ac_ref = fullfile(pwd, 'example09_acref.mat');

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

  % sub-sample pool to match ac_ref
  cur_mat.pool = cur_mat.pool(1:10:end, :);

  % Assert that the field pool is equal within a tolerance, and that pool_max
  % is exactly equal, since it is an integer (index) value.
  assert_difference_below(ref_mat.pool, cur_mat.pool, 1e-4);
  assert_equal(ref_mat.pool_max, cur_mat.pool_max);
end

function modify_cfg(original_cfg, modified_cfg)
    cfg = fileread(original_cfg);
    fh = fopen(modified_cfg, 'w');
    % Since 'cfg' is a single string, one cannot match using '^' and '$', but
    % must match individual newlines instead.  This, however, avoids splitting
    % the string into a cell array and then joining it again for writing.
    fwrite(fh, regexprep(cfg, '(\nsrate)|(\nio)|(\ncmd)', '\n#'));
    fclose(fh);
end
