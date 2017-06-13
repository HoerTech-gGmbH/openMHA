function [fh, fname, ph] = sd_plot( sData, xidx, yidx, sPlotPars, varargin )
% plot data from data structures
%
% Mandatory paramaters:
%  sData  : Data structure with fields 'fields', 'values' and 'data'
%           (additional fields are ignored)
%  xidx   : column number of X-data
%  yidx   : column number of Y-data
%
% Optional parameters:
%  sPlotPars  : plot parameter structure, with optional fields:
%                - fontsize (14)
%                - markersize (fontsize/1.8)
%                - linewidth (1)
%                - gridfreq (1)
%                - firstgrid (1)
%                - colors ({'b','g','r','k','m','c','y'})
%                - linestyles ({'-','--','-.',':','-','-','-'})
%                - markers ({'d','x','*','o','+','*','d'})
%                - errorscale (1)
%                - xshift (0)
%
% Possible optional parameters (tag-value-pairs) :
%   'average'     : 'mean' or 'median' (mean)
%   'restrictions': list of data restrictions
%   'parameter'   : column number of parameter field
%   'xorder'      : re-ordering index vector
%   'errorscale'  : error scale (1)
% 
% Return values:
%  fh    : figure handle
%  fname : Unique file name containing information on data selection 
%
% Author: Giso Grimm
% Date: 11/2006
% Modified: 6/2007
  ;
  if nargin < 4
    sPlotPars = struct;
  end
  sAnalysisPar = struct();
  for k=1:length(varargin)/2
    sAnalysisPar.(varargin{2*k-1}) = varargin{2*k};
  end
  if ischar( xidx )
    xidx = strmatch(xidx,sData.fields,'exact');
  end
  if iscell( yidx )
    csY = yidx;
    yidx = [];
    for k=1:numel(csY)
      yidx = [yidx,strmatch(csY{k},sData.fields,'exact')];
    end
  end
  if ischar( yidx )
    yidx = strmatch(yidx,sData.fields,'exact');
  end
  sAnalysisPar = default_field( sAnalysisPar, 'average', 'mean' );
  sAnalysisPar = default_field( sAnalysisPar, 'restrictions', {} );
  sAnalysisPar = default_field( sAnalysisPar, 'parameter', [] );
  sAnalysisPar = default_field( sAnalysisPar, 'xorder', [] );
  sAnalysisPar = default_field( sAnalysisPar, 'errorscale', 1 );
  if strcmp(sAnalysisPar.average,'median')
    sAnalysisPar = default_field( sAnalysisPar, 'statfun', @avgfun_median );
  else
    sAnalysisPar = default_field( sAnalysisPar, 'statfun', @avgfun_mean );
  end
  if ischar( sAnalysisPar.parameter )
    sAnalysisPar.parameter = strmatch(sAnalysisPar.parameter,sData.fields,'exact');
  end
  sPlotPars = default_plot_pars( sPlotPars );
  restrictions = sAnalysisPar.restrictions;
  xorder = sAnalysisPar.xorder;
  pidx = sAnalysisPar.parameter;
  d = sData.data;
  [d, restrictions] = apply_restrictions( d, restrictions, sData );
  sAnalysisPar = default_field( sAnalysisPar, 'xlabel', sprintf('%s ',sData.fields{xidx}) );
  sAnalysisPar = default_field( sAnalysisPar, 'ylabel', sData.fields(yidx));
  x_name = sAnalysisPar.xlabel;
  y_name = sprintf('%s ',sData.fields{yidx});
  par_name = sprintf('%s ',sData.fields{pidx});
  fname = sprintf('%s_%s_%s_%s',y_name,x_name,par_name,restrictions);
  fname = tofilename(fname);
  vpar = unique( d(:,pidx) );
  fh = figure('PaperType','A4','Name',fname,'MenuBar','none');
  set(fh,'UserData',sPlotPars);
  nplots = length(yidx);
  ph = [];
  for kpl=1:nplots
    hax = subplot(nplots,1,kpl);
    pos = get(hax,'Position');
    set(hax,'Position',pos.*[1 1 0.74 1],'Tag','sd_ax');
    [tmp,vph] = ...
	sub_plot_data( d, xidx, yidx(kpl), pidx, xorder, ...
		       sData, sAnalysisPar, kpl);
    ph = [ph;vph];
    if kpl==1
      h = title(restrictions);
      set(h,'Interpreter','none','fontsize',sPlotPars.fontsize,'units','normalized',...
	    'horizontalalignment','left','position',[0 1.03 1]);
    end
    if kpl==nplots
      h = xlabel(x_name,'interpreter','none');
      set(h,'fontsize',sPlotPars.fontsize,'fontweight','bold');
    end
  end
  if ~isempty(pidx)
    csLeg = sData.values{pidx}(vpar);
    csLeg = any2cell( csLeg );
    h = legend(csLeg,'Interpreter','none','Location','NorthEast');
    set(get(h,'title'),'string',par_name,'interpreter','none',...
		      'fontsize',sPlotPars.fontsize,'fontweight','bold',...
		      'position',[0.5 1.02 1]);
    set(h,'Position',[0.73 0.05 0.19 0.12]);
    set(h,'Position',[0.73 0.05 0.19 0.12]);
    set(findobj(h,'type','line'),'linewidth',0.8);
  end
  
function s = default_plot_pars( s )
  s = default_field( s, 'fontsize', 14 );
  s = default_field( s, 'markersize', s.fontsize/1.8 );
  s = default_field( s, 'linewidth', 1 );
  s = default_field( s, 'gridfreq', 1 );
  s = default_field( s, 'firstgrid', 1 );
  s = default_field( s, 'colors', {'b','g','r','k','m','c','y'} );
  s = default_field( s, 'linestyles', {'-','--','-.',':','-','-','-'} );
  s = default_field( s, 'markers', {'d','x','*','o','+','*','d'} );
  s = default_field( s, 'xshift', 0 );
  if ~iscell(s.linestyles)
    s.linestyles = {s.linestyles};
  end
  if ~iscell(s.markers)
    s.markers = {s.markers};
  end
  if ~iscell(s.colors)
    s.colors = {s.colors};
  end
  return
  
function s = default_field( s, sField, sVal )
  if ~isfield( s, sField )
    s.(sField) = sVal;
  end

function x = any2cell( x )
  if isnumeric(x)
    tmp = {};
    for k=1:numel(x)
      tmp{end+1} = num2str(x(k));
    end
    x = tmp;
  end
  return

function [d, str] = apply_restrictions( d, restrictions, sData )
  str = '';
  for cd=restrictions
    lcd = cd{:};
    didx = [];
    for key=lcd{2}
      didx = [didx;find(d(:,lcd{1})==key)];
    end
    d = d(didx,:);
    val = any2cell(sData.values{lcd{1}}(lcd{2}));
    val = sprintf('+%s',val{:});
    val = val(2:end);
    field = sData.fields{lcd{1}};
    if length(str) == 0
      str = sprintf('%s:%s',field,val);
    else
      str = sprintf('%s,%s:%s',str,field,val);
    end
  end
  if isempty(d)
    error('No data.');
  end
  
function [vpar,vph] = sub_plot_data( d, xidx, yidx, pidx, xorder, sData, ...
			       sAnalysisPar,kpl)
  vph = [];
  sPlotPars = get(gcf,'UserData');
  vy = d(:,yidx);
  vx = unique( d(:,xidx) );
  vpar = unique( d(:,pidx) );
  npar = max(1,length(vpar));
  nx = length(vx);
  if isempty( xorder )
    xorder = 1:nx;
  end
  mmean = zeros(nx,npar);
  mstdl = zeros(nx,npar);
  mstdu = zeros(nx,npar);
  %clinespec = {'b-d','g--x','r-.*','k:o','m-+','c-*','y-d'};
  for kpar=1:npar
    sLineSpec = [ ...
	sPlotPars.colors{1+mod(kpar-1,length(sPlotPars.colors))}, ...
	sPlotPars.linestyles{1+mod(kpar-1,length(sPlotPars.linestyles))}, ...
	sPlotPars.markers{1+mod(kpar-1,length(sPlotPars.markers))} ...
		];
    kxm = 0;
    for kx=xorder
      kxm = kxm + 1;
      if ~isempty(pidx)
	point_idx = find( (d(:,xidx)==vx(kx)) .* (d(:,pidx)==vpar(kpar)) );
      else
	point_idx = find( (d(:,xidx)==vx(kx)) );
      end
      data = vy(point_idx);
      [mmean(kxm,kpar),mstdl(kxm,kpar),mstdu(kxm,kpar)] = ...
	  sAnalysisPar.statfun(data);
    end
    mstdl = mstdl * sAnalysisPar.errorscale;
    mstdu = mstdu * sAnalysisPar.errorscale;
    dx = sPlotPars.xshift*(kpar-npar/2-0.5)*0.03;
    if any(mstdl(:,kpar))|any(mstdu(:,kpar))
      h = errorbar([1:length(vx)]+dx,mmean(:,kpar),mstdl(:,kpar),mstdu(:,kpar), ...
		   sLineSpec);
    else
      h = plot([1:length(vx)]+dx,mmean(:,kpar), ...
	       sLineSpec);
    end
    set(h,'Linewidth',sPlotPars.linewidth,...
	  'MarkerSize',sPlotPars.markersize);
    vph = [vph;h];
    hold on;
  end
  hold off;
  xlab = sData.values{xidx}(vx);
  xlab = xlab(xorder);
  xlab = any2cell( xlab(sPlotPars.firstgrid:sPlotPars.gridfreq:end) );
  set(gca,'xtick',sPlotPars.firstgrid:sPlotPars.gridfreq:length(vx),...
	  'xticklabel',xlab);
  h = ylabel(sAnalysisPar.ylabel{kpl},'interpreter','none');
  set(gca,'fontsize',sPlotPars.fontsize);
  set(h,'fontsize',sPlotPars.fontsize,'fontweight','bold');
  xlim([0.5 length(vx)+0.5]);
  

function s = tofilename( s )
  s(strfind(s,'+')) = '';
  s(strfind(s,' ')) = '';
  s(strfind(s,'.')) = '';
  s(strfind(s,',')) = '_';
  s(strfind(s,'/')) = '';
  s(strfind(s,sprintf('\n'))) = '_';
  s(strfind(s,'(')) = '';
  s(strfind(s,')')) = '';
  s(strfind(s,'[')) = '';
  s(strfind(s,']')) = '';
  s(strfind(s,'{')) = '';
  s(strfind(s,'}')) = '';
  s(strfind(s,':')) = '=';
  s(strfind(s,'''')) = '';
  s = strrep(s, 'SIIenhancementdB','SI');
  s = strrep(s, 'spectraldistortiondB','SD');
  s = strrep(s, 'degreeofdiffusiveness','DOD');
  s = strrep(s, 'environment','env');
  s = strrep(s, 'algorithm','alg');
  s = strrep(s, 'competingazimuth','caz');
  s = strrep(s, 'caz=a000diffuse','');
  s = strrep(s, 'a000','a0');
  s = strrep(s, 'a060','a60');
  s = strrep(s, 'desiredSNRdB','dSNR');
  s = strrep(s, 'DOD_env_dSNR_alg=coh1_','DODenvdSNR');
  %s = strrep(s, '_', '');
  s = strrep(s, 'caz=a60diffuse','');

function x = donothing( x, data )
  return
  
function [m,l,u] = avgfun_mean( x )
  m = mean(x);
  l = -std(x);
  u = std(x);
  
function [m,l,u] = avgfun_median( x )
  m = median(x);
  l = quantile(x,0.25)-m;
  u = quantile(x,0.75)-m;
