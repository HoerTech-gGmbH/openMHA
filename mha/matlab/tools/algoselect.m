function algoselect( mha, reallabels )
% ALGOSELECT - Select active algorithm (renamed to 'A','B','C',...)
%
% Usage:
% algoselect( mha )
%
%   mha : IP address, host name or handle of MHA instance
%
% Author: Giso Grimm
% Date: 12/2009
  if nargin < 1
    mha = [];
  end
  if nargin < 2
    reallabels = false;
  end
  mha = mha_ensure_mhahandle( mha );
  sCfg = mha_get_basic_cfg_network( mha );
  sAddr = [];
  if isfield(sCfg.base,'altconfig')
    sAddr = sCfg.base.altconfig;
  elseif isfield(sCfg.base,'altplugs')
    sAddr = sCfg.base.altplugs;
  end
  if isempty(sAddr)
    error('No algo selection plugin in MHA');
  end
  ButtonWidth = 140;
  ButtonHeight = 140;
  ButtonSpace = 40;
  csLabels = mha_get(mha,[sAddr,'.labels']);
  sCurrent = mha_get(mha,[sAddr,'.select']);
  sNLab = char('A'+([1:length(csLabels)]-1));
  ScrS = get(0,'ScreenSize');
  vWSize = [ButtonSpace+(ButtonSpace+ButtonWidth)*length(csLabels),...
	    ButtonHeight+2*ButtonSpace];
  vPos = [round((ScrS(3:4)-vWSize)/2),vWSize];
  fh = figure('Name','MHA algoselect','Position',vPos,...
	      'NumberTitle','off','MenuBar','none');
  vUIh = [];
  N = length(csLabels);
  dx = 0.1/N;
  for k=1:N
    vPos = [(1-dx)*((k-1)/N+dx),0.1,1/N-dx,0.8];
    vUIh(k) = uicontrol('Units','normalized',...
			'Position',vPos);
    if reallabels
      set(vUIh(k),'String',csLabels{k});
    else
      set(vUIh(k),'String',sNLab(k));
    end
  end
  sData = struct;
  sData.uih = vUIh;
  sData.mha = mha;
  sData.addr = [sAddr,'.select'];
  sData.labels = csLabels;
  for k=1:length(vUIh)
    sData.index = k;
    sData.label = csLabels{k};
    set(vUIh(k),...
	'style','ToggleButton','FontSize',18,'FontWeight','bold',...
	'UserData',sData,'Callback',@select_algo);
    if strcmp(sCurrent,csLabels{k})
      set(vUIh(k),'Value',1,'BackgroundColor',activecol);
    else
      set(vUIh(k),'Value',0,'BackgroundColor',bgcol);
    end
  end
  
function c = bgcol
  c = 0.7*ones(1,3);
  
function c = activecol
  c = [253,230,12]/255;
  
function select_algo(varargin)
  sData = get(gcbo,'UserData');
  ui_other = setdiff(sData.uih,gcbo);
  set(ui_other,'Value',0,'BackgroundColor',bgcol);
  mha_set(sData.mha,sData.addr,sData.label);
  set(gcbo,'Value',1,'BackgroundColor',activecol);