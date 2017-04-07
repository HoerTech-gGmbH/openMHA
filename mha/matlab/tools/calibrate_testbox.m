%
%
% 1. Get Delay

srate = 44100;
t = 0.5;
x = 0.1*rand(t*srate,1);
x = x - sum(x)/(t*srate);

[y,fs,bufs] = tascar_jackio(x(:,[1,1]),{'system:playback_1','system:playback_2'},{'system:capture_1','system:capture_2'});%

[c1,l1] = xcorr(y(:,1),x);
[c2,l2] = xcorr(y(:,2),x);

[cmax1,imax1] = max(c1);
[cmax2,imax2] = max(c2);

delay1 = l1(imax1);
delay2 = l2(imax2);

sprintf('Delay 1 = %f',delay1/fs);


% 2. Get impulse response

[irs,fs] = tascar_getirs( 2^14, 8, 0.1, {'system:playback_1','system:playback_2'},{'system:capture_1','system:capture_2'}, 2 );
irs = smooth_irs(irs,fs,1000);
irs = ir2minphaseir(irs,512);

%phasecomp = unwrap(angle(realfft(irs)));

% 3. Start MHA using delay from channel 1 measurements
mha = mha_start();
mha_query(mha,'','read:testbox_calibration_2.cfg');
mha_set(mha,'mha.chain.testbox.algo_delay',delay1);

fs = mha_get(mha,'fragsize');
%mha_set(mha,'mha.chain.algo.fftfilter.irs',irs(1:fs)');
mha_set(mha,'mha.calib_in.fir',irs(1:fs)');


% 4. Start testbox evaluation script

testbox(mha)


% 5. Connect input and output and hit "start"