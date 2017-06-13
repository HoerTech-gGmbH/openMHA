function y = freq_interp_sh( f_in, y_in, f )
% FREQ_INTERP - linear interpolation on logarithmic frequency scale
% with sample and hold on edges
%
% Usage:
% y = freq_interp_sh( f_in, y_in, f );
%
  ;
  f_in = max(eps,f_in);
  y = interp1(log([0.5*f_in(1);f_in(:);2*f_in(end)]),...
	      [y_in(1);y_in(:);y_in(end)],...
	      log(f),'linear','extrap');

