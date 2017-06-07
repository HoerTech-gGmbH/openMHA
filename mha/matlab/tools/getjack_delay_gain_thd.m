function sState = getjack_delay_gain_thd( ch_in, ch_out, bdisp )
% getjack_delay_gain_thd - measure a sound card delay with jack
%
% Usage:
% sState = getjack_delay_gain_thd( ch_in, ch_out )
%
% Author: Giso Grimm
% Date: 1/2009
  ;
  if nargin < 1
    ch_in = 1;
  end
  if nargin < 2
    ch_out = 1;
  end
  if nargin < 3
    bdisp = 0;
  end
  [csp_out,csp_in] = jackgetports;
  sState.port_in = csp_in{ch_in};
  sState.port_out = csp_out{ch_out};
  [tmp,sState.srate,sState.fragsize] = ...
      jackiomex(zeros(2048,1),{sState.port_in},{sState.port_out});
  x = 0.02*(rand(round(0.5*sState.srate),1)-0.5);
  n = [x(end-3*sState.fragsize+1:end,1);x];
  y = jackiomex(n,{sState.port_in},{sState.port_out});
  y = y(end-round(0.5*sState.srate)+1:end,1);
  H = realfft(y)./realfft(x);
  sState.gain = median(20*log10(abs(H)));
  irs = realifft(H);
  [tmp,delay] = max(irs);
  sState.delay = delay-1;
  sState.delay_ms = sState.delay/sState.srate*1000;
  f0 = 1000;
  s = 10.^(0.05*(min(0,-sState.gain)-10)) * ...
      sin(f0*2*pi*[1:(sState.srate+sState.delay)]'/sState.srate);
  sy = jackiomex(s,{sState.port_in},{sState.port_out});
  sy = sy(end-sState.srate+1:end,1);
  SY = abs(realfft(sy)).^2;
  idx_f0 = f0+1;
  idx_f25 = [2*f0:f0:min(5*f0,round(sState.srate/2))]+1;
  sState.thd = 10*log10(sum(SY(idx_f25))/SY(idx_f0));
  if bdisp
    disp(sprintf('%d %d %d %1.2f %1.1f %1.1f',...
		 sState.fragsize,...
		 sState.srate,...
		 sState.delay,...
		 sState.delay_ms,...
		 sState.gain,...
		 sState.thd));
  end