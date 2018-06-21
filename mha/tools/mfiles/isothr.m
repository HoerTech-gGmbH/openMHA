function [vIsoThrDB, vsF] = isothr(vsDesF);
% [vIsoThrDB, vsF] = isothr(vsDesF);
%
% values from 20 Hz to 12500 Hz are taken from ISO 226:2003(E)
% values from 14000 Hz to 18000 Hz are taken from ISO 389-7:2005
% values at 0 and 20000 Hz are not taken from ISO Threshold contour !!

  iso226_389 = [
            0   80.0
           20   78.5
           25   68.7
           31.5 59.5
           40   51.1
           50   44.0
           63   37.5
           80   31.5
          100   26.5
          125   22.1
          160   17.9
          200   14.4
          250   11.4
          315    8.6
          400    6.2
          500    4.4
          630    3.0
          800    2.2
         1000    2.4
         1250    3.5
         1600    1.7
         2000   -1.3
         2500   -4.2
         3150   -6.0
         4000   -5.4
         5000   -1.5
         6300    6.0
         8000   12.6
        10000   13.9
        12500   12.3
	
        14000   18.4
        16000   40.2
        18000   73.2
        20000   70.0
  ];
  vThr = iso226_389(:,2);
  vsF  = iso226_389(:,1);
  
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
