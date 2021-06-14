function handle = sel_speaker(src,event)
handle = guidata(src);
mha = handle.mha;

switch handle.speaker.Value
    case 1
        mhactl_wrapper(mha,{'?read:/etc/mahalia/calibration/speaker-m/calib.cfg'});
        mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.fresponse.select','identity');
    case 2
        mhactl_wrapper(mha,{'?read:/etc/mahalia/calibration/speaker-s/calib.cfg'});
        mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.fresponse.select','identity');
end
