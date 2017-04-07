function b_valid = loctest_sound_play( sPar, sCfg )
  b_valid = true;
  % for speaker system do only allow 30 deg steps:
  if strcmp( sPar.method, 'speaker' )
    b_valid = (mod(sPar.az,30) == 0);
  end
  if b_valid 
    disp('now playing sound...');
    % hier kommt sowohl richtungskonfiguration (send_osc) als auch
    % soundmex hin.
    disp(sPar)
    pause(1);
  end