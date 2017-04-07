function sGt = gainrule_Lecluyse2013( sAud, sCfg )
% Gain prescription rule for Essex Aid after Lecluyse (2013)
% Author: Tobias Herzke
% Date: 2013
  ;
  % configure global parameters:
  residual.f = [249 250 500 1000 2000 4000 8000 8001];
  residual.dB = [20 20 15 15 10 10 20 20];
  gain = [];
  
  for side='lr'
    % get the interpolated hearing threshold level:
    vF = [sAud.(side).htl_ac.data.f];
    HTL = [sAud.(side).htl_ac.data.hl];
    HTL = max(0,freq_interp_sh(vF,HTL,sCfg.frequencies));

    residual_dB = freq_interp_sh(residual.f, residual.dB,sCfg.frequencies);
    
    this_gain = max(0,HTL-residual_dB);
    this_gain = min(40, this_gain);
    
    for i = 1:length(sCfg.frequencies)
        if (sCfg.frequencies(i) > 5900) & (HTL(i) > 75)
            this_gain(i) = 0;
        end
    end
    
    gain = [gain;this_gain];
    sGt.(side) = repmat(this_gain, length(sCfg.levels), 1);
  end
  
  tc = 95 - gain;
  tm = ones(size(gain)) * 25;
  moc_factor = ones(size(sCfg.frequencies)) * 0.5;
  moc_tau = ones(size(sCfg.frequencies)) * 0.05;

  sGt.essex_io = struct;
  sGt.essex_io.tc = tc;
  sGt.essex_io.tm = tm;
  sGt.essex_io.gain = gain;
  sGt.essex_io.moc_factor = moc_factor;
  sGt.essex_io.moc_tau = moc_tau;
