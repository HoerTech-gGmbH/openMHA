function [vIsoThrDB, vsF] = isothr(vsDesF);
% WARNING: source 
% [vIsoThrDB, vsF] = isothr(vsDesF);
%
% author: Jens-E. Appell
%
% values from 20 Hz to 12500 Hz are taken from ISO 226 (1985-05-01)
% values at 14000 Hz and 15000 Hz are taken from ISO-Threshold-table
% in Klaus Bethges thesis.
% values at 0 and 20000 Hz are not taken from ISO Threshold contour !!
  vThr = [ 80    74.3 65.0  56.3   48.4 41.7 35.5  29.8 25.1 20.7 ...
           16.8 13.8 11.2 8.9   7.2 6.0 5.0  4.4 4.2 3.8  2.6 1.0 ...
           -1.2 -3.6 -3.9 -1.1 6.6 15.3 16.4 11.6    16.0 24.1    70.0]';
  vsF  =1000*[0.0    0.02 0.025 0.0315 0.04 0.05 0.063 0.08 0.1 ...
              0.125 0.16 0.2  0.25 0.315 0.4 0.5 0.63 0.8 1.0 1.25 ...
              1.6 2.0 2.5  3.15 4.0  5.0  6.3 8.0  10.  12.5    14.0 ...
              15.0    20.0]';
  if( ~isempty( find( vsDesF < 50 ) ) )
    warning('frequency values below 50 Hz set to 50 Hz');
    vsDesF(find( vsDesF < 50 )) = 50;
  end
  if nargin > 0,
    vIsoThrDB = interp1(vsF,vThr,vsDesF,'linear','extrap');
    vsF = vsDesF;
  else,
    vIsoThrDB = vThr;
  end;



