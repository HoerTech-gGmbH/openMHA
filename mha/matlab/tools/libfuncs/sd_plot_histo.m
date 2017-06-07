function [ph,mN] = sd_plot_histo( s, c_par, c_val, nBins )
% PLOT_STRUCT_DATA_VAL_HISTO - Histogram plotting function
%
% Usage:
% plot_sd_val_histo( s, c_par, c_val )
%
% s     : data structure
% c_par : parameter column
% c_val : data column
%
% Author: Giso Grimm, 2007
  ;
  % compact the value vectors and remove unused entries:
  s = sd_compactval( s );
  % get or calculate bins:
  v_min = min(s.data(:,c_val));
  v_max = max(s.data(:,c_val));
  if nargin < 4
    nBins = 10;
  end
  if prod(size(nBins))==1
    dv = (v_max-v_min)/nBins;
    vBins = [v_min:dv:v_max];
  else
    vBins = nBins;
    nBins = length(vBins)-1;
  end
  % get histograms:
  cValues = s.values{c_par};
  mN = zeros(nBins,length(cValues));
  for k=1:length(cValues)
    idx = find(s.data(:,c_par)==k);
    vN = histc(s.data(idx,c_val),vBins);
    vN(end) = [];
    mN(:,k) = vN ./ sum(vN);
  end
  % plot the data:
  ph = plot(0.5*(vBins(1:end-1)+vBins(2:end)),mN);
  xlim([min(vBins) max(vBins)]);
  if isnumeric(cValues)
    cValuesO = {};
    for k=1:length(cValues)
      cValuesO{k} = sprintf('%g',cValues(k));
    end
    cValues = cValuesO;
  end
  legend(cValues,'Location','Best')
  title(s.fields{c_par});
  xlabel(s.fields{c_val});
