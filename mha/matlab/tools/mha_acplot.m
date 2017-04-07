function mha_acplot( sChain, csVars, cAxPar, vXFun )
% MHA_ACPLOT - plot MHA AC variables in real-time, utilizing the
% MHA plugin ac2osc_file
%
% Usage:
% mha_acplot( sChain, csVars, cAxPar, vXFun )
%
% Parameters:
% - sChain : Name of chain and plugin
% - csVars : cell string array of AC variable names
% - cAxPar : cell array with cell string array of axes property
%            pairs (optional)
% - vXFun  : function to be evaluated before plotting data. Value can
%            be either a character string containing a Matlab
%            expression with a numeric vector return value, or a
%            function handle of a function with one argument (x
%            data). The name of the x data is 'x'.
%
% Example:
% mha_acplot( 'plugin.acsend', {'c_rcoh','l_level_db'}, ...
%             {{'YLim',[0 1]},{'YLim',[-50 80],'XLim',[1 13]}} )
%
% Notes:
%  1. configure the 'ac2osc_file' plugin properly
%  2. start this Matlab script in the current working directory of
%     the MHA *before* starting MHA processing
%
  if nargin < 3
    cAxPar = {};
  end
  if nargin < 4
    vXFun = {};
  end
  global lph;
  global lfunh;
  lph = struct;
  lfunh = struct;
  nx = floor(sqrt(length(csVars)));
  ny = length(csVars)/nx;
  for k=1:length(csVars)
    subplot(nx,ny,k);
    lph.(csVars{k}) = plot(1,1,'linewidth',2);
    if length(cAxPar) >= k
      for kap = 1:ceil(length(cAxPar{k})/2)
	set(gca,cAxPar{k}{2*kap-1},cAxPar{k}{2*kap});
      end
    end
    if length(vXFun) >= k
      if ~isempty(vXFun{k})
	lfunh.(csVars{k}) = vXFun{k};
      end
    end
    title(csVars{k},'Interpreter','none');
  end
  set(gcf,'name',sChain);
  drawnow;
  system(sprintf('rm -f %s.osc',sChain));
  system(sprintf('mkfifo %s.osc',sChain));
  while 1
    readoscfile( [sChain '.osc'], @plot_cb );
  end

function err = plot_cb( name, x )
  err = 0;
  global lph;
  global lfunh;
  name = name(2:end);
  if isfield(lph,name)
    if isfield(lfunh,name)
      if ischar(lfunh.(name))
	eval(['x=' lfunh.(name) ';']);
      else
	x = lfunh.(name)(x);
      end
    end
    set(lph.(name),'XData',1:length(x),'YData',x);
    drawnow;
  end
